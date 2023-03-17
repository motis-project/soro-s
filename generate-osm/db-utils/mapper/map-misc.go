package mapper

import (
	findNodes "transform-osm/db-utils/find-nodes"
	OSMUtil "transform-osm/osm-utils"

	"github.com/pkg/errors"
)

// MapSpeedLimits processes all speed limits.
func MapSpeedLimits(
	osmData *OSMUtil.Osm,
	anchors map[float64]([]*OSMUtil.Node),
	nodeIdCounter *int,
	knoten Spurplanknoten,
	tracker NotFoundElementTracker,
) error {
	err := searchSpeedLimit(
		osmData,
		anchors,
		nodeIdCounter,
		knoten,
		tracker,
		true)
	if err != nil {
		return errors.Wrap(err, "failed finding falling speed limit")
	}

	err = searchSpeedLimit(
		osmData,
		anchors,
		nodeIdCounter,
		knoten,
		tracker,
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
	tracker NotFoundElementTracker,
	isFalling bool,
) error {
	speeds := knoten.MaxSpeedF
	if !isFalling {
		speeds = knoten.MaxSpeedS
	}

	for _, speed := range speeds {
		kilometrage, _ := findNodes.FormatKilometrageStringInFloat(speed.KnotenTyp.Kilometrierung.Value)

		maxNode, err := findNodes.FindBestOSMNode(osmData, anchors, kilometrage)
		if err != nil {
			if errors.Cause(err) == findNodes.ErrNoSuitableAnchors {
				tracker.AddNotFoundElement(SpeedLimits, speed.Kilometrierung.Value)
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

// MapSlopes processes all slopes.
func MapSlopes(
	osmData *OSMUtil.Osm,
	anchors map[float64]([]*OSMUtil.Node),
	nodeIdCounter *int,
	knoten Spurplanknoten,
	tracker NotFoundElementTracker,
) error {
	for _, slope := range knoten.Neigung {
		kilometrage, _ := findNodes.FormatKilometrageStringInFloat(slope.KnotenTyp.Kilometrierung.Value)

		maxNode, err := findNodes.FindBestOSMNode(osmData, anchors, kilometrage)
		if err != nil {
			if errors.Cause(err) == findNodes.ErrNoSuitableAnchors {
				tracker.AddNotFoundElement(Slopes, slope.Kilometrierung.Value)
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

// MapEoTDs processes all speed limits.
func MapEoTDs(
	osmData *OSMUtil.Osm,
	anchors map[float64]([]*OSMUtil.Node),
	nodeIdCounter *int,
	knoten Spurplanknoten,
	tracker NotFoundElementTracker,
) error {
	err := searchEoTD(
		osmData,
		anchors,
		nodeIdCounter,
		knoten,
		tracker,
		true)
	if err != nil {
		return errors.Wrap(err, "failed finding falling end of train detector")
	}

	err = searchEoTD(
		osmData,
		anchors,
		nodeIdCounter,
		knoten,
		tracker,
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
	tracker NotFoundElementTracker,
	isFalling bool,
) error {
	track_ends := append(knoten.FstrZugschlussstelleF, knoten.SignalZugschlussstelleF...)
	if !isFalling {
		track_ends = append(knoten.FstrZugschlussstelleS, knoten.SignalZugschlussstelleS...)
	}

	for _, eotd := range track_ends {
		kilometrage, _ := findNodes.FormatKilometrageStringInFloat(eotd.KnotenTyp.Kilometrierung.Value)

		maxNode, err := findNodes.FindBestOSMNode(osmData, anchors, kilometrage)
		if err != nil {
			if errors.Cause(err) == findNodes.ErrNoSuitableAnchors {
				tracker.AddNotFoundElement(Eotds, eotd.Kilometrierung.Value)
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

// MapSimpleElement maps all non-directional simple DB elements.
// These are: line switches, kilometrage jumps, borders and bumpers.
// Inputting anything else as "elementType" will result in an error.
// Mapping is then done in the usual fashion, idetifying at least 2 anchors, etc.
func MapSimpleElement(
	osmData *OSMUtil.Osm,
	anchors map[float64]([]*OSMUtil.Node),
	nodeIdCounter *int,
	elements []*SimpleElement,
	elementType ElementType,
	tracker NotFoundElementTracker,
) error {
	for _, element := range elements {
		kilometrage, _ := findNodes.FormatKilometrageStringInFloat(element.KnotenTyp.Kilometrierung.Value)

		maxNode, err := findNodes.FindBestOSMNode(osmData, anchors, kilometrage)
		if err != nil {
			if errors.Cause(err) == findNodes.ErrNoSuitableAnchors {
				tracker.AddNotFoundElement(elementType, element.Kilometrierung.Value)
				continue
			}
			return errors.Wrap(err, "failed to map line switche "+element.Kilometrierung.Value)
		}

		elementSubType := "line_switch"
		if elementType == KilometrageJump {
			elementSubType = "km_jump"
		} else if elementType == Border {
			elementSubType = "border"
		} else if elementType == Bumper {
			elementSubType = "bumper"
		}

		newNode := createSimpleNode(
			nodeIdCounter,
			maxNode,
			elementSubType,
		)
		OSMUtil.InsertNewNodeWithReferenceNode(
			osmData,
			&newNode,
			maxNode,
		)
	}
	return nil
}

// MapNamedSimpleElement maps non-directional named simple DB elements.
// These are tunnels and track ends.
// Inputting anything else as "elementType" will result in an error.
// Mapping is then done in the usual fashion, identifying at least two anchors, etc.
func MapNamedSimpleElement(
	osmData *OSMUtil.Osm,
	anchors map[float64]([]*OSMUtil.Node),
	nodeIdCounter *int,
	elementType ElementType,
	elements []*NamedSimpleElement,
	tracker NotFoundElementTracker,
) error {
	for _, element := range elements {
		kilometrage, _ := findNodes.FormatKilometrageStringInFloat(element.KnotenTyp.Kilometrierung.Value)

		maxNode, err := findNodes.FindBestOSMNode(osmData, anchors, kilometrage)
		if err != nil {
			if errors.Cause(err) == findNodes.ErrNoSuitableAnchors {
				tracker.AddNotFoundElement(elementType, element.Name.Value)
				continue
			}
			return errors.Wrap(err, "failed to map line switche "+element.Name.Value)
		}

		elementSubType := "tunnel"
		if elementType == TrackEnd {
			elementSubType = "track_end"
		}

		newNode := createNamedSimpleNode(
			nodeIdCounter,
			maxNode,
			elementSubType,
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
