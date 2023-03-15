package dbUtils

import (
	"encoding/xml"
	"strconv"
	"strings"
	OSMUtil "transform-osm/osm-utils"

	"github.com/pkg/errors"
)

var XML_TAG_NAME_CONST = xml.Name{Space: " ", Local: "tag"}

// findAndMapAnchorMainSignals identifies Hauptsignal(S/F)-Node pairs, that match up.
// Matching up in this context means, that the Node has the tags 'railway:signal' and 'name:...' where ... is the same
// (excluding spaces) as the Signal-name.
// In notFoundSignals... all signals that could not be identified will be returned.
func findAndMapAnchorMainSignals(
	knoten Spurplanknoten,
	osm *OSMUtil.Osm,
	anchors map[float64][]*OSMUtil.Node,
	notFoundSignalsFalling *[]*NamedSimpleElement,
	notFoundSignalsRising *[]*NamedSimpleElement,
	signalList map[string]OSMUtil.Signal,
	foundAnchorCount *int,
	nodeIdCounter *int,
) error {
	conflictingSignalNames := map[string]bool{}
	err := processHauptsignal(
		knoten,
		notFoundSignalsFalling,
		anchors,
		signalList,
		conflictingSignalNames,
		osm,
		true,
		foundAnchorCount,
		nodeIdCounter,
	)
	if err != nil {
		return errors.Wrap(err, "failed processing falling main signals")
	}

	err = processHauptsignal(
		knoten,
		notFoundSignalsRising,
		anchors,
		signalList,
		conflictingSignalNames,
		osm,
		false,
		foundAnchorCount,
		nodeIdCounter,
	)
	if err != nil {
		return errors.Wrap(err, "failed processing rising main signals")
	}
	return nil
}

// processHauptsignal iterates over all Hautpsignal[S/F] (depending on 'is Falling')
// and does all the identification.
func processHauptsignal(
	knoten Spurplanknoten,
	notFoundSignals *[]*NamedSimpleElement,
	anchors map[float64][]*OSMUtil.Node,
	signalList map[string]OSMUtil.Signal,
	conflictingSignalNames map[string]bool,
	osm *OSMUtil.Osm,
	isFalling bool,
	foundAnchorCount *int,
	nodeIdCounter *int,
) error {
	signals := knoten.HauptsigF
	if !isFalling {
		signals = knoten.HauptsigS
	}

	for _, signal := range signals {
		matchingSignalNodes := []*OSMUtil.Node{}

		for _, node := range osm.Node {
			if len(node.Tag) != 0 {
				railwayTag, _ := OSMUtil.FindTagOnNode(node, "railway")
				refTag, _ := OSMUtil.FindTagOnNode(node, "ref")

				if railwayTag == "signal" &&
					strings.ReplaceAll(refTag, " ", "") == signal.Name.Value {
					matchingSignalNodes = append(matchingSignalNodes, node)
				}
			}
		}

		if len(matchingSignalNodes) == 1 {
			conflictFreeSignal, err := insertNewHauptsignal(
				nodeIdCounter,
				matchingSignalNodes[0],
				signal,
				isFalling,
				notFoundSignals,
				anchors,
				conflictingSignalNames,
				osm,
				foundAnchorCount,
			)
			if err != nil {
				return errors.Wrap(err, "failed to insert signal")
			}
			if conflictFreeSignal {
				*foundAnchorCount++
				signalList[matchingSignalNodes[0].Id] = OSMUtil.Signal{
					Name: signal.Name.Value,
					Lat:  matchingSignalNodes[0].Lat,
					Lon:  matchingSignalNodes[0].Lon,
				}
				return nil
			}
		}
		conflictingSignalNames[signal.Name.Value] = true
		*notFoundSignals = append(*notFoundSignals, signal)
	}
	return nil
}

