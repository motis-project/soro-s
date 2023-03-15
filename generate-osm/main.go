package main

import (
	"encoding/json"
	"encoding/xml"
	"fmt"
	"log"
	"os"
	"path/filepath"
	"strconv"
	"strings"

	combineLines "transform-osm/combine-lines"
	dbUtils "transform-osm/db-utils"
	osmUtils "transform-osm/osm-utils"

	"github.com/pkg/errors"
	"github.com/urfave/cli/v2"
)

func main() {
	os.Mkdir("./temp", 0755)
	var generateLines bool
	var mapDB bool
	var inputFile string
	var outputFile string

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
			&cli.BoolFlag{
				Name:        "map-DB",
				Aliases:     []string{"mdb"},
				Usage:       "Generate lines all lines new and map DB data",
				Destination: &mapDB,
			},
			&cli.StringFlag{
				Name:        "input",
				Aliases:     []string{"i"},
				Value:       "./temp/base.osm.pbf",
				Usage:       "The input file to read as OSM PBF file",
				Destination: &inputFile,
			},
			&cli.StringFlag{
				Name:        "output",
				Aliases:     []string{"o"},
				Value:       "./finalOsm.xml",
				Usage:       "The output file to write result to as XML file",
				Destination: &outputFile,
			},
		},
		Action: func(cCtx *cli.Context) error {
			if err := generateOsm(generateLines, mapDB, inputFile, outputFile); err != nil {
				return err
			}

			return nil
		},
	}

	if err := app.Run(os.Args); err != nil {
		log.Fatal(err)
	}
}

func generateOsm(generateLines bool, mapDB bool, inputFile string, outputFile string) error {
	if !filepath.IsAbs(inputFile) {
		inputFile, _ = filepath.Abs(inputFile)
	}
	if _, err := os.Stat(inputFile); err != nil {
		return errors.Wrap(err, "input file does not exist: "+inputFile)
	}
	if filepath.Ext(inputFile) != ".pbf" {
		return errors.New("input file is not a PBF file: " + inputFile)
	}
	tempFolderPath, _ := filepath.Abs("./temp")
	refs, err := osmUtils.GenerateOsmTrackRefs(inputFile, tempFolderPath)
	if err != nil {
		return errors.Wrap(err, "failed to get ref ids")
	}

	tracksFilePath, _ := filepath.Abs(tempFolderPath + "/tracks.osm.pbf")
	osmUtils.ExecuteOsmFilterCommand([]string{
		inputFile,
		"-o",
		tracksFilePath,
		"r/route=tracks,railway",
		"--overwrite",
	})

	searchFile := osmUtils.SearchFile{}
	maxNewNodeID := -1

	tempLinesDir, _ := filepath.Abs(tempFolderPath + "/lines")
	tempDBLinesDir, _ := filepath.Abs(tempFolderPath + "/DBLines")
	tempDBResoucesDir, _ := filepath.Abs(tempFolderPath + "/DBResources")
	if generateLines || mapDB {
		if err = os.RemoveAll(tempLinesDir); err != nil {
			return errors.Wrap(err, "failed to remove lines folder")
		}
		if err = os.RemoveAll(tempDBLinesDir); err != nil {
			return errors.Wrap(err, "failed to remove DBLines folder")
		}
		if err = os.Mkdir(tempLinesDir, 0755); err != nil {
			return errors.Wrap(err, "failed to create lines folder")
		}

		for _, refId := range refs {
			lineOsmFile, err := filepath.Abs(tempLinesDir + "/" + refId + ".xml")
			if err != nil {
				return errors.Wrap(err, "failed to get line xml file path for ref: "+refId)
			}

			osmUtils.ExecuteOsmFilterCommand([]string{
				tracksFilePath,
				"-o",
				lineOsmFile,
				"ref=" + refId,
				"--overwrite",
			})

		}

		if mapDB {
			if _, err := os.Stat("./temp/DBResources"); errors.Is(err, os.ErrNotExist) {
				return errors.Wrap(err, "DBResource-director does not exists, please first create temp/DBResources")
			}
			relevant_refs, err := dbUtils.Parse(refs, tempDBLinesDir, tempDBResoucesDir)
			if err != nil {
				return errors.Wrap(err, "failed parsing DB data")
			}
			var haltList map[string]osmUtils.Halt
			var mainSignalList map[string]osmUtils.Signal
			var otherSignalList map[string]osmUtils.Signal
			haltList, mainSignalList, otherSignalList, maxNewNodeID, err = dbUtils.MapDB(relevant_refs, tempLinesDir, tempDBLinesDir)
			if err != nil {
				return errors.Wrap(err, "failed mapping DB data")
			}
			searchFile.Halts = haltList
			searchFile.MainSignals = mainSignalList
			searchFile.OtherSignals = otherSignalList
		}

		fmt.Println("Generated all lines")
	}

	// Combine all the lines into one file
	osmData, err := combineLines.CombineAllLines(tempLinesDir)

	if err != nil {
		errorMessageSplit := strings.Split(err.Error(), ":")
		if errorMessageSplit[0] == combineLines.ErrLinesDirNotFound.Error() {
			return errors.New("you need to generate lines first")
		}

		return errors.Wrap(err, "failed to combine lines")
	}

	osmData.Version = "0.6"
	osmData.Generator = "osmium/1.14.0"

	sortedOsmData := osmUtils.SortOsm(osmData)

	if mapDB {
		outputOSM, err := insertOsm(sortedOsmData, outputFile, maxNewNodeID)
		if err != nil {
			return errors.Wrap(err, "failed inserting mapped DB data")
		}
		outputFile = outputFile[:len(outputFile)-4] + "DB.xml"

		stationsList, stationHaltOsm, err := osmUtils.GenerateStations(inputFile, tempFolderPath)
		if err != nil {
			return errors.Wrap(err, "failed generating stations and halts")
		}
		searchFile.Stations = stationsList
		searchFileJsonPath, _ := filepath.Abs(outputFile[:len(outputFile)-4] + ".json")
		err = saveSearchFile(searchFile, searchFileJsonPath)
		if err != nil {
			return errors.Wrap(err, "failed writing stations JSON")
		}

		for i, node := range outputOSM.Node {
			value, found := osmUtils.FindTagOnNode(node, "railway")

			if found == nil {
				if value == "station" || value == "halt" {
					outputOSM.Node = append(outputOSM.Node[:i], outputOSM.Node[i+1:]...)
				}
			}
		}
		outputOSM.Node = append(outputOSM.Node, stationHaltOsm.Node...)

		sortedOsmData = osmUtils.SortOsm(outputOSM)
	}

	output, err := xml.MarshalIndent(sortedOsmData, "", "     ")
	if err != nil {
		return errors.Wrap(err, "failed marshalling final data")
	}
	output = []byte(xml.Header + string(output))
	err = os.WriteFile(outputFile, output, 0644)
	if err != nil {
		return errors.Wrap(err, "failed writing final osm file: "+outputFile)
	}
	return nil
}

