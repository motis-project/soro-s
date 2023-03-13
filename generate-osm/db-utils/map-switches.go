package dbUtils

import (
	"strconv"
	OSMUtil "transform-osm/osm-utils"

	"github.com/pkg/errors"
)

// findAndMapAnchorSwitches identifies all Nodes in the provided 'osm' with the tag 'railway:switch'.
// If their 'name' or 'ref'-Tag matches the name of the signal from the DB-data currently being processed.
// This is done firstly for all "WeichenAnfang" where we also add a node to the osm-data.
// In a second pass, also "WeichenStamm", "WeichenAbzweigLinks" and "WeichenAbzweigRechts" are being looked at.
// However we do not add a node for those to only map each switch once.
func findAndMapAnchorSwitches(
	abschnitt *Spurplanabschnitt,
	osm *OSMUtil.Osm,
	anchors map[float64][]*OSMUtil.Node,
	foundAnchorCount *int,
	optionalNewId *int,
) error {
	for _, knoten := range abschnitt.Knoten {
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
					newSwitchNode := createNewSwitch(optionalNewId, node, switchBegin)
					osm.Node = append(osm.Node, &newSwitchNode)
					*foundAnchorCount++
				}
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
	}
	return nil
}

// createNewSwitch creates a new node with the tags 'type:element', 'subtype:simple_switch' and 'id:...' where ... is the name of the provided switch.
// It also increments the "global" NodeIDCounter provided in 'id'.
func createNewSwitch(
	id *int,
	node *OSMUtil.Node,
	switchBegin *Weichenanfang,
) OSMUtil.Node {
	*id++

	return OSMUtil.Node{
		Id:  strconv.Itoa(*id),
		Lat: node.Lat,
		Lon: node.Lon,
		Tag: []*OSMUtil.Tag{
			{XMLName: XML_TAG_NAME_CONST, K: "type", V: "element"},
			{XMLName: XML_TAG_NAME_CONST, K: "subtype", V: "simple_switch"},
			{XMLName: XML_TAG_NAME_CONST, K: "id", V: switchBegin.Name.Value},
		},
	}
}
