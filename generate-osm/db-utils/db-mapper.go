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
	totalNumberOfAnchors, totalElementsNotFound := 0, 0
	linesWithNoAnchors := []string{}
	linesWithOneAnchor := []string{}

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

		fmt.Printf("Mapping line %s \n", line)

		var notFoundSignalsFalling []*NamedSimpleElement = []*NamedSimpleElement{}
		var notFoundSignalsRising []*NamedSimpleElement = []*NamedSimpleElement{}
		var notFoundSwitches []*Weichenanfang = []*Weichenanfang{}
		var foundAnchorCount = 0
		for _, stelle := range dbIss.Betriebsstellen {
			for _, abschnitt := range stelle.Abschnitte {
				for _, knoten := range abschnitt.Knoten {
					err = findAndMapAnchorMainSignals(
						*knoten,
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
						*knoten,
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
		}

		numElementsNotFound := len(notFoundSignalsFalling) + len(notFoundSignalsRising) + len(notFoundSwitches)
		percentAnchored := ((float64)(foundAnchorCount) / ((float64)(foundAnchorCount) + (float64)(numElementsNotFound))) * 100.0
		fmt.Printf("Could anchor %d/%d (%f%%) of signals and switches. \n", foundAnchorCount, foundAnchorCount+numElementsNotFound, percentAnchored)

		totalNumberOfAnchors += foundAnchorCount
		totalElementsNotFound += numElementsNotFound

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
			linesWithNoAnchors = append(linesWithNoAnchors, line)
			continue
		}
		if len(anchors) == 1 {
			linesWithOneAnchor = append(linesWithOneAnchor, line)
			// TODO: Node not found, find closest mapped Node and work from there
		} else {
			elementsNotFound := make(map[string]([]string))
			for _, stelle := range issWithMappedSignals.Betriebsstellen {
				for _, abschnitt := range stelle.Abschnitte {
					for _, knoten := range abschnitt.Knoten {
						err = mapUnanchoredSignals(
							&osm,
							&anchors,
							&newNodeIdCounter,
							*knoten,
							"ms",
							elementsNotFound,
						)
						if err != nil {
							return errors.Wrap(err, "failed finding main signals")
						}
						err = mapUnanchoredSwitches(
							&osm,
							&anchors,
							&newNodeIdCounter,
							*knoten,
							elementsNotFound,
						)
						if err != nil {
							return errors.Wrap(err, "failed finding switches")
						}
					}

				}
			}

			simpleElements := make(map[string]([]*SimpleElement))
			namedSimpleElements := make(map[string]([]*NamedSimpleElement))

			for _, stelle := range dbIss.Betriebsstellen {
				for _, abschnitt := range stelle.Abschnitte {
					for _, knoten := range abschnitt.Knoten {
						simpleElements["line_switch"] = knoten.Streckenwechsel0
						simpleElements["km_jump"] = knoten.KmSprungAnf
						simpleElements["border"] = knoten.BetriebsStGr
						simpleElements["bumper"] = knoten.Prellbock

						namedSimpleElements["tunnel"] = knoten.Tunnel
						namedSimpleElements["track_end"] = knoten.Gleisende

						for elementName, elementList := range simpleElements {
							err = mapSimpleElement(
								&osm,
								anchors,
								&newNodeIdCounter,
								*knoten,
								elementName,
								elementList,
								elementsNotFound,
							)
							if err != nil {
								return errors.Wrap(err, "failed finding "+elementName)
							}
						}
						for elementName, elementList := range namedSimpleElements {
							err = mapNamedSimpleElement(
								&osm,
								anchors,
								&newNodeIdCounter,
								*knoten,
								elementName,
								elementList,
								elementsNotFound,
							)
							if err != nil {
								return errors.Wrap(err, "failed finding "+elementName)
							}
						}
						for _, signalType := range []string{"as", "ps"} {
							err = mapUnanchoredSignals(
								&osm,
								&anchors,
								&newNodeIdCounter,
								*knoten,
								signalType,
								elementsNotFound,
							)
							if err != nil {
								return errors.Wrap(err, "failed finding "+signalType)
							}
						}

						err = mapCrosses(
							&osm,
							anchors,
							&newNodeIdCounter,
							*knoten,
							elementsNotFound,
						)
						if err != nil {
							return errors.Wrap(err, "failed finding crosses")
						}
						err = mapHalts(
							&osm,
							&anchors,
							&newNodeIdCounter,
							*knoten,
							elementsNotFound,
						)
						if err != nil {
							return errors.Wrap(err, "failed finding halts")
						}
						err = mapSpeedLimits(
							&osm,
							&anchors,
							&newNodeIdCounter,
							*knoten,
							elementsNotFound,
						)
						if err != nil {
							return errors.Wrap(err, "failed finding speed limits")
						}
						err = mapEoTDs(
							&osm,
							&anchors,
							&newNodeIdCounter,
							*knoten,
							elementsNotFound,
						)
						if err != nil {
							return errors.Wrap(err, "failed finding end of train detectors")
						}
						err = mapSlopes(
							&osm,
							&anchors,
							&newNodeIdCounter,
							*knoten,
							elementsNotFound,
						)
						if err != nil {
							return errors.Wrap(err, "failed finding slopes")
						}
					}
				}
			}

			for elementType, nameList := range elementsNotFound {
				fmt.Printf("Could not find %s: %v \n", elementType, nameList)
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

	totalPercentAnchored := ((float64)(totalNumberOfAnchors) / ((float64)(totalNumberOfAnchors) + (float64)(totalElementsNotFound))) * 100.0
	fmt.Printf("Could in total anchor %d/%d (%f%%) of signals and switches. \n", totalNumberOfAnchors, totalNumberOfAnchors+totalElementsNotFound, totalPercentAnchored)
	fmt.Printf("Lines with no anchors: %d out of %d (%v)\n", len(linesWithNoAnchors), len(refs), linesWithNoAnchors)
	fmt.Printf("Lines with only one anchor: %d out of %d (%v)\n", len(linesWithOneAnchor), len(refs), linesWithOneAnchor)
	return nil
}
