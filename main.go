package main

func main() {
	opt := Option{}
	opt.ReadFromJSON("kvbench.json")

	doc := Doc{}
	doc.InitFromOption(opt)

	bench := Bench{&doc, &opt}
	bench.validDB()
	bench.Run()

	for _, db := range doc.DB {
		stats := ReadStatisticsFromJSON("/home/qyzhang/Projects/kvbench-golang/stat.json")
		db.FillStats(stats)
	}

	tex := Tex{"template.tex", "out.tex"}
	tex.Generate(doc)
}
