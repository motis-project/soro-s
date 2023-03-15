package dbUtils

import (
	OSMUtil "transform-osm/osm-utils"

	"github.com/pkg/errors"
)

// mapSpeedLimits processes all speed limits.
func mapSpeedLimits(
	osmData *OSMUtil.Osm,
	anchors map[float64]([]*OSMUtil.Node),
	nodeIdCounter *int,
	knoten Spurplanknoten,
	elementsNotFound map[string]([]string),
) error {
	err := searchSpeedLimit(
		osmData,
		anchors,
		nodeIdCounter,
		knoten,
		elementsNotFound,
		true)
	if err != nil {
		return errors.Wrap(err, "failed finding falling speed limit")
	}

	err = searchSpeedLimit(
		osmData,
		anchors,
		nodeIdCounter,
		knoten,
		elementsNotFound,
		false)
	if err != nil {
		return errors.Wrap(err, "failed finding rising speed limit")
	}
	return nil
}

// searchSpeedLimit searches for a Node, that best fits the speed limit to be mapped.
// This search is based on at least two anchored elements and their respective distance to the speed limit at hand.
// If no ore only one anchor could be identified, or all anchors are otherwise insufficient, no mapping can be done.
func searchSpeedLimit(
	osmData *OSMUtil.Osm,
	anchors map[float64]([]*OSMUtil.Node),
	nodeIdCounter *int,
	knoten Spurplanknoten,
	elementsNotFound map[string]([]string),
	isFalling bool,
) error {
	speeds := knoten.MaxSpeedF
	if !isFalling {
		speeds = knoten.MaxSpeedS
	}

	for _, speed := range speeds {
		kilometrage, _ := formatKilometrageStringInFloat(speed.KnotenTyp.Kilometrierung.Value)

		maxNode, err := findBestOSMNode(osmData, anchors, kilometrage)
		if err != nil {
			if errors.Cause(err) == errNoSuitableAnchors {
				elementsNotFound["speed limits"] = append(elementsNotFound["speed limits"], speed.Kilometrierung.Value)
				continue
			}
			return errors.Wrap(err, "failed to map speed limit "+speed.Kilometrierung.Value)
		}

		newNode := createDirectionalNode(
			nodeIdCounter,
			maxNode,
			"spl",
			isFalling,
		)
		newNode.Tag = append(newNode.Tag, &OSMUtil.Tag{XMLName: XML_TAG_NAME_CONST, K: "speed", V: speed.Geschwindigkeit.Value})
		OSMUtil.InsertNewNodeWithReferenceNode(
			osmData,
			&newNode,
			maxNode,
		)
	}
	return nil
}

// mapSlopes processes all slopes.
func mapSlopes(
	osmData *OSMUtil.Osm,
	anchors map[float64]([]*OSMUtil.Node),
	nodeIdCounter *int,
	knoten Spurplanknoten,
	elementsNotFound map[string]([]string),
) error {
	for _, slope := range knoten.Neigung {
		kilometrage, _ := formatKilometrageStringInFloat(slope.KnotenTyp.Kilometrierung.Value)

		maxNode, err := findBestOSMNode(osmData, anchors, kilometrage)
		if err != nil {
			if errors.Cause(err) == errNoSuitableAnchors {
				elementsNotFound["slopes"] = append(elementsNotFound["slopes"], slope.Kilometrierung.Value)
				continue
			}
			return errors.Wrap(err, "failed to map slope "+slope.Kilometrierung.Value)
		}

		newNode := createSimpleNode(
			nodeIdCounter,
			maxNode,
			"slope",
		)
		newNode.Tag = append(newNode.Tag, &OSMUtil.Tag{XMLName: XML_TAG_NAME_CONST, K: "rising", V: slope.Rising.Value})
		newNode.Tag = append(newNode.Tag, &OSMUtil.Tag{XMLName: XML_TAG_NAME_CONST, K: "falling", V: slope.Falling.Value})
		OSMUtil.InsertNewNodeWithReferenceNode(
			osmData,
			&newNode,
			maxNode,
		)
	}
	return nil
}

// mapEoTDs processes all speed limits.
func mapEoTDs(
	osmData *OSMUtil.Osm,
	anchors map[float64]([]*OSMUtil.Node),
	nodeIdCounter *int,
	knoten Spurplanknoten,
	elementsNotFound map[string]([]string),
) error {
	err := searchEoTD(
		osmData,
		anchors,
		nodeIdCounter,
		knoten,
		elementsNotFound,
		true)
	if err != nil {
		return errors.Wrap(err, "failed finding falling end of train detector")
	}

	err = searchEoTD(
		osmData,
		anchors,
		nodeIdCounter,
		knoten,
		elementsNotFound,
		false)
	if err != nil {
		return errors.Wrap(err, "failed finding rising end of train detector")
	}
	return nil
}

