package mapper

import (
	"encoding/xml"
	"strconv"

	OSMUtil "transform-osm/osm-utils"
)

var XML_TAG_NAME_CONST = xml.Name{Space: " ", Local: "tag"}

// createNamedDirectionalNode creates a Node with the following Tags:
// 'type:element', 'subtype:"subtype"', 'direction:...' and 'id:"name"', where ... depends on "isFalling".
// It also increments the global id counter.
func createNamedDirectionalNode(
	id *int,
	node *OSMUtil.Node,
	subtype string,
	name string,
	isFalling bool,
) OSMUtil.Node {
	newNode := createDirectionalNode(id, node, subtype, isFalling)
	newNode.Tag = append(newNode.Tag, &OSMUtil.Tag{XMLName: XML_TAG_NAME_CONST, K: "id", V: name})

	return newNode
}

// createDirectionalNode creates a Node with the following Tags:
// 'type:element', 'subtype:"subtype"' and 'direction:...' where ... depends on "isFalling".
// It also increments the global id counter.
func createDirectionalNode(
	id *int,
	node *OSMUtil.Node,
	subtype string,
	isFalling bool,
) OSMUtil.Node {
	directionString := "falling"
	if !isFalling {
		directionString = "rising"
	}

	newNode := createSimpleNode(id, node, subtype)
	newNode.Tag = append(newNode.Tag, &OSMUtil.Tag{XMLName: XML_TAG_NAME_CONST, K: "direction", V: directionString})

	return newNode
}

// createNamedSimpleNode creates a Node with the following Tags:
// 'type:element', 'subtype:"subtype"' and 'id:"name".
// It also increments the global id counter.
func createNamedSimpleNode(
	id *int,
	node *OSMUtil.Node,
	subtype string,
	name string,
) OSMUtil.Node {
	newNode := createSimpleNode(id, node, subtype)
	newNode.Tag = append(newNode.Tag, &OSMUtil.Tag{XMLName: XML_TAG_NAME_CONST, K: "id", V: name})

	return newNode
}

// createSimpleNode returns a Node with the following Tags:
// 'type:element' and 'subtype:"subtype"'.
// It also increments the global id counter.
func createSimpleNode(
	id *int,
	node *OSMUtil.Node,
	subtype string,
) OSMUtil.Node {
	*id++

	return OSMUtil.Node{
		Id:  strconv.Itoa(*id),
		Lat: node.Lat,
		Lon: node.Lon,
		Tag: []*OSMUtil.Tag{
			{XMLName: XML_TAG_NAME_CONST, K: "type", V: "element"},
			{XMLName: XML_TAG_NAME_CONST, K: "subtype", V: subtype},
		},
	}
}
