package findNodes_test

import (
	"testing"
	findNodes "transform-osm/db-utils/find-nodes"
	osmUtils "transform-osm/osm-utils"

	"github.com/stretchr/testify/assert"
)

func TestGetWayNdRef(t *testing.T) {
	type args struct {
		way           osmUtils.Way
		ndRefPosition findNodes.NdRefPosition
	}
	tests := []struct {
		name string
		args args
		want string
	}{
		{
			name: "get first nd ref",
			args: args{
				way: osmUtils.Way{
					Nd: []*osmUtils.Nd{
						{
							Ref: "1",
						},
						{
							Ref: "2",
						},
					},
				},
				ndRefPosition: findNodes.FirstNdRef,
			},
			want: "1",
		},
		{
			name: "get second nd ref",
			args: args{
				way: osmUtils.Way{
					Nd: []*osmUtils.Nd{
						{
							Ref: "1",
						},
						{
							Ref: "2",
						},
					},
				},
				ndRefPosition: findNodes.SecondNdRef,
			},
			want: "2",
		},
		{
			name: "get second last nd ref",
			args: args{
				way: osmUtils.Way{
					Nd: []*osmUtils.Nd{
						{
							Ref: "1",
						},
						{
							Ref: "2",
						},
						{
							Ref: "3",
						},
					},
				},
				ndRefPosition: findNodes.SecondLastNdRef,
			},
			want: "2",
		},
		{
			name: "get last nd ref",
			args: args{
				way: osmUtils.Way{
					Nd: []*osmUtils.Nd{
						{
							Ref: "1",
						},
						{
							Ref: "2",
						},
						{
							Ref: "3",
						},
					},
				},
				ndRefPosition: findNodes.LastNdRef,
			},
			want: "3",
		},
	}

	for _, tt := range tests {
		t.Run(tt.name, func(t *testing.T) {
			if got := findNodes.GetWayNdRef(tt.args.way, tt.args.ndRefPosition); got != tt.want {
				t.Errorf("GetWayNdRef() = %v, want %v", got, tt.want)
			}
		})
	}
}

func TestSortAnchors(t *testing.T) {
	type args struct {
		anchors     map[float64]([]*osmUtils.Node)
		kilometrage float64
	}
	tests := []struct {
		name string
		args args
		want []float64
	}{
		{
			name: "sort anchors by distance to kilometrage",
			args: args{
				anchors: map[float64]([]*osmUtils.Node){
					1.0: {
						{
							Id: "1",
						},
					},
					2.0: {
						{
							Id: "2",
						},
					},
					3.0: {
						{
							Id: "3",
						},
					},
				},
				kilometrage: 2.4,
			},
			want: []float64{2.0, 3.0, 1.0},
		},
	}

	for _, tt := range tests {
		t.Run(tt.name, func(t *testing.T) {
			got := findNodes.SortAnchors(tt.args.anchors, tt.args.kilometrage)
			assert.Equal(t, tt.want, got)
		})
	}
}

func TestFormatKilometrageStringInFloat(t *testing.T) {
	type args struct {
		kilometrage string
	}
	type want struct {
		kilometrage float64
		wantErr     bool
	}
	tests := []struct {
		name string
		args args
		want want
	}{
		{
			name: "format kilometrage string in float",
			args: args{
				kilometrage: "1,000",
			},
			want: want{
				kilometrage: 1.000,
				wantErr:     false,
			},
		},
		{
			name: "format kilometrage string in float",
			args: args{
				kilometrage: "2,500",
			},
			want: want{
				kilometrage: 2.500,
				wantErr:     false,
			},
		},
		{
			name: "format kilometrage string in float with +",
			args: args{
				kilometrage: "5,000+0,150",
			},
			want: want{
				kilometrage: 5.150,
				wantErr:     false,
			},
		},
		{
			name: "throws error when kilometrage is not a float",
			args: args{
				kilometrage: "2,abc",
			},
			want: want{
				kilometrage: 0.0,
				wantErr:     true,
			},
		},
		{
			name: "throws error when kilometrage is not a float",
			args: args{
				kilometrage: "2,000+0,abc",
			},
			want: want{
				kilometrage: 0.0,
				wantErr:     true,
			},
		},
	}

	for _, tt := range tests {
		t.Run(tt.name, func(t *testing.T) {
			kilometrage, err := findNodes.FormatKilometrageStringInFloat(tt.args.kilometrage)

			if tt.want.wantErr {
				assert.NotNil(t, err)
			} else {
				assert.Nil(t, err)
			}

			assert.Equal(t, tt.want.kilometrage, kilometrage)
		})
	}
}

