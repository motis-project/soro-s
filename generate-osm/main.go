package main

import (
	"encoding/json"
	"encoding/xml"
	"fmt"
	"os"
	"path/filepath"
	combineLines "transform-osm/combine-lines"
	osmUtils "transform-osm/osm-utils"
	stationsHaltsDisplay "transform-osm/stations-halts-display"
)

func main() {
	generateLines := true
	os.Mkdir("./temp", 0755)
	baseOsm, _ := filepath.Abs("./temp/base.osm.pbf")
	tracksWithoutRelationsFile, _ := filepath.Abs("./temp/tracksWithoutRelations.osm.pbf")
	osmUtils.ExecuteOsmFilterCommand([]string{
		"-R",
		baseOsm,
		"-o",
		tracksWithoutRelationsFile,
		"r/route=tracks",
		"--overwrite",
	})
	tracksFile, _ := filepath.Abs("./temp/tracks.osm.pbf")
	osmUtils.ExecuteOsmFilterCommand([]string{
		baseOsm,
		"-o",
		tracksFile,
		"r/route=tracks",
		"--overwrite",
	})

	refOutputFile, _ := filepath.Abs("./temp/trackRefs.xml")
	osmUtils.ExecuteOsmFilterCommand([]string{
		tracksWithoutRelationsFile,
		"-o",
		refOutputFile,
		"r/ref",
		"--overwrite",
	})

	refs := getRefIds(refOutputFile)
	if generateLines {
		os.RemoveAll("./temp/lines")
		os.Mkdir("./temp/lines", 0755)
		for i, refId := range refs {
			lineOsmFile, _ := filepath.Abs("./temp/lines/" + refId + ".xml")
			osmUtils.ExecuteOsmFilterCommand([]string{
				tracksFile,
				"-o",
				lineOsmFile,
				"ref=" + refId,
				"--overwrite",
			})
			fmt.Printf("Processed %d/%d: %s\r", i+1, len(refs), refId)
		}
	}

	// Combine all the lines into one file
	osmData := combineLines.CombineAllLines()
	osmData.Version = "0.6"
	osmData.Generator = "osmium/1.14.0"

	// Create stations file
	stattionsUnfilteredFile, _ := filepath.Abs("./temp/stationsUnfiltered.osm.pbf")
	stattionsUnfilteredV2File, _ := filepath.Abs("./temp/stationsUnfilteredV2.osm.pbf")
	stationsFile, _ := filepath.Abs("./temp/stations.xml")
	osmUtils.ExecuteOsmFilterCommand([]string{
		baseOsm,
		"-o",
		stattionsUnfilteredFile,
		"n/public_transport=station,stop_position",
		"--overwrite",
	})
	osmUtils.ExecuteOsmFilterCommand([]string{
		stattionsUnfilteredFile,
		"-o",
		stattionsUnfilteredV2File,
		"n/railway=station,halt",
		"--overwrite",
	})
	osmUtils.ExecuteOsmFilterCommand([]string{
		stattionsUnfilteredV2File,
		"-o",
		stationsFile,
		"-i",
		"n/subway=yes",
		"n/monorail=yes",
		"n/usage",
		"n/tram=yes",
		"--overwrite",
	})

	stationsOsm, jsonData := stationsHaltsDisplay.StationsHaltsDisplay(stationsFile)
	// save stations as json
	output, err := json.MarshalIndent(jsonData, "", "     ")
	if err != nil {
		fmt.Printf("error: %v\n", err)
	}
	os.WriteFile("./temp/stations.json", output, 0644)

	osmData.Way = append(osmData.Way, stationsOsm.Way...)
	osmData.Node = append(osmData.Node, stationsOsm.Node...)
	osmData.Relation = append(osmData.Relation, stationsOsm.Relation...)
	fmt.Println("Version: ", osmData.Version)
	fmt.Println("Generator: ", osmData.Generator)

	sortedOsmData := osmUtils.SortOsm(osmData)
	output, err = xml.MarshalIndent(sortedOsmData, "", "     ")
	if err != nil {
		fmt.Printf("error: %v\n", err)
	}
	output = []byte(xml.Header + string(output))
	os.WriteFile("./temp/finalOsm.xml", output, 0644)
}

func getRefIds(trackRefFile string) []string {
	data, _ := os.ReadFile(trackRefFile)
	var osmData osmUtils.Osm
	if err := xml.Unmarshal([]byte(data), &osmData); err != nil {
		panic(err)
	}
	var refs []string
	for _, s := range osmData.Relation {
		for _, m := range s.Tag {
			if m.K == "ref" {
				refs = append(refs, m.V)
			}
		}
	}

	return refs
}
