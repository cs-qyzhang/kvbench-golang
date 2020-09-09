package main

import (
	"encoding/json"
	"io/ioutil"
)

// Statistic is used to read stat.json
type Statistic struct {
	AverageLatency float64 `json:"averageLatency"`
	Duration       float64 `json:"duration"`
	MaxLatency     float64 `json:"maxLatency"`
	Name           string  `json:"name"`
	Throughput     float64 `json:"throughput"`
	Total          uint64  `json:"total"`
}

// ReadStatisticsFromJSON will read options from json file
func ReadStatisticsFromJSON(jsonFile string) (stats []Statistic) {
	jsonContent, err := ioutil.ReadFile(jsonFile)
	if err != nil {
		panic(err)
	}
	err = json.Unmarshal(jsonContent, &stats)
	if err != nil {
		panic(err)
	}
	return
}
