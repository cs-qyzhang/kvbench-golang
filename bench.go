package main

import (
	"fmt"
	"strconv"
)

// Bench structure
type Bench struct {
	doc *Doc
	opt *Option
}

func (b *Bench) validDB() bool {
	for _, db := range b.doc.DB {
		output := runCmd(db.Task+" --are-you-kvbench", []string{})
		if output != "YES!" {
			panic(db.Name + "is not a kvbench-compatiable program!!!")
		}
	}
	return true
}

func (db *DB) command() string {
	cmd := db.Task
	for i, phase := range db.Phases {
		if i != 0 {
			arg := phase.Name
			if phase.Name == "scan" {
				arg += strconv.FormatUint(phase.ScanSize, 10)
			}
			arg += "=" + strconv.Itoa(phase.NrThread) + " " + strconv.FormatUint(phase.Size, 10)
			cmd += " " + arg
		}
	}
	return cmd
}

// Run benchmark
func (b *Bench) Run() {
	for _, db := range b.doc.DB {
		// run pre task
		if db.PreTask != "" {
			fmt.Printf("run pre task: %s\n", db.PreTask)
			runCmdRealTimeOutput(db.PreTask, []string{})
		}
		// run task
		task := db.command()
		fmt.Printf("run task: %s\n", task)
		runCmdRealTimeOutput(task, db.Env)
		// run after task
		if db.AfterTask != "" {
			fmt.Printf("run after task: %s\n", db.AfterTask)
			runCmdRealTimeOutput(db.AfterTask, []string{})
		}
		fmt.Println()
	}
}
