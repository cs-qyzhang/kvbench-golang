package main

import (
	"encoding/json"
	"io/ioutil"
)

// Option is read from json file
type Option struct {
	Author         string `json:"author"`
	Title          string `json:"title"`
	ReportTemplate string `json:"reportTemplate"`
	ReportOutput   string `json:"reportOutput"`
	GenerateReport bool   `json:"generateReport"`
	KeyDataFile    string `json:"keyDataFile"`
	ValueDataFile  string `json:"valueDataFile"`
	NrThread       int    `json:"nrThread,omitempty"`
	Phases         []struct {
		Type          string `json:"type"`
		Size          uint64 `json:"size"`
		NrThread      int    `json:"nrThread"`
		ScanSize      uint64 `json:"scanSize,omitempty"`
		KeyDataFile   string `json:"keyDataFile,omitempty"`
		ValueDataFile string `json:"valueDataFile,omitempty"`
	} `json:"phases"`
	DB []struct {
		Name      string   `json:"name"`
		ClassName string   `json:"class,omitempty"`
		KeyType   string   `json:"keyType"`
		ValueType string   `json:"valueType"`
		PreTask   string   `json:"preTask,omitempty"`
		Env       []string `json:"env,omitempty"`
		Task      string   `json:"Task"`
		AfterTask string   `json:"afterTask,omitempty"`
	} `json:"db"`
}

// ReadFromJSON will read options from json file
func (opt *Option) ReadFromJSON(jsonFile string) {
	jsonContent, err := ioutil.ReadFile(jsonFile)
	if err != nil {
		panic(err)
	}
	err = json.Unmarshal(jsonContent, opt)
	if err != nil {
		panic(err)
	}
}
