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

func findAndMapAnchorMainSignals(
	abschnitt *Spurplanabschnitt,
	osm *OSMUtil.Osm,
	anchors map[float64][]*OSMUtil.Node,
	notFoundSignalsFalling *[]*Signal,
	notFoundSignalsRising *[]*Signal,
	foundAnchorCount *int,
	optionalNewId *int,
) {
	conflictingSignalNames := map[string]bool{}
	for _, knoten := range abschnitt.Knoten {
		processHauptsignal(
			*knoten,
			notFoundSignalsFalling,
			anchors,
			&conflictingSignalNames,
			osm,
			true,
			foundAnchorCount,
			optionalNewId,
		)
		processHauptsignal(
			*knoten,
			notFoundSignalsRising,
			anchors,
			&conflictingSignalNames,
			osm,
			false,
			foundAnchorCount,
			optionalNewId,
		)
	}
}

func processHauptsignal(
	knoten Spurplanknoten,
	notFoundSignals *[]*Signal,
	anchors map[float64][]*OSMUtil.Node,
	conflictingSignalNames *map[string]bool,
	osm *OSMUtil.Osm,
	isFalling bool,
	foundAnchorCount *int,
	optionalNewId *int,
) {
	signals := knoten.HauptsigF
	if !isFalling {
		signals = knoten.HauptsigS
	}

	for _, signal := range signals {
		conflictFreeSignal := false
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
			conflictFreeSignal = insertNewHauptsignal(
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
			if conflictFreeSignal {
				*foundAnchorCount++
				return
			}
			(*conflictingSignalNames)[signal.Name.Value] = true
		}
		*notFoundSignals = append(*notFoundSignals, signal)
	}
}

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
) bool {
	if conflictingSignalNames[signal.Name.Value] {
		return false
	}
	signalKilometrage, err := formatKilometrageStringInFloat(signal.KnotenTyp.Kilometrierung.Value)
	if err != nil {
		panic(err)
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
				return false
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
	return true
}

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

func mapUnanchoredMainSignals(
	osmData *OSMUtil.Osm,
	anchors *map[float64]([]*OSMUtil.Node),
	nodeIdCounter *int,
	dbData XmlIssDaten,
) {
	for _, stelle := range dbData.Betriebsstellen {
		for _, abschnitt := range stelle.Abschnitte {
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
	}
}

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
			fmt.Printf("Error: %s \n", err.Error())
			continue
		}

		if maxNode == nil {
			print("Hello")
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
			return nil, errors.Wrap(err, "could not find OSM-node")
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
		return nil, errors.New("could not find OSM-node")
	}

	return newNode, nil
}

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
			return 0, errors.Wrap(err, "could not parse kilometrage: "+in)
		}
		out += floatPart
	}

	return out, nil
}
