package stationsHaltsDisplay

import (
	"encoding/xml"
	"os"
	osmUtils "transform-osm/osm-utils"
)

func StationsHaltsDisplay(stationsFile string) (osmUtils.Osm, map[string]map[string]map[string]string) {
	data, _ := os.ReadFile(stationsFile)
	var osmData osmUtils.Osm
	if err := xml.Unmarshal([]byte(data), &osmData); err != nil {
		panic(err)
	}

	stations := make(map[string]map[string]string)
	halts := make(map[string]map[string]string)
	for _, n := range osmData.Node {
		var name string
		for _, t := range n.Tag {
			if t.K == "name" {
				name = t.V
			}

			if t.K == "railway" && t.V == "station" {
				stations[n.Id] = map[string]string{
					"name": name,
					"lat":  n.Lat,
					"lon":  n.Lon,
				}
				n.Tag = append(n.Tag, &osmUtils.Tag{K: "type", V: "station"})
			}

			if t.K == "railway" && t.V == "halt" {
				halts[n.Id] = map[string]string{
					"name": name,
					"lat":  n.Lat,
					"lon":  n.Lon,
				}
				n.Tag = append(n.Tag, &osmUtils.Tag{K: "type", V: "element"})
				n.Tag = append(n.Tag, &osmUtils.Tag{K: "subtype", V: "hlt"})
			}
		}
	}

	return osmData, map[string]map[string]map[string]string{
		"stations": stations,
		"halts":    halts,
	}
}
