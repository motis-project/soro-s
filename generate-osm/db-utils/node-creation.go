package dbUtils

import (
	"strconv"

	OSMUtil "transform-osm/osm-utils"
)

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
