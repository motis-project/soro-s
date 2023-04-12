package mapper_test

import (
	"encoding/xml"
	"testing"
	"transform-osm/db-utils/mapper"
	OSMUtil "transform-osm/osm-utils"

	"github.com/stretchr/testify/assert"
)

var XML_TAG_NAME_CONST = xml.Name{Space: " ", Local: "tag"}

func TestCreateNamedDirectionalNode(t *testing.T) {
	type args struct {
		id        *int
		node      *OSMUtil.Node
		subtype   string
		name      string
		isFalling bool
	}
	type want struct {
		node OSMUtil.Node
	}
	id := 0
	tests := []struct {
		name string
		args args
		want want
	}{
		{
			name: "creates a node with the correct id and tags for a falling node",
			args: args{
				id: &id,
				node: &OSMUtil.Node{
					Id:  "10",
					Lat: "1",
					Lon: "2",
				},
				subtype:   "test",
				name:      "coolName",
				isFalling: true,
			},
			want: want{
				node: OSMUtil.Node{
					Id:  "1",
					Lat: "1",
					Lon: "2",
					Tag: []*OSMUtil.Tag{
						{XMLName: XML_TAG_NAME_CONST, K: "type", V: "element"},
						{XMLName: XML_TAG_NAME_CONST, K: "subtype", V: "test"},
						{XMLName: XML_TAG_NAME_CONST, K: "id", V: "coolName"},
						{XMLName: XML_TAG_NAME_CONST, K: "direction", V: "falling"},
					},
				},
			},
		},
		{
			name: "creates a node with the correct id and tags for a rising node",
			args: args{
				id: &id,
				node: &OSMUtil.Node{
					Id:  "10",
					Lat: "1",
					Lon: "2",
				},
				subtype:   "test",
				name:      "coolName",
				isFalling: false,
			},
			want: want{
				node: OSMUtil.Node{
					Id:  "2",
					Lat: "1",
					Lon: "2",
					Tag: []*OSMUtil.Tag{
						{XMLName: XML_TAG_NAME_CONST, K: "type", V: "element"},
						{XMLName: XML_TAG_NAME_CONST, K: "subtype", V: "test"},
						{XMLName: XML_TAG_NAME_CONST, K: "id", V: "coolName"},
						{XMLName: XML_TAG_NAME_CONST, K: "direction", V: "rising"},
					},
				},
			},
		},
	}

	for _, tt := range tests {
		t.Run(tt.name, func(t *testing.T) {
			node := mapper.CreateSimpleNode(tt.args.id, tt.args.node, tt.args.subtype)
			assert.Equal(t, tt.want.node.Id, node.Id, "Id should be equal")
			assert.Equal(t, tt.want.node.Lat, node.Lat, "Lat should be equal")
			assert.Equal(t, tt.want.node.Lon, node.Lon, "Lon should be equal")
			assert.Equal(t, tt.want.node.Tag[0].K, tt.want.node.Tag[0].K, "Tag[0].K should be equal")
			assert.Equal(t, tt.want.node.Tag[0].V, tt.want.node.Tag[0].V, "Tag[0].V should be equal")
			assert.Equal(t, tt.want.node.Tag[1].K, tt.want.node.Tag[1].K, "Tag[1].K should be equal")
			assert.Equal(t, tt.want.node.Tag[1].V, tt.want.node.Tag[1].V, "Tag[1].V should be equal")
			assert.Equal(t, tt.want.node.Tag[2].K, tt.want.node.Tag[2].K, "Tag[2].K should be equal")
			assert.Equal(t, tt.want.node.Tag[2].V, tt.want.node.Tag[2].V, "Tag[2].V should be equal")
			assert.Equal(t, tt.want.node.Tag[3].K, tt.want.node.Tag[3].K, "Tag[3].K should be equal")
			assert.Equal(t, tt.want.node.Tag[3].V, tt.want.node.Tag[3].V, "Tag[3].V should be equal")
		})
	}
}

func TestCreateDirectionalNode(t *testing.T) {
	type args struct {
		id        *int
		node      *OSMUtil.Node
		subtype   string
		isFalling bool
	}
	type want struct {
		node OSMUtil.Node
	}
	id := 0
	tests := []struct {
		name string
		args args
		want want
	}{
		{
			name: "creates a node with the correct id and tags for a falling node",
			args: args{
				id: &id,
				node: &OSMUtil.Node{
					Id:  "10",
					Lat: "1",
					Lon: "2",
				},
				subtype:   "test",
				isFalling: true,
			},
			want: want{
				node: OSMUtil.Node{
					Id:  "1",
					Lat: "1",
					Lon: "2",
					Tag: []*OSMUtil.Tag{
						{XMLName: XML_TAG_NAME_CONST, K: "type", V: "element"},
						{XMLName: XML_TAG_NAME_CONST, K: "subtype", V: "test"},
						{XMLName: XML_TAG_NAME_CONST, K: "direction", V: "falling"},
					},
				},
			},
		},
		{
			name: "creates a node with the correct id and tags for a rising node",
			args: args{
				id: &id,
				node: &OSMUtil.Node{
					Id:  "10",
					Lat: "1",
					Lon: "2",
				},
				subtype:   "test",
				isFalling: false,
			},
			want: want{
				node: OSMUtil.Node{
					Id:  "2",
					Lat: "1",
					Lon: "2",
					Tag: []*OSMUtil.Tag{
						{XMLName: XML_TAG_NAME_CONST, K: "type", V: "element"},
						{XMLName: XML_TAG_NAME_CONST, K: "subtype", V: "test"},
						{XMLName: XML_TAG_NAME_CONST, K: "direction", V: "rising"},
					},
				},
			},
		},
	}

	for _, tt := range tests {
		t.Run(tt.name, func(t *testing.T) {
			node := mapper.CreateSimpleNode(tt.args.id, tt.args.node, tt.args.subtype)
			assert.Equal(t, tt.want.node.Id, node.Id, "Id should be equal")
			assert.Equal(t, tt.want.node.Lat, node.Lat, "Lat should be equal")
			assert.Equal(t, tt.want.node.Lon, node.Lon, "Lon should be equal")
			assert.Equal(t, tt.want.node.Tag[0].K, tt.want.node.Tag[0].K, "Tag[0].K should be equal")
			assert.Equal(t, tt.want.node.Tag[0].V, tt.want.node.Tag[0].V, "Tag[0].V should be equal")
			assert.Equal(t, tt.want.node.Tag[1].K, tt.want.node.Tag[1].K, "Tag[1].K should be equal")
			assert.Equal(t, tt.want.node.Tag[1].V, tt.want.node.Tag[1].V, "Tag[1].V should be equal")
			assert.Equal(t, tt.want.node.Tag[2].K, tt.want.node.Tag[2].K, "Tag[2].K should be equal")
			assert.Equal(t, tt.want.node.Tag[2].V, tt.want.node.Tag[2].V, "Tag[2].V should be equal")
		})
	}
}

