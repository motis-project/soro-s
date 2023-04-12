package findNodes_test

import (
	"testing"
	findNodes "transform-osm/db-utils/find-nodes"
	osmUtils "transform-osm/osm-utils"

	"github.com/stretchr/testify/assert"
)

func TestFindBestOSMNode(t *testing.T) {
	type args struct {
		osm         *osmUtils.Osm
		anchors     map[float64]([]*osmUtils.Node)
		kilometrage float64
	}
	type want struct {
		newNode *osmUtils.Node
		wantErr bool
	}

	testNodes := []osmUtils.Node{
		{
			Id:  "1",
			Lat: "1.0",
			Lon: "1.0",
		},
		{
			Id:  "2",
			Lat: "2.0",
			Lon: "2.0",
		},
		{
			Id:  "3",
			Lat: "3.0",
			Lon: "3.0",
		},
		{
			Id:  "4",
			Lat: "4.0",
			Lon: "4.0",
		},
		{
			Id:  "5",
			Lat: "5.0",
			Lon: "5.0",
		},
	}
	testData := []osmUtils.Osm{
		{
			Node: []*osmUtils.Node{
				&testNodes[0],
				&testNodes[1],
				&testNodes[2],
				&testNodes[3],
				&testNodes[4],
			},
		},
		{
			Node: []*osmUtils.Node{
				&testNodes[0],
				&testNodes[1],
				&testNodes[4],
			},
			Way: []*osmUtils.Way{
				{
					Nd: []*osmUtils.Nd{
						{Ref: "5"},
						{Ref: "1"},
						{Ref: "2"},
						{Ref: "5"},
					},
				},
			},
		},
	}

	tests := []struct {
		name string
		args args
		want want
	}{
		{
			name: "find new node immediately",
			args: args{
				osm: &testData[0],
				anchors: map[float64]([]*osmUtils.Node){
					1: []*osmUtils.Node{&testNodes[0]},
					2: []*osmUtils.Node{&testNodes[1]},
				},
				kilometrage: 1,
			},
			want: want{
				newNode: &testNodes[0],
				wantErr: false,
			},
		},
		{
			name: "no suitable anchors",
			args: args{
				osm: &testData[0],
				anchors: map[float64]([]*osmUtils.Node){
					1: []*osmUtils.Node{&testNodes[0]},
					2: []*osmUtils.Node{&testNodes[1]},
				},
				kilometrage: 5,
			},
			want: want{
				newNode: nil,
				wantErr: true,
			},
		},
		{
			name: "need two new anchors but find node anyway",
			args: args{
				osm: &testData[1],
				anchors: map[float64]([]*osmUtils.Node){
					1: []*osmUtils.Node{&testNodes[0]},
					2: []*osmUtils.Node{&testNodes[1]},
					3: []*osmUtils.Node{&testNodes[2]},
					4: []*osmUtils.Node{&testNodes[3]},
				},
				kilometrage: 5,
			},
			want: want{
				newNode: &testNodes[4],
				wantErr: false,
			},
		},
	}

	for _, tt := range tests {
		t.Run(tt.name, func(t *testing.T) {
			node, err := findNodes.FindBestOSMNode(tt.args.osm, tt.args.anchors, tt.args.kilometrage)

			if tt.want.wantErr {
				assert.NotNil(t, err)
			} else {
				assert.Nil(t, err)
			}

			assert.EqualValues(t, tt.want.newNode, node)
		})
	}
}
