package dbUtils

import (
	"encoding/xml"
	"fmt"
	"math"
	"sort"
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
	abschnitt *Spurplanabschnitt,
	osm *OSMUtil.Osm,
	anchors map[float64][]*OSMUtil.Node,
	notFoundSignalsFalling *[]*Signal,
	notFoundSignalsRising *[]*Signal,
	foundAnchorCount *int,
	optionalNewId *int,
) error {
	conflictingSignalNames := map[string]bool{}
	for _, knoten := range abschnitt.Knoten {
		err := processHauptsignal(
			*knoten,
			notFoundSignalsFalling,
			anchors,
			&conflictingSignalNames,
			osm,
			true,
			foundAnchorCount,
			optionalNewId,
		)
		if err != nil {
			return errors.Wrap(err, "failed processing falling main signals")
		}
		err = processHauptsignal(
			*knoten,
			notFoundSignalsRising,
			anchors,
			&conflictingSignalNames,
			osm,
			false,
			foundAnchorCount,
			optionalNewId,
		)
		if err != nil {
			return errors.Wrap(err, "failed processing rising main signals")
		}
	}
	return nil
}

// processHauptsignal iterates over all Hautpsignal[S/F] (depending on 'is Falling')
// and does all the identification.
func processHauptsignal(
	knoten Spurplanknoten,
	notFoundSignals *[]*Signal,
	anchors map[float64][]*OSMUtil.Node,
	conflictingSignalNames *map[string]bool,
	osm *OSMUtil.Osm,
	isFalling bool,
	foundAnchorCount *int,
	optionalNewId *int,
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

		if len(matchingSignalNodes) > 1 {
			(*conflictingSignalNames)[signal.Name.Value] = true
		} else if len(matchingSignalNodes) == 1 {
			conflictFreeSignal, err := insertNewHauptsignal(
				optionalNewId,
				matchingSignalNodes[0],
				signal,
				isFalling,
				notFoundSignals,
				anchors,
				*conflictingSignalNames,
				osm,
				foundAnchorCount,
			)
			if err != nil {
				return errors.Wrap(err, "failed to insert signal")
			}
			if conflictFreeSignal {
				*foundAnchorCount++
				return nil
			}
			(*conflictingSignalNames)[signal.Name.Value] = true
		}
		*notFoundSignals = append(*notFoundSignals, signal)
	}
	return nil
}

// insertNewHauptsignal tries to insert a new main signal.
// Inserting is prohibited, if the signal name is already conflicting or a conflict could be found.
// A conflict exists, when there are either multiple Signals with the same name but different kilometrages,
// or when there exists more than one node, that could be identified as a certian Signal (i.e. with the same name).
func insertNewHauptsignal(
	newId *int,
	signalNode *OSMUtil.Node,
	signal *Signal,
	isFalling bool,
	notFound *[]*Signal,
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
					errorSignal := Signal{}
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
	newSignalNode := createNewHauptsignal(
		newId,
		signalNode,
		signal,
		isFalling,
	)
	OSMUtil.InsertNewNodeWithReferenceNode(osm, &newSignalNode, signalNode)
	if len(anchors[signalKilometrage]) == 0 {
		anchors[signalKilometrage] = []*OSMUtil.Node{&newSignalNode}
	} else {
		anchors[signalKilometrage] = append(anchors[signalKilometrage], &newSignalNode)
	}
	return true, nil
}

// createNewHauptsignal creates a new OSM-Node with the following tags:
// 'type:element', 'subtype:ms', 'id:(Signal name)' and 'direction:...' where ... depends on 'isFalling'.
func createNewHauptsignal(
	id *int,
	node *OSMUtil.Node,
	signal *Signal,
	isFalling bool,
) OSMUtil.Node {
	directionString := "falling"
	if !isFalling {
		directionString = "rising"
	}
	*id++

	return OSMUtil.Node{
		Id:  strconv.Itoa(*id),
		Lat: node.Lat,
		Lon: node.Lon,
		Tag: []*OSMUtil.Tag{
			{XMLName: XML_TAG_NAME_CONST, K: "type", V: "element"},
			{XMLName: XML_TAG_NAME_CONST, K: "subtype", V: "ms"},
			{XMLName: XML_TAG_NAME_CONST, K: "id", V: signal.Name.Value},
			{XMLName: XML_TAG_NAME_CONST, K: "direction", V: directionString},
		},
	}
}

// mapUnanchoredMainSignals processes all main signals for which no unique Node could be determined.
func mapUnanchoredMainSignals(
	osmData *OSMUtil.Osm,
	anchors *map[float64]([]*OSMUtil.Node),
	nodeIdCounter *int,
	abschnitt Spurplanabschnitt,
) {
	for _, knoten := range abschnitt.Knoten {
		searchUnanchoredMainSignal(
			osmData,
			anchors,
			nodeIdCounter,
			*knoten,
			true)
		searchUnanchoredMainSignal(
			osmData,
			anchors,
			nodeIdCounter,
			*knoten,
			false)
	}
}

