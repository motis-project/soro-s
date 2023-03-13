package osmUtils

import (
	"encoding/xml"
	"os"
	"path/filepath"

	"github.com/pkg/errors"
)

// GenerateStationsAndHalts curates a list of all stations and halts in the OSM-file unter 'inputFilePath.
// It generates both OSM-data (a list of Nodes) and a seperate JSON-structure stored.
func GenerateStationsAndHalts(inputFilePath string, tempFolderPath string) (SearchFile, Osm, error) {
	stationsUnfilteredFilePath, _ := filepath.Abs(tempFolderPath + "/stationsUnfiltered.osm.pbf")
	stationsFile, _ := filepath.Abs(tempFolderPath + "/stations.xml")

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
		"--overwrite",
	})

	data, _ := os.ReadFile(stationsFile)
	var osm Osm
	if err := xml.Unmarshal([]byte(data), &osm); err != nil {
		return SearchFile{}, Osm{}, errors.Wrap(err, "failed unmarshalling osm: "+stationsFile)
	}

	searchFile, stationHaltOsm := generateSearchFile(osm)

	return searchFile, stationHaltOsm, nil
}

// generateSerachFile does all th heavy lifting for GenerateStationsAndHalts, doing all the curating.
func generateSearchFile(osm Osm) (searchFile SearchFile, stationHaltOsm Osm) {
	stations := make(map[string]Station)
	halts := make(map[string]Halt)
	stationHaltsNodes := make([]*Node, 0)

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
