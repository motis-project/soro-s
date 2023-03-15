package osmUtils

import (
	"encoding/xml"
	"os"
	"path/filepath"

	"github.com/pkg/errors"
)

// GenerateStationsAndHalts curates a list of all stations and halts in the OSM-file unter 'inputFilePath.
// It generates both OSM-data (a list of Nodes) and a seperate JSON-structure stored.
func GenerateStations(inputFilePath string, tempFolderPath string) (map[string]Station, Osm, error) {
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
		return nil, Osm{}, errors.Wrap(err, "failed unmarshalling osm: "+stationsFile)
	}

	searchFile, stationHaltOsm := generateSearchFile(osm)

	return searchFile, stationHaltOsm, nil
}

// generateSerachFile does all th heavy lifting for GenerateStationsAndHalts, doing all the curating.
func generateSearchFile(osm Osm) (stationsList map[string]Station, stationHaltOsm Osm) {
	stations := make(map[string]Station)
	stationHaltsNodes := make([]*Node, 0)

	for _, node := range osm.Node {
		var name string = ""
		for _, t := range node.Tag {
			if t.K == "name" {
				name = t.V
			}

			if name != "" && t.K == "railway" {
				if t.V == "station" || t.V == "halt" {
					stations[node.Id] = Station{
						Name: name,
						Lat:  node.Lat,
						Lon:  node.Lon,
					}
					node.Tag = append(node.Tag, &Tag{K: "type", V: "station"})
				}
				stationHaltsNodes = append(stationHaltsNodes, node)
			}
		}
	}

	return stations, Osm{
		Node: stationHaltsNodes,
	}
}
