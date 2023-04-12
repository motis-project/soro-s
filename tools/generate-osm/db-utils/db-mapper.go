package dbUtils

import (
	"encoding/xml"
	"fmt"
	"os"

	"transform-osm/db-utils/mapper"
	OSMUtil "transform-osm/osm-utils"

	"github.com/pkg/errors"
)

// MapDB maps all elements present in DB-data line files in 'DBDir' using the respective OSM line file present in 'osmDir'.
// First, all anchor-able signals and switches are mapped, second all other non-anchor-able elements.
func MapDB(
	refs []string,
	osmDir string,
	DBDir string,
) (map[string]OSMUtil.Halt, map[string]OSMUtil.Signal, map[string]OSMUtil.Signal, int, error) {
	newNodeIdCounter := 100000000000
	totalNumberOfAnchors, totalElementsNotFound := 0, 0
	linesWithNoAnchors := []string{}
	linesWithOneAnchor := []string{}
	haltList := make(map[string]OSMUtil.Halt)
	mainSignalList := make(map[string]OSMUtil.Signal)
	otherSignalList := make(map[string]OSMUtil.Signal)

	for _, line := range refs {
		var anchors map[float64]([]*OSMUtil.Node) = map[float64]([]*OSMUtil.Node){}
		var osm OSMUtil.Osm
		var dbIss mapper.XmlIssDaten

		osmLineFilePath := osmDir + "/" + line + ".xml"
		osmFile, err := os.ReadFile(osmLineFilePath)
		if err != nil {
			return nil, nil, nil, -1, errors.Wrap(err, "failed reading osm line file: "+osmLineFilePath)
		}
		dbLineFilePath := DBDir + "/" + line + "_DB.xml"
		dbFile, err := os.ReadFile(dbLineFilePath)
		if err != nil {
			return nil, nil, nil, -1, errors.Wrap(err, "failed reading DB line file: "+dbLineFilePath)
		}

		if err := xml.Unmarshal([]byte(osmFile), &osm); err != nil {
			return nil, nil, nil, -1, errors.Wrap(err, "failed unmarshalling osm file: "+osmLineFilePath)
		}
		if err := xml.Unmarshal([]byte(dbFile), &dbIss); err != nil {
			return nil, nil, nil, -1, errors.Wrap(err, "failed unmarshalling db file: "+dbLineFilePath)
		}

		fmt.Printf("Mapping line %s \n", line)

		var notFoundSignalsFalling = []*mapper.NamedSimpleElement{}
		var notFoundSignalsRising = []*mapper.NamedSimpleElement{}
		var notFoundSwitches = []*mapper.Weichenanfang{}
		var foundAnchorCount = 0
		for _, stelle := range dbIss.Betriebsstellen {
			for _, abschnitt := range stelle.Abschnitte {
				for _, knoten := range abschnitt.Knoten {
					err = mapper.FindAndMapAnchorMainSignals(
						*knoten,
						&osm,
						anchors,
						&notFoundSignalsFalling,
						&notFoundSignalsRising,
						mainSignalList,
						&foundAnchorCount,
						&newNodeIdCounter,
					)
					if err != nil {
						return nil, nil, nil, -1, errors.Wrap(err, "failed anchoring main signals")
					}

					err = mapper.FindAndMapAnchorSwitches(
						*knoten,
						&osm,
						anchors,
						&notFoundSwitches,
						&foundAnchorCount,
						&newNodeIdCounter,
					)
					if err != nil {
						return nil, nil, nil, -1, errors.Wrap(err, "failed anchoring switches")
					}
				}

			}
		}

		numElementsNotFound := len(notFoundSignalsFalling) + len(notFoundSignalsRising) + len(notFoundSwitches)
		percentAnchored := ((float64)(foundAnchorCount) / ((float64)(foundAnchorCount) + (float64)(numElementsNotFound))) * 100.0
		fmt.Printf("Could anchor %d/%d (%f%%) of signals and switches. \n", foundAnchorCount, foundAnchorCount+numElementsNotFound, percentAnchored)

		totalNumberOfAnchors += foundAnchorCount
		totalElementsNotFound += numElementsNotFound

		var issWithMappedSignals = mapper.XmlIssDaten{
			Betriebsstellen: []*mapper.Spurplanbetriebsstelle{{
				Abschnitte: []*mapper.Spurplanabschnitt{{
					Knoten: []*mapper.Spurplanknoten{{
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
			tracker := mapper.NewNotFoundElementTracker()

			for _, stelle := range issWithMappedSignals.Betriebsstellen {
				for _, abschnitt := range stelle.Abschnitte {
					for _, knoten := range abschnitt.Knoten {
						err = mapper.MapUnanchoredMainSignals(
							&osm,
							anchors,
							mainSignalList,
							&newNodeIdCounter,
							*knoten,
							tracker,
						)
						if err != nil {
							return nil, nil, nil, -1, errors.Wrap(err, "failed finding main signals")
						}
						err = mapper.MapUnanchoredSwitches(
							&osm,
							anchors,
							&newNodeIdCounter,
							*knoten,
							tracker,
						)
						if err != nil {
							return nil, nil, nil, -1, errors.Wrap(err, "failed finding switches")
						}
					}
				}
			}

			simpleElements := make(map[mapper.ElementType]([]*mapper.SimpleElement))
			namedSimpleElements := make(map[mapper.ElementType]([]*mapper.NamedSimpleElement))

			for _, stelle := range dbIss.Betriebsstellen {
				for _, abschnitt := range stelle.Abschnitte {
					for _, knoten := range abschnitt.Knoten {
						simpleElements[mapper.LineSwitch] = knoten.Streckenwechsel0
						simpleElements[mapper.KilometrageJump] = knoten.KmSprungAnf
						simpleElements[mapper.Border] = knoten.BetriebsStGr
						simpleElements[mapper.Bumper] = knoten.Prellbock

						namedSimpleElements[mapper.Tunnel] = knoten.Tunnel
						namedSimpleElements[mapper.TrackEnd] = knoten.Gleisende

						for elementName, elementList := range simpleElements {
							err = mapper.MapSimpleElement(
								&osm,
								anchors,
								&newNodeIdCounter,
								elementList,
								elementName,
								tracker,
							)
							if err != nil {
								return nil, nil, nil, -1, errors.Wrap(err, "failed finding "+elementName.String())
							}
						}
						for elementName, elementList := range namedSimpleElements {
							err = mapper.MapNamedSimpleElement(
								&osm,
								anchors,
								&newNodeIdCounter,
								elementName,
								elementList,
								tracker,
							)
							if err != nil {
								return nil, nil, nil, -1, errors.Wrap(err, "failed finding "+elementName.String())
							}
						}
						err = mapper.MapUnanchoredProtectionSignals(
							&osm,
							anchors,
							otherSignalList,
							&newNodeIdCounter,
							*knoten,
							tracker,
						)
						if err != nil {
							return nil, nil, nil, -1, errors.Wrap(err, "failed finding protection signals")
						}
						err = mapper.MapUnanchoredApproachSignals(
							&osm,
							anchors,
							otherSignalList,
							&newNodeIdCounter,
							*knoten,
							tracker,
						)
						if err != nil {
							return nil, nil, nil, -1, errors.Wrap(err, "failed finding approach signals")
						}

						err = mapper.MapCrosses(
							&osm,
							anchors,
							&newNodeIdCounter,
							*knoten,
							tracker,
						)
						if err != nil {
							return nil, nil, nil, -1, errors.Wrap(err, "failed finding crosses")
						}
						err = mapper.MapHalts(
							&osm,
							anchors,
							haltList,
							&newNodeIdCounter,
							*knoten,
							tracker,
						)
						if err != nil {
							return nil, nil, nil, -1, errors.Wrap(err, "failed finding halts")
						}
						err = mapper.MapSpeedLimits(
							&osm,
							anchors,
							&newNodeIdCounter,
							*knoten,
							tracker,
						)
						if err != nil {
							return nil, nil, nil, -1, errors.Wrap(err, "failed finding speed limits")
						}
						err = mapper.MapEoTDs(
							&osm,
							anchors,
							&newNodeIdCounter,
							*knoten,
							tracker,
						)
						if err != nil {
							return nil, nil, nil, -1, errors.Wrap(err, "failed finding end of train detectors")
						}
						err = mapper.MapSlopes(
							&osm,
							anchors,
							&newNodeIdCounter,
							*knoten,
							tracker,
						)
						if err != nil {
							return nil, nil, nil, -1, errors.Wrap(err, "failed finding slopes")
						}
					}
				}
			}

			tracker.PrintNotFoundElements()
		}

		if new_Data, err := xml.MarshalIndent(osm, "", "	"); err != nil {
			return nil, nil, nil, -1, errors.Wrap(err, "failed marshalling osm data")
		} else {
			if err := os.WriteFile(osmLineFilePath,
				[]byte(xml.Header+string(new_Data)), 0644); err != nil {
				return nil, nil, nil, -1, errors.Wrap(err, "failed writing file: "+osmLineFilePath)
			}
		}
	}

	totalPercentAnchored := ((float64)(totalNumberOfAnchors) / ((float64)(totalNumberOfAnchors) + (float64)(totalElementsNotFound))) * 100.0
	fmt.Printf("Could in total anchor %d/%d (%f%%) of signals and switches. \n", totalNumberOfAnchors, totalNumberOfAnchors+totalElementsNotFound, totalPercentAnchored)
	fmt.Printf("Lines with no anchors: %d out of %d (%v)\n", len(linesWithNoAnchors), len(refs), linesWithNoAnchors)
	fmt.Printf("Lines with only one anchor: %d out of %d (%v)\n", len(linesWithOneAnchor), len(refs), linesWithOneAnchor)
	return haltList, mainSignalList, otherSignalList, newNodeIdCounter, nil
}
