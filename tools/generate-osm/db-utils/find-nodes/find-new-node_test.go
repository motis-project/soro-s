package findNodes_test

import (
	"testing"
	findNodes "transform-osm/db-utils/find-nodes"
	osmUtils "transform-osm/osm-utils"

	"github.com/stretchr/testify/assert"
)

func TestFindNewNode(t *testing.T) {
	type args struct {
		osm                      *osmUtils.Osm
		anchorNode1, anchorNode2 *osmUtils.Node
		dist1, dist2             float64
	}
	type want struct {
		newNode *osmUtils.Node
		wantErr bool
	}
	tests := []struct {
		name string
		args args
		want want
	}{
		{
			name: "no calc needed",
			args: args{
				osm: &osmUtils.Osm{},
				anchorNode1: &osmUtils.Node{
					Id: "1",
				},
				anchorNode2: &osmUtils.Node{
					Id: "2",
				},
				dist1: 0,
				dist2: 2,
			},
			want: want{
				newNode: &osmUtils.Node{
					Id: "1",
				},
				wantErr: false,
			},
		},
		{
			name: "anchor 1 insufficient",
			args: args{
				osm: &osmUtils.Osm{},
				anchorNode1: &osmUtils.Node{
					Id: "1",
				},
				anchorNode2: &osmUtils.Node{
					Id: "2",
				},
				dist1: 1,
				dist2: 2,
			},
			want: want{
				newNode: nil,
				wantErr: true,
			},
		},
		{
			name: "anchor 2 insufficient",
			args: args{
				osm: &osmUtils.Osm{
					Node: []*osmUtils.Node{
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
					},
					Way: []*osmUtils.Way{
						{
							Nd: []*osmUtils.Nd{
								{Ref: "1"},
								{Ref: "2"},
								{Ref: "3"},
							},
						},
					},
				},
				anchorNode1: &osmUtils.Node{
					Id: "2",
				},
				anchorNode2: &osmUtils.Node{
					Id: "4",
				},
				dist1: 1,
				dist2: 2,
			},
			want: want{
				newNode: nil,
				wantErr: true,
			},
		},
		{
			name: "no insufficiencies 1",
			args: args{
				osm: &osmUtils.Osm{
					Node: []*osmUtils.Node{
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
					},
					Way: []*osmUtils.Way{
						{
							Nd: []*osmUtils.Nd{
								{Ref: "1"},
								{Ref: "2"},
								{Ref: "3"},
								{Ref: "1"},
							},
						},
					},
				},
				anchorNode1: &osmUtils.Node{
					Id: "2",
				},
				anchorNode2: &osmUtils.Node{
					Id: "3",
				},
				dist1: 1,
				dist2: 2,
			},
			want: want{
				newNode: &osmUtils.Node{
					Id:  "1",
					Lat: "1.0",
					Lon: "1.0",
				},
				wantErr: false,
			},
		},
		{
			name: "no insufficiencies 2",
			args: args{
				osm: &osmUtils.Osm{
					Node: []*osmUtils.Node{
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
					},
					Way: []*osmUtils.Way{
						{
							Nd: []*osmUtils.Nd{
								{Ref: "1"},
								{Ref: "2"},
								{Ref: "3"},
								{Ref: "1"},
							},
						},
					},
				},
				anchorNode1: &osmUtils.Node{
					Id: "2",
				},
				anchorNode2: &osmUtils.Node{
					Id: "3",
				},
				dist1: 1,
				dist2: 0,
			},
			want: want{
				newNode: &osmUtils.Node{
					Id:  "3",
					Lat: "3.0",
					Lon: "3.0",
				},
				wantErr: false,
			},
		},
		{
			name: "no insufficiencies 3",
			args: args{
				osm: &osmUtils.Osm{
					Node: []*osmUtils.Node{
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
					},
					Way: []*osmUtils.Way{
						{
							Nd: []*osmUtils.Nd{
								{Ref: "1"},
								{Ref: "2"},
								{Ref: "3"},
								{Ref: "4"},
							},
						},
					},
				},
				anchorNode1: &osmUtils.Node{
					Id: "2",
				},
				anchorNode2: &osmUtils.Node{
					Id: "3",
				},
				dist1: 1,
				dist2: 2,
			},
			want: want{
				newNode: &osmUtils.Node{
					Id:  "4",
					Lat: "4.0",
					Lon: "4.0",
				},
				wantErr: false,
			},
		},
	}

	for _, tt := range tests {
		t.Run(tt.name, func(t *testing.T) {
			node, err := findNodes.FindNewNode(tt.args.osm, tt.args.anchorNode1, tt.args.anchorNode2, tt.args.dist1, tt.args.dist2)

			if tt.want.wantErr {
				assert.NotNil(t, err)
			} else {
				assert.Nil(t, err)
			}

			assert.EqualValues(t, tt.want.newNode, node)
		})
	}
}
