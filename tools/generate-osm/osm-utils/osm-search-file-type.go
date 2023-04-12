package osmUtils

type SearchFile struct {
	Stations     map[string]Station `json:"stations"`
	Halts        map[string]Halt    `json:"halts"`
	MainSignals  map[string]Signal  `json:"main_signals"`
	OtherSignals map[string]Signal  `json:"other_signals"`
}

type Station struct {
	Name string `json:"name"`
	Lat  string `json:"lat"`
	Lon  string `json:"lon"`
}

type Halt struct {
	Name string `json:"name"`
	Lat  string `json:"lat"`
	Lon  string `json:"lon"`
}

type Signal struct {
	Name string `json:"name"`
	Lat  string `json:"lat"`
	Lon  string `json:"lon"`
}
