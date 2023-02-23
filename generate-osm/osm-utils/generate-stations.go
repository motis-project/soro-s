package osmUtils

import (
	"encoding/xml"
	"os"
	"path/filepath"
)

func GenerateStationsAndHalts(inputFilePath string, tempFolderPath string) (searchFile SearchFile, stationHaltOsm Osm) {
	stationsUnfilteredFilePath, _ := filepath.Abs(tempFolderPath + "./stationsUnfiltered.osm.pbf")
	stationsFile, _ := filepath.Abs(tempFolderPath + "./stations.xml")

	ExecuteOsmFilterCommand([]string{
		inputFilePath,
		"-o",
		stationsUnfilteredFilePath,
		"n/railway=station,halt,facility",
		"--overwrite",
	})
	ExecuteOsmFilterCommand([]string{
		stationsUnfilteredFilePath,
		"-o",
		stationsFile,
		"-i",
		"n/subway=yes",
		"n/monorail=yes",
		"n/usage",
		"n/tram=yes",
		"--overwrite",
	})

	data, _ := os.ReadFile(stationsFile)
	var osm Osm
	if err := xml.Unmarshal([]byte(data), &osm); err != nil {
		panic(err)
	}

	return generateSearchFile(osm)
}

func generateSearchFile(osm Osm) (searchFile SearchFile, stationHaltOsm Osm) {
	stations := make(map[string]Station)
	halts := make(map[string]Halt)
	stationHaltsNodes := make([]*Node, 0)
	osm.Node = append(osm.Node, getWiesbadenHbfAsNode())

	for _, node := range osm.Node {
		var name string = ""
		for _, t := range node.Tag {
			if t.K == "name" {
				name = t.V
			}

			if name != "" && t.K == "railway" {
				if t.V == "station" {
					stations[node.Id] = Station{
						Name: name,
						Lat:  node.Lat,
						Lon:  node.Lon,
					}
					node.Tag = append(node.Tag, &Tag{K: "type", V: "station"})
				}
				if t.V == "halt" {
					halts[node.Id] = Halt{
						Name: name,
						Lat:  node.Lat,
						Lon:  node.Lon,
					}
					node.Tag = append(node.Tag, &Tag{K: "type", V: "element"})
					node.Tag = append(node.Tag, &Tag{K: "subtype", V: "hlt"})
				}
				stationHaltsNodes = append(stationHaltsNodes, node)
			}
		}
	}

	return SearchFile{
			Stations: stations,
			Halts:    halts,
		}, Osm{
			Node: stationHaltsNodes,
		}
}

func getWiesbadenHbfAsNode() *Node {
	return &Node{
		Id:        "420133769",
		Version:   "1",
		Timestamp: "2021-03-01T14:00:00Z",
		Lat:       "50.06988",
		Lon:       "8.24404",
		Tag: []*Tag{
			{K: "name", V: "Wiesbaden Hauptbahnhof"},
			{K: "railway", V: "station"},
			{K: "railway:ref", V: "FW"},
			{K: "public_transport", V: "station"},
		},
	}
}
