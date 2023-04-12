package mapper

import (
	findNodes "transform-osm/db-utils/find-nodes"
	osmUtils "transform-osm/osm-utils"

	"github.com/pkg/errors"
)

// MapHalts processes all halts.
// This includes freight as well as passenger train halts.
func MapHalts(
	osmData *osmUtils.Osm,
	anchors map[float64]([]*osmUtils.Node),
	haltList map[string]osmUtils.Halt,
	nodeIdCounter *int,
	knoten Spurplanknoten,
	tracker NotFoundElementTracker,
) error {
	err := searchHalt(
		osmData,
		anchors,
		haltList,
		nodeIdCounter,
		knoten,
		tracker,
		true)
	if err != nil {
		return errors.Wrap(err, "failed finding falling stop position")
	}
	err = searchHalt(
		osmData,
		anchors,
		haltList,
		nodeIdCounter,
		knoten,
		tracker,
		false)
	if err != nil {
		return errors.Wrap(err, "failed finding rising stop position")
	}
	return nil
}

// searchHalt searches for a Node, that best fits the halt to be mapped.
// This search is based on at least two anchored elements and their respective distance to the halt at hand.
// If no ore only one anchor could be identified, or all anchors are otherwise insufficient, no mapping can be done.
func searchHalt(
	osmData *osmUtils.Osm,
	anchors map[float64]([]*osmUtils.Node),
	haltList map[string]osmUtils.Halt,
	nodeIdCounter *int,
	knoten Spurplanknoten,
	tracker NotFoundElementTracker,
	isFalling bool,
) error {
	halts := append(knoten.HalteplGzF, knoten.HalteplRzF...)
	if !isFalling {
		halts = append(knoten.HalteplGzS, knoten.HalteplRzS...)
	}

	for _, halt := range halts {
		kilometrage, _ := findNodes.FormatKilometrageStringInFloat(halt.KnotenTyp.Kilometrierung.Value)

		maxNode, err := findNodes.FindBestOSMNode(osmData, anchors, kilometrage)
		if err != nil {
			if errors.Cause(err) == findNodes.ErrNoSuitableAnchors {
				tracker.AddNotFoundElement(Halt, halt.Name.Value)
				continue
			}
			return errors.Wrap(err, "failed to map stop position "+halt.Name.Value)
		}

		newSignalNode := createNamedDirectionalNode(
			nodeIdCounter,
			maxNode,
			"hlt",
			halt.Name.Value,
			isFalling,
		)
		osmUtils.InsertNewNodeWithReferenceNode(
			osmData,
			&newSignalNode,
			maxNode,
		)
		haltList[newSignalNode.Id] = osmUtils.Halt{
			Name: halt.Name.Value,
			Lat:  newSignalNode.Lat,
			Lon:  newSignalNode.Lon,
		}
	}
	return nil
}
