package main

import (
	"bytes"
	"container/list"
	"fmt"
	"hash/fnv"
	"math/rand"
	"os"
	"sync"
	"sync/atomic"
	"time"
)

type bucket struct {
	list list.List
	flag int32
}

type dataGenerator struct {
	data       []bucket
	nrThread   int
	bucketSize uint64
}

func (b *bucket) add(new uint64) bool {
	if atomic.CompareAndSwapInt32(&b.flag, 0, 1) {
		for e := b.list.Front(); e != nil; e = e.Next() {
			if e.Value == new {
				atomic.StoreInt32(&b.flag, 0)
				return false
			}
		}
		b.list.PushBack(new)
		atomic.StoreInt32(&b.flag, 0)
		return true
	}
	return false
}

func (d *dataGenerator) generate(size uint64, wg *sync.WaitGroup, id int, file *os.File, lock *sync.Mutex) {
	seed := fnv.New64a()
	seed.Write([]byte(fmt.Sprintf("%v", time.Now().Unix()+int64(id)*123456)))
	rnd := rand.New(rand.NewSource(int64(seed.Sum64())))
	buffer := bytes.NewBufferString("")
	buffer.Grow(10000)
	i := uint64(0)
	for i < size {
		new := rnd.Uint64()
		bucket := d.data[new%d.bucketSize]
		if bucket.add(new) {
			i++
			buffer.WriteString(fmt.Sprintf("%v\n", new))
			if buffer.Len() > 9970 {
				lock.Lock()
				_, err := buffer.WriteTo(file)
				lock.Unlock()
				if err != nil {
					panic(err)
				}
			}
		}
	}
	if buffer.Len() > 0 {
		_, err := buffer.WriteTo(file)
		if err != nil {
			panic(err)
		}
	}
	wg.Done()
}

// GeneratePlain unique random data and write to file
func GeneratePlain(nrThread int, size uint64, fileName string) {
	file, err := os.Create(fileName)
	if err != nil {
		panic(err)
	}
	defer file.Close()

	d := dataGenerator{}
	d.nrThread = nrThread
	d.bucketSize = size / 8
	d.data = make([]bucket, d.bucketSize)

	lock := sync.Mutex{}

	wg := sync.WaitGroup{}
	wg.Add(d.nrThread)
	defer wg.Wait()

	perThread := size / uint64(d.nrThread)
	for i := 0; i < d.nrThread-1; i++ {
		go d.generate(perThread, &wg, i, file, &lock)
	}
	go d.generate(size-(uint64(d.nrThread)-1)*perThread, &wg, d.nrThread-1, file, &lock)
}

// func main() {
// 	size, err := strconv.ParseUint(os.Args[1], 10, 64)
// 	fmt.Println(size)
// 	if err != nil {
// 		panic(err)
// 	}
// 	GeneratePlain(7, size, "out.dat")
// }