// insertNewHauptsignal tries to insert a new main signal.
// Inserting is prohibited, if the signal name is already conflicting or a conflict could be found.
// A conflict exists, when there are either multiple Signals with the same name but different kilometrages,
// or when there exists more than one node, that could be identified as a certian Signal (i.e. with the same name).
func insertNewHauptsignal(
	nodeIdCounter *int,
	signalNode *OSMUtil.Node,
	signal *NamedSimpleElement,
	isFalling bool,
	notFound *[]*NamedSimpleElement,
	anchors map[float64][]*OSMUtil.Node,
	conflictingSignalNames map[string]bool,
	osm *OSMUtil.Osm,
	foundAnchorCount *int,
) (bool, error) {
	if conflictingSignalNames[signal.Name.Value] {
		return false, nil
	}
	signalKilometrage, err := formatKilometrageStringInFloat(signal.KnotenTyp.Kilometrierung.Value)
	if err != nil {
		return false, errors.Wrap(err, "failed to format kilometrage")
	}
	for anchorKilometrage, currentAnchors := range anchors {
		for _, possibleAnchor := range currentAnchors {
			if possibleAnchor.Lat == signalNode.Lat && possibleAnchor.Lon == signalNode.Lon && anchorKilometrage != signalKilometrage {
				for _, errorAnchor := range currentAnchors {
					errorSignal := NamedSimpleElement{}
					errorSignal.KnotenTyp = KnotenTyp{
						Kilometrierung: Wert{
							Value: strconv.FormatFloat(anchorKilometrage, 'f', 3, 64),
						},
					}
					errorSignal.Name = Wert{
						Value: errorAnchor.Tag[3].V,
					}
					*notFound = append(*notFound, &errorSignal)
					*foundAnchorCount--

					errorAnchor.Tag = errorAnchor.Tag[:(len(errorAnchor.Tag) - 4)]
				}
				delete(anchors, anchorKilometrage)
				return false, nil
			}
		}
	}

	newSignalNode := createNamedDirectionalNode(
		nodeIdCounter,
		signalNode,
		"ms",
		signal.Name.Value,
		isFalling,
	)
	OSMUtil.InsertNewNodeWithReferenceNode(
		osm,
		&newSignalNode,
		signalNode,
	)

	if len(anchors[signalKilometrage]) == 0 {
		anchors[signalKilometrage] = []*OSMUtil.Node{&newSignalNode}
	} else {
		anchors[signalKilometrage] = append(anchors[signalKilometrage], &newSignalNode)
	}
	return true, nil
}

// mapUnanchoredMainSignals processes all main signals for which no unique Node could be determined.
func mapUnanchoredSignals(
	osmData *OSMUtil.Osm,
	anchors map[float64]([]*OSMUtil.Node),
	signalList map[string]OSMUtil.Signal,
	nodeIdCounter *int,
	knoten Spurplanknoten,
	signalType string,
	elementsNotFound map[string]([]string),
) error {
	err := searchUnanchoredSignal(
		osmData,
		anchors,
		signalList,
		nodeIdCounter,
		knoten,
		signalType,
		elementsNotFound,
		true)
	if err != nil {
		return errors.Wrap(err, "failed finding falling main signal")
	}

	err = searchUnanchoredSignal(
		osmData,
		anchors,
		signalList,
		nodeIdCounter,
		knoten,
		signalType,
		elementsNotFound,
		false)
	if err != nil {
		return errors.Wrap(err, "failed finding rising main signal")
	}
	return nil
}

// serachUnanchoredMainSignal searches for a Node, that best fits the Signal to be mapped.
// This search is based on at least two anchored elements and their respective distance to the signal at hand.
// If no ore only one anchor could be identified, or all anchors are otherwise insufficient, no mapping can be done.
func searchUnanchoredSignal(
	osmData *OSMUtil.Osm,
	anchors map[float64]([]*OSMUtil.Node),
	signalList map[string]OSMUtil.Signal,
	nodeIdCounter *int,
	knoten Spurplanknoten,
	signalType string,
	elementsNotFound map[string]([]string),
	isFalling bool,
) error {
	var signalTypeLong string
	var signals []*NamedSimpleElement

	switch signalType {
	case "ms":
		signalTypeLong = "main signal"
		signals = knoten.HauptsigF
		if !isFalling {
			signals = knoten.HauptsigS
		}
	case "as":
		signalTypeLong = "approach signal"
		signals = knoten.VorsigF
		if !isFalling {
			signals = knoten.VorsigS
		}
	case "ps":
		signalTypeLong = "protection signal"
		signals = knoten.SchutzsigF
		if !isFalling {
			signals = knoten.SchutzsigS
		}
	}

	for _, signal := range signals {
		kilometrage, _ := formatKilometrageStringInFloat(signal.KnotenTyp.Kilometrierung.Value)

		maxNode, err := findBestOSMNode(osmData, anchors, kilometrage)
		if err != nil {
			if errors.Cause(err) == errNoSuitableAnchors {
				elementsNotFound[signalTypeLong+"s"] = append(elementsNotFound[signalTypeLong+"s"], signal.Name.Value)
				continue
			}
			return errors.Wrap(err, "failed to map "+signalTypeLong+" "+signal.Name.Value)

		}

		newSignalNode := createNamedDirectionalNode(
			nodeIdCounter,
			maxNode,
			signalType,
			signal.Name.Value,
			isFalling,
		)
		OSMUtil.InsertNewNodeWithReferenceNode(
			osmData,
			&newSignalNode,
			maxNode,
		)
		signalList[newSignalNode.Id] = OSMUtil.Signal{
			Name: signal.Name.Value,
			Lat:  newSignalNode.Lat,
			Lon:  newSignalNode.Lon,
		}
	}
	return nil
}
