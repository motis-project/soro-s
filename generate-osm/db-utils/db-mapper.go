package dbUtils

import (
	"encoding/xml"
	"fmt"
	"os"

	OSMUtil "transform-osm/osm-utils"

	"github.com/pkg/errors"
)

// MapDB maps all elements present in DB-data line files in 'DBDir' using the respective OSM line file present in 'osmDir'.
// First, all anchor-able signals and switches are mapped, second all other non-anchor-able elements.
func MapDB(
	refs []string,
	osmDir string,
	DBDir string,
) error {
	newNodeIdCounter := 0
	linesWithNoAnchors := 0
	for _, line := range refs {
		var anchors map[float64]([]*OSMUtil.Node) = map[float64]([]*OSMUtil.Node){}
		var osm OSMUtil.Osm
		var dbIss XmlIssDaten

		osmLineFilePath := osmDir + "/" + line + ".xml"
		osmFile, err := os.ReadFile(osmLineFilePath)
		if err != nil {
			return errors.Wrap(err, "failed reading osm line file: "+osmLineFilePath)
		}
		dbLineFilePath := DBDir + "/" + line + "_DB.xml"
		dbFile, err := os.ReadFile(dbLineFilePath)
		if err != nil {
			return errors.Wrap(err, "failed reading DB line file: "+dbLineFilePath)
		}

		if err := xml.Unmarshal([]byte(osmFile), &osm); err != nil {
			return errors.Wrap(err, "failed unmarshalling osm file: "+osmLineFilePath)
		}
		if err := xml.Unmarshal([]byte(dbFile), &dbIss); err != nil {
			return errors.Wrap(err, "failed unmarshalling db file: "+dbLineFilePath)
		}

		fmt.Printf("Processing line %s \n", line)

		var notFoundSignalsFalling []*Signal = []*Signal{}
		var notFoundSignalsRising []*Signal = []*Signal{}
		var notFoundSwitches []*Weichenanfang = []*Weichenanfang{}
		var foundAnchorCount = 0
		for _, stelle := range dbIss.Betriebsstellen {
			for _, abschnitt := range stelle.Abschnitte {
				err = findAndMapAnchorMainSignals(
					abschnitt,
					&osm,
					anchors,
					&notFoundSignalsFalling,
					&notFoundSignalsRising,
					&foundAnchorCount,
					&newNodeIdCounter,
				)
				if err != nil {
					return errors.Wrap(err, "failed anchoring main signals")
				}
				err = findAndMapAnchorSwitches(
					abschnitt,
					&osm,
					anchors,
					&notFoundSwitches,
					&foundAnchorCount,
					&newNodeIdCounter,
				)
				if err != nil {
					return errors.Wrap(err, "failed anchoring switches")
				}
			}
		}

		numSignalsNotFound := (float64)(len(notFoundSignalsFalling) + len(notFoundSignalsRising))
		percentAnchored := ((float64)(foundAnchorCount) / ((float64)(foundAnchorCount) + numSignalsNotFound)) * 100.0
		fmt.Printf("Could anchor %f %% of signals. \n", percentAnchored)

		var issWithMappedSignals = XmlIssDaten{
			Betriebsstellen: []*Spurplanbetriebsstelle{{
				Abschnitte: []*Spurplanabschnitt{{
					Knoten: []*Spurplanknoten{{
						HauptsigF:  notFoundSignalsFalling,
						HauptsigS:  notFoundSignalsRising,
						WeichenAnf: notFoundSwitches,
					}},
				}},
			}},
		}

		if len(anchors) == 0 {
			fmt.Print("Could not find anchors! \n")
			continue
		}
		if len(anchors) == 1 {
			fmt.Print("Could not find enough anchors! \n")
			// TODO: Node not found, find closest mapped Node and work from there
		} else {
			for _, stelle := range issWithMappedSignals.Betriebsstellen {
				for _, abschnitt := range stelle.Abschnitte {
					mapUnanchoredMainSignals(
						&osm,
						&anchors,
						&newNodeIdCounter,
						*abschnitt,
					)
					mapUnanchoredSwitches(&osm,
						&anchors,
						&newNodeIdCounter,
						*abschnitt,
					)
				}
			}
		}

		if new_Data, err := xml.MarshalIndent(osm, "", "	"); err != nil {
			return errors.Wrap(err, "failed marshalling osm data")
		} else {
			if err := os.WriteFile(osmLineFilePath,
				[]byte(xml.Header+string(new_Data)), 0644); err != nil {
				return errors.Wrap(err, "failed writing file: "+osmLineFilePath)
			}
		}
	}

	fmt.Printf("Lines with no anchors: %d out of %d \n", linesWithNoAnchors, len(refs))
	return nil
}