// searchEoTD searches for a Node, that best fits the end of train detector to be mapped.
// This search is based on at least two anchored elements and their respective distance to the end of train detector at hand.
// If no ore only one anchor could be identified, or all anchors are otherwise insufficient, no mapping can be done.
func searchEoTD(
	osmData *OSMUtil.Osm,
	anchors map[float64]([]*OSMUtil.Node),
	nodeIdCounter *int,
	knoten Spurplanknoten,
	elementsNotFound map[string]([]string),
	isFalling bool,
) error {
	track_ends := append(knoten.FstrZugschlussstelleF, knoten.SignalZugschlussstelleF...)
	if !isFalling {
		track_ends = append(knoten.FstrZugschlussstelleS, knoten.SignalZugschlussstelleS...)
	}

	for _, eotd := range track_ends {
		kilometrage, _ := formatKilometrageStringInFloat(eotd.KnotenTyp.Kilometrierung.Value)

		maxNode, err := findBestOSMNode(osmData, anchors, kilometrage)
		if err != nil {
			if errors.Cause(err) == errNoSuitableAnchors {
				elementsNotFound["eotds"] = append(elementsNotFound["eotds"], eotd.Kilometrierung.Value)
				continue
			}
			return errors.Wrap(err, "failed to map eotd "+eotd.Kilometrierung.Value)
		}

		newNode := createDirectionalNode(
			nodeIdCounter,
			maxNode,
			"eotd",
			isFalling,
		)
		OSMUtil.InsertNewNodeWithReferenceNode(
			osmData,
			&newNode,
			maxNode,
		)
	}
	return nil
}

// mapSimpleElement maps all non-directional simple DB elements.
// These are: line switches, kilometrage jumps, borders and bumpers.
// Inputting anything else as "elementType" will result in an error.
// Mapping is then done in the usual fashion, idetifying at least 2 anchors, etc.
func mapSimpleElement(
	osmData *OSMUtil.Osm,
	anchors map[float64]([]*OSMUtil.Node),
	nodeIdCounter *int,
	knoten Spurplanknoten,
	elementType string,
	elements []*SimpleElement,
	elementsNotFound map[string]([]string),
) error {
	var elementTypeLong string
	switch elementType {
	case "line_switch":
		elementTypeLong = "line switch"
	case "km_jump":
		elementTypeLong = "kilometrage jump"
	case "border":
		elementTypeLong = "border"
	case "bumper":
		elementTypeLong = "bumper"
	default:
		return errors.New("failed to identify" + elementType + " as simple element.")
	}

	for _, element := range elements {
		kilometrage, _ := formatKilometrageStringInFloat(element.KnotenTyp.Kilometrierung.Value)

		maxNode, err := findBestOSMNode(osmData, anchors, kilometrage)
		if err != nil {
			if errors.Cause(err) == errNoSuitableAnchors {
				elementsNotFound[elementTypeLong+"s"] = append(elementsNotFound[elementTypeLong+"s"], element.Kilometrierung.Value)
				continue
			}
			return errors.Wrap(err, "failed to map line switche "+element.Kilometrierung.Value)
		}

		newNode := createSimpleNode(
			nodeIdCounter,
			maxNode,
			elementType,
		)
		OSMUtil.InsertNewNodeWithReferenceNode(
			osmData,
			&newNode,
			maxNode,
		)
	}
	return nil
}

// mapNamedSimpleElement maps non-directional named simple DB elements.
// These are tunnels and track ends.
// Inputting anything else as "elementType" will result in an error.
// Mapping is then done in the usual fashion, identifying at least two anchors, etc.
func mapNamedSimpleElement(
	osmData *OSMUtil.Osm,
	anchors map[float64]([]*OSMUtil.Node),
	nodeIdCounter *int,
	knoten Spurplanknoten,
	elementType string,
	elements []*NamedSimpleElement,
	elementsNotFound map[string]([]string),
) error {
	var elementTypeLong string
	if elementType == "tunnel" || elementType == "track_end" {
		elementTypeLong = elementType
	} else {
		return errors.New("failed to identify " + elementType + " as named simple element.")
	}

	for _, element := range elements {
		kilometrage, _ := formatKilometrageStringInFloat(element.KnotenTyp.Kilometrierung.Value)

		maxNode, err := findBestOSMNode(osmData, anchors, kilometrage)
		if err != nil {
			if errors.Cause(err) == errNoSuitableAnchors {
				elementsNotFound[elementTypeLong+"s"] = append(elementsNotFound[elementTypeLong+"s"], element.Name.Value)
				continue
			}
			return errors.Wrap(err, "failed to map line switche "+element.Name.Value)
		}

		newNode := createNamedSimpleNode(
			nodeIdCounter,
			maxNode,
			elementType,
			element.Name.Value,
		)
		OSMUtil.InsertNewNodeWithReferenceNode(
			osmData,
			&newNode,
			maxNode,
		)
	}
	return nil
}
