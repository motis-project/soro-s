package mapper

import (
	findNodes "transform-osm/db-utils/find-nodes"
	osmUtils "transform-osm/osm-utils"

	"github.com/pkg/errors"
)

// FindAndMapAnchorSwitches identifies all Nodes in the provided 'osm' with the tag 'railway:switch'.
// If their 'name' or 'ref'-Tag matches the name of the signal from the DB-data currently being processed.
// This is done firstly for all "WeichenAnfang" where we also add a node to the osm-data.
// In a second pass, also "WeichenStamm", "WeichenAbzweigLinks" and "WeichenAbzweigRechts" are being looked at.
// However we do not add a node for those to only map each switch once.
func FindAndMapAnchorSwitches(
	knoten Spurplanknoten,
	osm *osmUtils.Osm,
	anchors map[float64][]*osmUtils.Node,
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

			railwayTag, _ := osmUtils.FindTagOnNode(node, "railway")
			refTag, _ := osmUtils.FindTagOnNode(node, "ref")
			name, _ := osmUtils.FindTagOnNode(node, "name")

			if railwayTag == "switch" &&
				(refTag == switchBegin.Name.Value || name == switchBegin.Name.Value) {

				kilometrageFloat, err := findNodes.FormatKilometrageStringInFloat(switchBegin.Kilometrierung.Value)
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
				osmUtils.InsertNewNodeWithReferenceNode(
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

			railwayTag, _ := osmUtils.FindTagOnNode(node, "railway")
			refTag, _ := osmUtils.FindTagOnNode(node, "ref")
			name, _ := osmUtils.FindTagOnNode(node, "name")

			if railwayTag == "switch" {
				partnerName := switchBegin.Partner.Name

				kilometrageFloat, err := findNodes.FormatKilometrageStringInFloat(switchBegin.Kilometrierung.Value)
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

// MapUnanchoredSwitches processes all switches for which no unique Node could be determined.
func MapUnanchoredSwitches(
	osmData *osmUtils.Osm,
	anchors map[float64]([]*osmUtils.Node),
	nodeIdCounter *int,
	knoten Spurplanknoten,
	tracker NotFoundElementTracker,
) error {
	for _, simple_switch := range knoten.WeichenAnf {
		kilometrage, _ := findNodes.FormatKilometrageStringInFloat(simple_switch.KnotenTyp.Kilometrierung.Value)

		maxNode, err := findNodes.FindBestOSMNode(osmData, anchors, kilometrage)
		if err != nil {
			if errors.Cause(err) == findNodes.ErrNoSuitableAnchors {
				tracker.AddNotFoundElement(Switches, simple_switch.Name.Value)
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
		osmUtils.InsertNewNodeWithReferenceNode(
			osmData,
			&newSignalNode,
			maxNode,
		)
	}
	return nil
}

func MapCrosses(
	osmData *osmUtils.Osm,
	anchors map[float64]([]*osmUtils.Node),
	nodeIdCounter *int,
	knoten Spurplanknoten,
	tracker NotFoundElementTracker,
) error {
	for _, cross := range knoten.KreuzungsweicheAnfangLinks {
		kilometrage, _ := findNodes.FormatKilometrageStringInFloat(cross.KnotenTyp.Kilometrierung.Value)

		maxNode, err := findNodes.FindBestOSMNode(osmData, anchors, kilometrage)
		if err != nil {
			if errors.Cause(err) == findNodes.ErrNoSuitableAnchors {
				tracker.AddNotFoundElement(Crosses, cross.Name.Value)
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
		osmUtils.InsertNewNodeWithReferenceNode(
			osmData,
			&newSignalNode,
			maxNode,
		)
	}
	return nil
}
