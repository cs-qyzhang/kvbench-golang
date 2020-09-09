package main

import (
	"os"
	"text/template"
)

// Tex is used to generate .tex report
type Tex struct {
	templateFile string
	outputFile   string
}

// Generate tex report using doc structure
func (t *Tex) Generate(doc Doc) {
	// function used in template
	funcMap := template.FuncMap{
		"add": func(a, b int) int {
			return a + b
		},
		"newarray": func() []float64 {
			return []float64{}
		},
		"rangearray": func(size int) []int {
			arr := []int{}
			for i := 0; i < size; i++ {
				arr = append(arr, i)
			}
			return arr
		},
		"append": func(slice []float64, elems float64) []float64 {
			return append(slice, elems)
		},
		"assign": func(slice []float64, i int, new float64) []float64 {
			slice[i] = new
			return slice
		},
		"max": func(a, b float64) float64 {
			if a < b {
				return b
			}
			return a
		},
		"min": func(a, b float64) float64 {
			if a > b {
				return b
			}
			return a
		},
	}

	temp := template.New(t.templateFile)
	temp.Funcs(funcMap)
	temp.Delims("{<", ">}")
	temp, err := temp.ParseFiles(t.templateFile)
	if err != nil {
		panic(err)
	}

	// create and open output file
	file, err := os.Create(t.outputFile)
	if err != nil {
		panic(err)
	}

	err = temp.Execute(file, doc)
	if err != nil {
		panic(err)
	}
}
