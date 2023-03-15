package dbUtils

import (
	OSMUtil "transform-osm/osm-utils"

	"github.com/pkg/errors"
)

// mapUnanchoredMainSignals processes all main signals for which no unique Node could be determined.
func mapHalts(
	osmData *OSMUtil.Osm,
	anchors *map[float64]([]*OSMUtil.Node),
	nodeIdCounter *int,
	knoten Spurplanknoten,
	elementsNotFound map[string]([]string),
) error {
	err := searchHalt(
		osmData,
		anchors,
		nodeIdCounter,
		knoten,
		elementsNotFound,
		true)
	if err != nil {
		return errors.Wrap(err, "failed finding falling stop position")
	}
	err = searchHalt(
		osmData,
		anchors,
		nodeIdCounter,
		knoten,
		elementsNotFound,
		false)
	if err != nil {
		return errors.Wrap(err, "failed finding falling stop position")
	}
	return nil
}

// serachUnanchoredMainSignal searches for a Node, that best fits the Signal to be mapped.
// This search is based on at least two anchored elements and their respective distance to the signal at hand.
// If no ore only one anchor could be identified, or all anchors are otherwise insufficient, no mapping can be done.
func searchHalt(
	osmData *OSMUtil.Osm,
	anchors *map[float64]([]*OSMUtil.Node),
	nodeIdCounter *int,
	knoten Spurplanknoten,
	elementsNotFound map[string]([]string),
	isFalling bool,
) error {
	halts := append(knoten.HalteplGzF, knoten.HalteplRzF...)
	if !isFalling {
		halts = append(knoten.HalteplGzS, knoten.HalteplRzS...)
	}

	for _, halt := range halts {
		kilometrage, _ := formatKilometrageStringInFloat(halt.KnotenTyp.Kilometrierung.Value)

		maxNode, err := findBestOSMNode(osmData, anchors, kilometrage)
		if err != nil {
			if errors.Cause(err) == errNoSuitableAnchors {
				elementsNotFound["stop positions"] = append(elementsNotFound["stop positions"], halt.Name.Value)
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
		OSMUtil.InsertNewNodeWithReferenceNode(
			osmData,
			&newSignalNode,
			maxNode,
		)
	}
	return nil
}
