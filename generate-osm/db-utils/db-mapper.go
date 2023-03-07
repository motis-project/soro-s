package dbUtils

import (
	"encoding/xml"
	"fmt"
	"log"
	"os"

	OSMUtil "transform-osm/osm-utils"
)

func MapDB(
	refs []string,
	osmDir string,
	DBDir string,
) {
	newNodeIdCounter := 0
	linesWithNoAnchors := 0
	for _, line := range refs {
		var anchors map[string]([]*OSMUtil.Node) = map[string]([]*OSMUtil.Node){}
		var osm OSMUtil.Osm
		var dbIss XmlIssDaten
		osmFile, err := os.ReadFile(osmDir + "/" + line + ".xml")
		if err != nil {
			log.Fatal(err)
		}
		dbFile, err := os.ReadFile(DBDir + "/" + line + "_DB.xml")
		if err != nil {
			log.Fatal(err)
		}

		if err := xml.Unmarshal([]byte(osmFile), &osm); err != nil {
			panic(err)
		}
		if err := xml.Unmarshal([]byte(dbFile), &dbIss); err != nil {
			panic(err)
		}

		fmt.Printf("Processing line %s \n", line)
		var notFoundSignalsFalling []*Signal = []*Signal{}
		var notFoundSignalsRising []*Signal = []*Signal{}
		var foundAnchorCount = 0
		for _, stelle := range dbIss.Betriebsstellen {
			for _, abschnitt := range stelle.Abschnitte {
				findAndMapAnchorMainSignals(
					abschnitt,
					&osm,
					anchors,
					&notFoundSignalsFalling,
					&notFoundSignalsRising,
					&newNodeIdCounter,
				)

				findAndMapAnchorSwitches(
					abschnitt,
					&osm,
					anchors,
					&foundAnchorCount,
					&newNodeIdCounter,
				)
			}
		}
		if foundAnchorCount == 0 {
			fmt.Printf("Found %d anchors \n", foundAnchorCount)
			linesWithNoAnchors++
		}
		var issWithMappedSignals = XmlIssDaten{
			Betriebsstellen: []*Spurplanbetriebsstelle{{
				Abschnitte: []*Spurplanabschnitt{{
					Knoten: []*Spurplanknoten{{
						HauptsigF: notFoundSignalsFalling,
						HauptsigS: notFoundSignalsRising,
					}},
				}},
			}},
		}
		_ = issWithMappedSignals

		if new_Data, err := xml.MarshalIndent(osm, "", "	"); err != nil {
			panic(err)
		} else {
			if err := os.WriteFile(osmDir+"/"+line+".xml",
				[]byte(xml.Header+string(new_Data)), 0644); err != nil {
				panic(err)
			}
		}
	}

	fmt.Printf("Lines with no anchors: %d out of %d \n", linesWithNoAnchors, len(refs))
}
