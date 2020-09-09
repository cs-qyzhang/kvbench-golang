package main

import (
	"bufio"
	"fmt"
	"os"
	"os/exec"
	"strconv"
	"strings"
	"time"
)

// Stat is
type Stat struct {
	Duration       float64
	MaxLatency     float64
	AverageLatency float64
	Throughput     float64
}

// Phase is
type Phase struct {
	Name     string
	Stat     Stat
	NrThread int
	Size     uint64
	ScanSize uint64
}

// DB is
type DB struct {
	Name      string
	ClassName string
	KeyType   string
	ValueType string
	PreTask   string
	Env       []string
	Task      string
	AfterTask string
	Phases    []Phase // first phase represents overall
}

// Environ is
type Environ struct {
	MachineName string
	CPUModel    string
	RAMSize     string
	DiskModel   string
	OS          string
	Kernel      string
}

// Doc is the structure passed to tex template
type Doc struct {
	Author   string
	Title    string
	DateTime string
	Environ  Environ
	DB       []DB
}

// InitFromOption fills doc with option
func (doc *Doc) InitFromOption(opt Option) {
	doc.Author = opt.Author
	doc.Title = opt.Title
	for _, db := range opt.DB {
		newDB := DB{}
		newDB.Name = db.Name
		newDB.ClassName = db.ClassName
		newDB.KeyType = db.KeyType
		newDB.ValueType = db.ValueType
		newDB.PreTask = db.PreTask
		newDB.Task = db.Task
		newDB.AfterTask = db.AfterTask
		newDB.Env = db.Env

		newPhase := Phase{}
		newPhase.Name = "overall"
		newDB.Phases = append(newDB.Phases, newPhase)

		for _, phase := range opt.Phases {
			newPhase := Phase{}
			newPhase.Name = phase.Type
			newPhase.Size = phase.Size
			newPhase.NrThread = phase.NrThread
			newPhase.ScanSize = phase.ScanSize
			newDB.Phases = append(newDB.Phases, newPhase)
		}
		doc.DB = append(doc.DB, newDB)
	}

	now := time.Now()
	doc.DateTime = fmt.Sprintf("%d年%02d月%02d日\\hspace{0.5em}%02d:%02d", now.Year(), now.Month(), now.Day(), now.Hour(), now.Minute())

	doc.Environ.CPUModel = fmt.Sprintf("%s, %s", getCPUModle(), getCPUCores())
	doc.Environ.RAMSize = getRAMSize()
	doc.Environ.OS = getOSName()
	doc.Environ.Kernel = getKernel()
	doc.Environ.MachineName = getHostName()
}

// FillStats with statistic
func (db *DB) FillStats(stats []Statistic) {
	for i := range db.Phases {
		db.Phases[i].Stat.AverageLatency = stats[i].AverageLatency
		db.Phases[i].Stat.Duration = stats[i].Duration
		db.Phases[i].Stat.MaxLatency = stats[i].MaxLatency
		db.Phases[i].Stat.Throughput = stats[i].Throughput
	}
}

func runCmd(command string, env []string) string {
	cmd := exec.Command("bash", "-c", command)
	if len(env) > 0 {
		cmd.Env = append(cmd.Env, env...)
	}
	cmd.Env = append(cmd.Env, env...)
	output, err := cmd.Output()
	if err != nil {
		panic(err)
	}
	return strings.Trim(string(output), " \n\t")
}

func runCmdRealTimeOutput(command string, env []string) {
	cmd := exec.Command("bash", "-c", command)
	if len(env) > 0 {
		cmd.Env = append(cmd.Env, env...)
	}
	stdout, _ := cmd.StdoutPipe()
	cmd.Start()

	scanner := bufio.NewScanner(stdout)
	for scanner.Scan() {
		m := scanner.Text()
		fmt.Println(m)
	}
	cmd.Wait()
}

// return human readable string about bytes
func getSize(bytes uint64) string {
	factor := float64(1024)
	size := float64(bytes)
	for _, unit := range []string{"B", "KB", "MB", "GB", "TB", "PB"} {
		if size < factor {
			return fmt.Sprintf("%.2f%s", size, unit)
		}
		size = size / factor
	}
	return "???"
}

func getCPUModle() string {
	result := runCmd("cat /proc/cpuinfo | grep 'model name' | uniq", []string{})
	result = strings.Split(result, ":")[1]
	result = strings.Trim(result, " \n\t")
	return result
}

func getRAMSize() string {
	result := runCmd("awk '/MemTotal/ {print $2}' /proc/meminfo", []string{})
	kb, err := strconv.ParseUint(result, 10, 64)
	if err != nil {
		panic(err)
	}
	return getSize(kb * 1000)
}

func getPhysicalCores() string {
	return runCmd("lscpu -p | egrep -v '^#' | sort -u -t, -k 2,4 | wc -l", []string{})
}

func getLogicalCores() string {
	return runCmd("lscpu -p | egrep -v '^#' | wc -l", []string{})
}

func getCPUCores() string {
	logicalCores, err := strconv.Atoi(getLogicalCores())
	if err != nil {
		panic(err)
	}
	physicalCores, err := strconv.Atoi(getPhysicalCores())
	if err != nil {
		panic(err)
	}
	return fmt.Sprintf("%dx%d", logicalCores/physicalCores, physicalCores)
}

func getDiskSize() string {
	return runCmd("df -Ph | grep /dev/pmem0 | awk '{print $2}'", []string{})
}

// https://www.howtoforge.com/how_to_find_out_about_your_linux_distribution
func getOSName() string {
	raw := runCmd("cat /etc/os-release | grep PRETTY_NAME", []string{})
	osName := strings.Split(raw, "=")[1]
	return strings.Trim(osName, " \n\t\"")
}

func getKernel() string {
	return runCmd("uname -r", []string{})
}

func getHostName() string {
	hostName, err := os.Hostname()
	if err != nil {
		panic(err)
	}
	return hostName
}
