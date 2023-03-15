package dbUtils

import (
	OSMUtil "transform-osm/osm-utils"

	"github.com/pkg/errors"
)

// findAndMapAnchorSwitches identifies all Nodes in the provided 'osm' with the tag 'railway:switch'.
// If their 'name' or 'ref'-Tag matches the name of the signal from the DB-data currently being processed.
// This is done firstly for all "WeichenAnfang" where we also add a node to the osm-data.
// In a second pass, also "WeichenStamm", "WeichenAbzweigLinks" and "WeichenAbzweigRechts" are being looked at.
// However we do not add a node for those to only map each switch once.
func findAndMapAnchorSwitches(
	knoten Spurplanknoten,
	osm *OSMUtil.Osm,
	anchors map[float64][]*OSMUtil.Node,
	notFoundSwitches *[]*Weichenanfang,
	foundAnchorCount *int,
	nodeIdCounter *int,
) error {
	foundSwitch := false
	for _, switchBegin := range knoten.WeichenAnf {
		for _, node := range osm.Node {
			if len(node.Tag) == 0 {
				continue
			}

			railwayTag, _ := OSMUtil.FindTagOnNode(node, "railway")
			refTag, _ := OSMUtil.FindTagOnNode(node, "ref")
			name, _ := OSMUtil.FindTagOnNode(node, "name")

			if railwayTag == "switch" &&
				(refTag == switchBegin.Name.Value || name == switchBegin.Name.Value) {

				kilometrageFloat, err := formatKilometrageStringInFloat(switchBegin.Kilometrierung.Value)
				if err != nil {
					return errors.Wrap(err, "failed to format kilometrage")
				}

				anchors[kilometrageFloat] = append(anchors[kilometrageFloat], node)
				newSwitchNode := createNamedSimpleNode(
					nodeIdCounter,
					node,
					"simple_switch",
					switchBegin.Name.Value,
				)
				OSMUtil.InsertNewNodeWithReferenceNode(
					osm,
					&newSwitchNode,
					node,
				)
				*foundAnchorCount++
				foundSwitch = true
				break
			}
		}
		if !foundSwitch {
			*notFoundSwitches = append(*notFoundSwitches, switchBegin)
		}
	}

	restSwitches := make([]*Weichenknoten, len(knoten.WeichenStamm)+len(knoten.WeichenAbzwLinks)+len(knoten.WeichenAbzwRechts))
	copy(restSwitches, knoten.WeichenStamm)
	copy(restSwitches[len(knoten.WeichenStamm):], knoten.WeichenAbzwLinks)
	copy(restSwitches[len(knoten.WeichenStamm)+len(knoten.WeichenAbzwLinks):], knoten.WeichenAbzwRechts)
	for _, switchBegin := range restSwitches {
		for _, node := range osm.Node {
			if len(node.Tag) == 0 {
				continue
			}

			railwayTag, _ := OSMUtil.FindTagOnNode(node, "railway")
			refTag, _ := OSMUtil.FindTagOnNode(node, "ref")
			name, _ := OSMUtil.FindTagOnNode(node, "name")

			if railwayTag == "switch" {
				partnerName := switchBegin.Partner.Name

				kilometrageFloat, err := formatKilometrageStringInFloat(switchBegin.Kilometrierung.Value)
				if err != nil {
					return errors.Wrap(err, "failed to format kilometrage")
				}

				if (partnerName == refTag || partnerName == name) &&
					anchors[kilometrageFloat] == nil {
					anchors[kilometrageFloat] = append(anchors[kilometrageFloat], node)
					*foundAnchorCount++
				}
			}
		}
	}
	return nil
}

// mapUnanchoredSwitches processes all switches for which no unique Node could be determined.
func mapUnanchoredSwitches(
	osmData *OSMUtil.Osm,
	anchors *map[float64]([]*OSMUtil.Node),
	nodeIdCounter *int,
	knoten Spurplanknoten,
	elementsNotFound map[string]([]string),
) error {
	for _, simple_switch := range knoten.WeichenAnf {
		kilometrage, _ := formatKilometrageStringInFloat(simple_switch.KnotenTyp.Kilometrierung.Value)

		maxNode, err := findBestOSMNode(osmData, anchors, kilometrage)
		if err != nil {
			if errors.Cause(err) == errNoSuitableAnchors {
				elementsNotFound["switches"] = append(elementsNotFound["switches"], simple_switch.Name.Value)
				continue

			}
			return errors.Wrap(err, "failed to map switch "+simple_switch.Name.Value)
		}

		newSignalNode := createNamedSimpleNode(
			nodeIdCounter,
			maxNode,
			"simple_switch",
			simple_switch.Name.Value,
		)
		OSMUtil.InsertNewNodeWithReferenceNode(
			osmData,
			&newSignalNode,
			maxNode,
		)
	}
	return nil
}

func mapCrosses(
	osmData *OSMUtil.Osm,
	anchors map[float64]([]*OSMUtil.Node),
	nodeIdCounter *int,
	knoten Spurplanknoten,
	elementsNotFound map[string]([]string),
) error {
	for _, cross := range knoten.KreuzungsweicheAnfangLinks {
		kilometrage, _ := formatKilometrageStringInFloat(cross.KnotenTyp.Kilometrierung.Value)

		maxNode, err := findBestOSMNode(osmData, &anchors, kilometrage)
		if err != nil {
			if errors.Cause(err) == errNoSuitableAnchors {
				elementsNotFound["crosses"] = append(elementsNotFound["crosses"], cross.Name.Value)
				continue

			}
			return errors.Wrap(err, "failed to map cross "+cross.Name.Value)
		}

		newSignalNode := createNamedSimpleNode(
			nodeIdCounter,
			maxNode,
			"cross",
			cross.Name.Value,
		)
		OSMUtil.InsertNewNodeWithReferenceNode(
			osmData,
			&newSignalNode,
			maxNode,
		)
	}
	return nil
}