// serachUnanchoredMainSignal searches for a Node, that best fits the Signal to be mapped.
// This search is based on at least two anchored elements and their respective distance to the signal at hand.
// If no ore only one anchor could be identified, or all anchors are otherwise insufficient, no mapping can be done.
func searchUnanchoredMainSignal(
	osmData *OSMUtil.Osm,
	anchors *map[float64]([]*OSMUtil.Node),
	nodeIdCounter *int,
	knoten Spurplanknoten,
	isFalling bool,
) {
	if len(*anchors) == 0 {
		fmt.Print("Could not find anchors! \n")
		return
	}
	if len(*anchors) == 1 {
		fmt.Print("Could not find enough anchors! \n")
		// TODO: Node not found, find closest mapped Node and work from there
		return
	}

	directionString := "falling"
	signals := knoten.HauptsigF
	if !isFalling {
		directionString = "rising"
		signals = knoten.HauptsigS
	}

	for _, signal := range signals {
		kilometrage, _ := formatKilometrageStringInFloat(signal.KnotenTyp.Kilometrierung.Value)

		maxNode, err := findBestOSMNode(osmData, anchors, kilometrage)
		if err != nil {
			fmt.Printf("Error with finding node for signal %s: %s \n", signal.Name.Value, err.Error())
			continue
		}

		*nodeIdCounter++
		newSignalNode := OSMUtil.Node{
			Id:  strconv.Itoa(*nodeIdCounter),
			Lat: maxNode.Lat,
			Lon: maxNode.Lon,
			Tag: []*OSMUtil.Tag{
				{XMLName: XML_TAG_NAME_CONST, K: "type", V: "element"},
				{XMLName: XML_TAG_NAME_CONST, K: "subtype", V: "ms"},
				{XMLName: XML_TAG_NAME_CONST, K: "id", V: signal.Name.Value},
				{XMLName: XML_TAG_NAME_CONST, K: "direction", V: directionString},
			},
		}
		OSMUtil.InsertNewNodeWithReferenceNode(osmData, &newSignalNode, maxNode)
	}
}

// findBestOSMNode determines a pair of anchors based on which a new Node is searched.
// Based on those, a new Node is then determined.
func findBestOSMNode(
	osmData *OSMUtil.Osm,
	anchors *map[float64]([]*OSMUtil.Node),
	kilometrage float64,
) (*OSMUtil.Node, error) {
	sortedAnchors := sortAnchors(anchors, kilometrage)

	nearest, secondNearest := sortedAnchors[0], sortedAnchors[1]

	anchor1 := ((*anchors)[nearest])[0]
	anchor2 := ((*anchors)[secondNearest])[0]
	distance1 := math.Abs(nearest - kilometrage)
	distance2 := math.Abs(secondNearest - kilometrage)

	newNode, err := findNewNode(
		osmData,
		anchor1,
		anchor2,
		distance1,
		distance2,
	)

	if err == nil {
		return newNode, nil
	}

	newAnchorCounter := 2
	for err != nil && newAnchorCounter < len(sortedAnchors) {
		innerError := errors.Unwrap(err)
		errorParts := strings.Split(innerError.Error(), ": ")
		if errorParts[0] != "insufficient anchor" {
			return nil, errors.Wrap(err, "failed to find OSM-node")
		}

		faultyNodeID := strings.ReplaceAll(errorParts[1], " ", "")

		if faultyNodeID == anchor1.Id {
			nearest = sortedAnchors[newAnchorCounter]
			anchor1 = ((*anchors)[nearest])[0]
			distance1 = math.Abs(nearest - kilometrage)
			newAnchorCounter++
		} else {
			secondNearest = sortedAnchors[newAnchorCounter]
			anchor2 = ((*anchors)[secondNearest])[0]
			distance2 = math.Abs(secondNearest - kilometrage)
			newAnchorCounter++
		}
		newNode, err = findNewNode(
			osmData,
			anchor1,
			anchor2,
			distance1,
			distance2,
		)
	}

	if newAnchorCounter == len(sortedAnchors) {
		return nil, errors.New("failed to find suitable anchors")
	}

	return newNode, nil
}

// sortAnchors sorts the kilometrage-keys of the anchor-map.
// The sort is based on least distance to the provided 'kilometrage'.
func sortAnchors(
	anchors *map[float64]([]*OSMUtil.Node),
	kilometrage float64,
) []float64 {
	anchorKeys := []float64{}
	for anchorKey := range *anchors {
		anchorKeys = append(anchorKeys, anchorKey)
	}

	sort.SliceStable(anchorKeys, func(i, j int) bool {
		floatKilometrage1, floatKilometrage2 := anchorKeys[i], anchorKeys[j]
		return math.Abs(kilometrage-floatKilometrage1) < math.Abs(kilometrage-floatKilometrage2)
	})

	return anchorKeys
}

// formatKilometrageStringInFloat formats the kilometrage and parses them to a float64.
// Kilometrages in the DB-data always use a comma for decimal separation and sometimes, a plus sign is present.
// This plus sign is parsed in a way, that e.g. '5,000+0,150' becomes '5.15'.
func formatKilometrageStringInFloat(
	in string,
) (float64, error) {
	split := strings.Split(in, "+")
	if len(split) == 1 {
		return strconv.ParseFloat(
			strings.ReplaceAll(in, ",", "."),
			64)
	}

	out := 0.0
	for _, splitPart := range split {
		floatPart, err := strconv.ParseFloat(
			strings.ReplaceAll(splitPart, ",", "."),
			64)
		if err != nil {
			return 0, errors.Wrap(err, "failed to parse kilometrage: "+in)
		}
		out += floatPart
	}

	return out, nil
}