func insertOsm(modifiedOsm osmUtils.Osm, outputFilePath string, maxNewNodeID int) (osmUtils.Osm, error) {
	outputData, err := os.ReadFile(outputFilePath)
	if err != nil {
		return osmUtils.Osm{}, errors.Wrap(err, "failed reading output file "+outputFilePath)
	}
	var outputOSM = osmUtils.Osm{}
	err = xml.Unmarshal(outputData, &outputOSM)
	if err != nil {
		return osmUtils.Osm{}, errors.Wrap(err, "failed unmarshalling output file "+outputFilePath)
	}

	newNodes := make(map[string]bool)

	for _, modifiedWay := range modifiedOsm.Way {
		for _, outputWay := range outputOSM.Way {
			if modifiedWay.Id == outputWay.Id {
				for _, node := range modifiedWay.Nd {
					nodeID, _ := strconv.Atoi(node.Ref)
					if nodeID <= maxNewNodeID {
						newNodes[node.Ref] = true
					}
				}
				outputWay.Tag = modifiedWay.Tag
			}
		}
	}

	for nodeID := range newNodes {
		node, err := osmUtils.GetNodeById(&modifiedOsm, nodeID)
		if err != nil {
			return osmUtils.Osm{}, errors.Wrap(err, "failed to find node "+nodeID)
		}
		outputOSM.Node = append(outputOSM.Node, node)
	}
	return outputOSM, nil
}

func saveSearchFile(searchFile osmUtils.SearchFile, searchFileJsonPath string) error {
	output, err := json.MarshalIndent(searchFile, "", "     ")
	if err != nil {
		return errors.Wrap(err, "failed marshalling stations JSON file")
	}
	err = os.WriteFile(searchFileJsonPath, output, 0644)
	if err != nil {
		return errors.Wrap(err, "failed writing stations JSON file: "+searchFileJsonPath)
	}
	return nil
}