func TestCreateNamedSimpleNode(t *testing.T) {
	type args struct {
		id      *int
		node    *OSMUtil.Node
		subtype string
		name    string
	}
	type want struct {
		node OSMUtil.Node
	}
	id := 0
	tests := []struct {
		name string
		args args
		want want
	}{
		{
			name: "creates a node with the correct id and tags",
			args: args{
				id: &id,
				node: &OSMUtil.Node{
					Id:  "10",
					Lat: "1",
					Lon: "2",
				},
				subtype: "test",
				name:    "test",
			},
			want: want{
				node: OSMUtil.Node{
					Id:  "1",
					Lat: "1",
					Lon: "2",
					Tag: []*OSMUtil.Tag{
						{XMLName: XML_TAG_NAME_CONST, K: "type", V: "element"},
						{XMLName: XML_TAG_NAME_CONST, K: "subtype", V: "test"},
						{XMLName: XML_TAG_NAME_CONST, K: "id", V: "test"},
					},
				},
			},
		},
	}

	for _, tt := range tests {
		t.Run(tt.name, func(t *testing.T) {
			node := mapper.CreateSimpleNode(tt.args.id, tt.args.node, tt.args.subtype)
			assert.Equal(t, tt.want.node.Id, node.Id, "Id should be equal")
			assert.Equal(t, tt.want.node.Lat, node.Lat, "Lat should be equal")
			assert.Equal(t, tt.want.node.Lon, node.Lon, "Lon should be equal")
			assert.Equal(t, tt.want.node.Tag[0].K, tt.want.node.Tag[0].K, "Tag[0].K should be equal")
			assert.Equal(t, tt.want.node.Tag[0].V, tt.want.node.Tag[0].V, "Tag[0].V should be equal")
			assert.Equal(t, tt.want.node.Tag[1].K, tt.want.node.Tag[1].K, "Tag[1].K should be equal")
			assert.Equal(t, tt.want.node.Tag[1].V, tt.want.node.Tag[1].V, "Tag[1].V should be equal")
			assert.Equal(t, tt.want.node.Tag[2].K, tt.want.node.Tag[2].K, "Tag[2].K should be equal")
			assert.Equal(t, tt.want.node.Tag[2].V, tt.want.node.Tag[2].V, "Tag[2].V should be equal")
		})
	}
}

func TestCreateSimpleNode(t *testing.T) {
	type args struct {
		id      *int
		node    *OSMUtil.Node
		subtype string
	}
	type want struct {
		node OSMUtil.Node
	}
	id := 0
	tests := []struct {
		name string
		args args
		want want
	}{
		{
			name: "creates a node with the correct id and tags",
			args: args{
				id: &id,
				node: &OSMUtil.Node{
					Id:  "10",
					Lat: "1",
					Lon: "2",
				},
				subtype: "test",
			},
			want: want{
				node: OSMUtil.Node{
					Id:  "1",
					Lat: "1",
					Lon: "2",
					Tag: []*OSMUtil.Tag{
						{XMLName: XML_TAG_NAME_CONST, K: "type", V: "element"},
						{XMLName: XML_TAG_NAME_CONST, K: "subtype", V: "test"},
					},
				},
			},
		},
	}

	for _, tt := range tests {
		t.Run(tt.name, func(t *testing.T) {
			node := mapper.CreateSimpleNode(tt.args.id, tt.args.node, tt.args.subtype)
			assert.Equal(t, tt.want.node.Id, node.Id, "Id should be equal")
			assert.Equal(t, tt.want.node.Lat, node.Lat, "Lat should be equal")
			assert.Equal(t, tt.want.node.Lon, node.Lon, "Lon should be equal")
			assert.Equal(t, tt.want.node.Tag[0].K, tt.want.node.Tag[0].K, "Tag[0].K should be equal")
			assert.Equal(t, tt.want.node.Tag[0].V, tt.want.node.Tag[0].V, "Tag[0].V should be equal")
			assert.Equal(t, tt.want.node.Tag[1].K, tt.want.node.Tag[1].K, "Tag[1].K should be equal")
			assert.Equal(t, tt.want.node.Tag[1].V, tt.want.node.Tag[1].V, "Tag[1].V should be equal")
		})
	}
}