func TestComputeNodeInformation(t *testing.T) {
	type args struct {
		osm    *osmUtils.Osm
		nodeId string
	}
	type want struct {
		node    *osmUtils.Node
		nodeLat float64
		nodeLon float64
		errNil  bool
	}
	tests := []struct {
		name string
		args args
		want want
	}{
		{
			name: "compute node information",
			args: args{
				osm: &osmUtils.Osm{
					Node: []*osmUtils.Node{
						{
							Id:  "1",
							Lat: "1.0",
							Lon: "2.0",
						},
					},
				},
				nodeId: "1",
			},
			want: want{
				node: &osmUtils.Node{
					Id:  "1",
					Lat: "1.0",
					Lon: "2.0",
				},
				nodeLat: 1.0,
				nodeLon: 2.0,
				errNil:  true,
			},
		},
		{
			name: "throws error when node is not found",
			args: args{
				osm: &osmUtils.Osm{
					Node: []*osmUtils.Node{
						{
							Id:  "1",
							Lat: "1.0",
							Lon: "2.0",
						},
					},
				},
				nodeId: "2",
			},
			want: want{
				node:    nil,
				nodeLat: 0.0,
				nodeLon: 0.0,
				errNil:  false,
			},
		},
	}

	for _, tt := range tests {
		t.Run(tt.name, func(t *testing.T) {
			node, nodeLat, nodeLon, err := findNodes.ComputeNodeInformation(tt.args.osm, tt.args.nodeId)

			if tt.want.errNil {
				assert.Nil(t, err)
			} else {
				assert.NotNil(t, err)
			}

			assert.Equal(t, tt.want.node, node)
			assert.Equal(t, tt.want.nodeLat, nodeLat)
			assert.Equal(t, tt.want.nodeLon, nodeLon)
		})
	}

}

func TestComputeHaversineDistance(t *testing.T) {
	type args struct {
		lat1 float64
		lon1 float64
		lat2 float64
		lon2 float64
	}
	type want struct {
		distance float64
	}
	tests := []struct {
		name string
		args args
		want want
	}{
		{
			name: "compute haversine distance",
			args: args{
				lat1: 1.0,
				lon1: 2.0,
				lat2: 3.0,
				lon2: 4.0,
			},
			want: want{
				distance: 314.4029510236249,
			},
		},
	}

	for _, tt := range tests {
		t.Run(tt.name, func(t *testing.T) {
			distance := findNodes.ComputeHaversineDistance(tt.args.lat1, tt.args.lat2, tt.args.lon1, tt.args.lon2)
			assert.Equal(t, tt.want.distance, distance)
		})
	}
}

func TestFindNextRunningNode(t *testing.T) {
	type args struct {
		osm        *osmUtils.Osm
		wayDirUp   bool
		index      int
		runningWay osmUtils.Way
	}
	type want struct {
		nextNode *osmUtils.Node
		errNil   bool
	}
	tests := []struct {
		name string
		args args
		want want
	}{
		{
			name: "find next running node up",
			args: args{
				osm: &osmUtils.Osm{
					Node: []*osmUtils.Node{
						{
							Id: "1",
						},
						{
							Id: "2",
						},
					},
				},
				wayDirUp: true,
				index:    1,
				runningWay: osmUtils.Way{
					Nd: []*osmUtils.Nd{
						{
							Ref: "1",
						},
						{
							Ref: "2",
						},
					},
				},
			},
			want: want{
				nextNode: &osmUtils.Node{
					Id: "1",
				},
				errNil: true,
			},
		},
		{
			name: "find next running node down",
			args: args{
				osm: &osmUtils.Osm{
					Node: []*osmUtils.Node{
						{
							Id: "1",
						},
						{
							Id: "2",
						},
					},
				},
				wayDirUp: false,
				index:    0,
				runningWay: osmUtils.Way{
					Nd: []*osmUtils.Nd{
						{
							Ref: "1",
						},
						{
							Ref: "2",
						},
					},
				},
			},
			want: want{
				nextNode: &osmUtils.Node{
					Id: "2",
				},
				errNil: true,
			},
		},
		{
			name: "throws error when next node is not found",
			args: args{
				osm: &osmUtils.Osm{
					Node: []*osmUtils.Node{
						{
							Id: "1",
						},
						{
							Id: "2",
						},
					},
				},
				wayDirUp: false,
				index:    1,
				runningWay: osmUtils.Way{
					Nd: []*osmUtils.Nd{
						{
							Ref: "1",
						},
						{
							Ref: "2",
						},
						{
							Ref: "3",
						},
					},
				},
			},
			want: want{
				nextNode: nil,
				errNil:   false,
			},
		},
	}

	for _, tt := range tests {
		t.Run(tt.name, func(t *testing.T) {
			nextNode, err := findNodes.FindNextRunningNode(tt.args.osm, tt.args.wayDirUp, tt.args.index, tt.args.runningWay)

			if tt.want.errNil {
				assert.Nil(t, err)
			} else {
				assert.NotNil(t, err)
			}

			assert.Equal(t, tt.want.nextNode, nextNode)
		})
	}
}
