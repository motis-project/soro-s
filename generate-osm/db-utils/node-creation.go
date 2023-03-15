package dbUtils

import (
	"strconv"

	OSMUtil "transform-osm/osm-utils"
)

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
