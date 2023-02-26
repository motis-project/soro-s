package main

import (
	"encoding/json"
	"encoding/xml"
	"errors"
	"fmt"
	"log"
	"os"
	"path/filepath"
	combineLines "transform-osm/combine-lines"
	dbUtils "transform-osm/db-utils"
	osmUtils "transform-osm/osm-utils"

	// Mapper "transform-osm/map-db"

	"github.com/urfave/cli/v2"
)

func main() {
	os.Mkdir("./temp", 0755)
	var generateLines bool
	var inputFile string

	app := &cli.App{
		Name:  "generate-osm",
		Usage: "Generate OSM file from OSM PBF file and DB Data",
		Flags: []cli.Flag{
			&cli.BoolFlag{
				Name:        "generate-lines",
				Aliases:     []string{"gl"},
				Usage:       "Generate lines all lines new",
				Destination: &generateLines,
			},
			&cli.StringFlag{
				Name:        "input",
				Aliases:     []string{"i"},
				Value:       "./temp/base.osm.pbf",
				Usage:       "The input file to read as OSM PBF file",
				Destination: &inputFile,
			},
		},
		Action: func(cCtx *cli.Context) error {
			if err := generateOsm(generateLines, inputFile); err != nil {
				return err
			}

			return nil
		},
	}

	if err := app.Run(os.Args); err != nil {
		log.Fatal(err)
	}
}

func generateOsm(generateLines bool, inputFile string) error {
	if !filepath.IsAbs(inputFile) {
		inputFile, _ = filepath.Abs(inputFile)
	}
	if _, err := os.Stat(inputFile); err != nil {
		return errors.New("Input file does not exist: " + inputFile)
	}
	if filepath.Ext(inputFile) != ".pbf" {
		return errors.New("Input file is not a PBF file: " + inputFile)
	}
	tempFolderPath, _ := filepath.Abs("./temp")
	refs, err := osmUtils.GenerateOsmTrackRefs(inputFile, tempFolderPath)
	if err != nil {
		return errors.New("Failed to get ref ids: " + err.Error())
	}

	tracksFilePath, _ := filepath.Abs(tempFolderPath + "/tracks.osm.pbf")
	osmUtils.ExecuteOsmFilterCommand([]string{
		inputFile,
		"-o",
		tracksFilePath,
		"r/route=tracks",
		"--overwrite",
	})

	tempLinesDir, _ := filepath.Abs(tempFolderPath + "/lines")
	tempDBLinesDir, _ := filepath.Abs(tempFolderPath + "/DBLines")
	tempDBResoucesDir, _ := filepath.Abs(tempFolderPath + "/DBResources")
	if generateLines {
		if err = os.RemoveAll(tempLinesDir); err != nil {
			return errors.New("Failed to remove lines folder: " + err.Error())
		}
		if err = os.RemoveAll(tempDBLinesDir); err != nil {
			return errors.New("Failed to remove DBLines folder: " + err.Error())
		}
		if err = os.Mkdir(tempLinesDir, 0755); err != nil {
			return errors.New("Failed to create lines folder: " + err.Error())
		}
		if _, err := os.Stat(tempDBResoucesDir); os.IsNotExist(err) {
		    if err = os.Mkdir(tempDBResoucesDir, 0755); err != nil {
                return errors.New("Failed to create DBResources folder: " + err.Error())
            }
		}

		for _, refId := range refs {
			lineOsmFile, err := filepath.Abs(tempLinesDir + "/" + refId + ".xml")
			if err != nil {
				return errors.New("Failed to get line file path: " + err.Error())
			}
			osmUtils.ExecuteOsmFilterCommand([]string{
				tracksFilePath,
				"-o",
				lineOsmFile,
				"ref=" + refId,
				"--overwrite",
			})
		}

		relevant_refs := dbUtils.Parse(refs, tempDBLinesDir, tempDBResoucesDir)

		print(relevant_refs)
		print("\n")

		// Mapper.MapDB(relevant_refs, lineDir, db_lineDir)

		fmt.Println("Generated all lines")
	}

	// Combine all the lines into one file
	osmData, err := combineLines.CombineAllLines(tempLinesDir)

	if err != nil && errors.Is(err, combineLines.ErrLinesDirNotFound) {
		return errors.New("you need to generate lines first")
	} else if err != nil {
		return errors.New("failed to combine lines: " + err.Error())
	}
	osmData.Version = "0.6"
	osmData.Generator = "osmium/1.14.0"

	searchFile, stationHaltOsm := osmUtils.GenerateStationsAndHalts(inputFile, tempFolderPath)
	searchFileJsonPath, _ := filepath.Abs(tempFolderPath + "/searchFile.json")
	saveSearchFile(searchFile, searchFileJsonPath)

	for i, node := range osmData.Node {
		found, value := osmUtils.FindTagOnNode(node, "railway")

		if found {
			if value == "station" || value == "halt" {
				osmData.Node = append(osmData.Node[:i], osmData.Node[i+1:]...)
			}
		}
	}
	osmData.Node = append(osmData.Node, stationHaltOsm.Node...)

	sortedOsmData := osmUtils.SortOsm(osmData)
	output, err := xml.MarshalIndent(sortedOsmData, "", "     ")
	if err != nil {
		fmt.Printf("error: %v\n", err)
	}
	output = []byte(xml.Header + string(output))
	os.WriteFile("./temp/finalOsm.xml", output, 0644)

	return nil
}

func saveSearchFile(searchFile osmUtils.SearchFile, searchFileJsonPath string) {
	output, err := json.MarshalIndent(searchFile, "", "     ")
	if err != nil {
		fmt.Printf("error: %v\n", err)
	}
	os.WriteFile(searchFileJsonPath, output, 0644)
}
