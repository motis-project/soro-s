package findNodes_test

import (
	"testing"
	findNodes "transform-osm/db-utils/find-nodes"
	osmUtils "transform-osm/osm-utils"

	"github.com/stretchr/testify/assert"
)

func TestFindClosestMatch(t *testing.T) {
	type args struct {
		osm                                    *osmUtils.Osm
		up1ID, up2ID, down1ID, down2ID         string
		upDist1, upDist2, downDist1, downDist2 float64
	}
	type want struct {
		node    *osmUtils.Node
		wantErr bool
	}
	tests := []struct {
		name string
		args args
		want want
	}{
		{
			name: "up1 node not in OSM",
			args: args{
				osm:   &osmUtils.Osm{},
				up1ID: "1", up2ID: "2", down1ID: "3", down2ID: "4",
				upDist1: 0, upDist2: 0, downDist1: 0, downDist2: 0,
			},
			want: want{
				node:    nil,
				wantErr: true,
			},
		},
		{
			name: "up2 node not in OSM",
			args: args{
				osm: &osmUtils.Osm{
					Node: []*osmUtils.Node{
						{Id: "1"},
					},
				},
				up1ID: "1", up2ID: "2", down1ID: "3", down2ID: "4",
				upDist1: 0, upDist2: 0, downDist1: 0, downDist2: 0,
			},
			want: want{
				node:    nil,
				wantErr: true,
			},
		},
		{
			name: "down1 node not in OSM",
			args: args{
				osm: &osmUtils.Osm{
					Node: []*osmUtils.Node{
						{Id: "1"},
						{Id: "2"},
					},
				},
				up1ID: "1", up2ID: "2", down1ID: "3", down2ID: "4",
				upDist1: 0, upDist2: 0, downDist1: 0, downDist2: 0,
			},
			want: want{
				node:    nil,
				wantErr: true,
			},
		},
		{
			name: "down2 node not in OSM",
			args: args{
				osm: &osmUtils.Osm{
					Node: []*osmUtils.Node{
						{Id: "1"},
						{Id: "2"},
						{Id: "3"},
					},
				},
				up1ID: "1", up2ID: "2", down1ID: "3", down2ID: "4",
				upDist1: 0, upDist2: 0, downDist1: 0, downDist2: 0,
			},
			want: want{
				node:    nil,
				wantErr: true,
			},
		},
		{
			name: "downNode1 gets chosen for lower remDist and min dist to down2",
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
				},
				up1ID: "1", up2ID: "2", down1ID: "3", down2ID: "4",
				upDist1: 0, upDist2: 0, downDist1: 0, downDist2: 1,
			},
			want: want{
				node: &osmUtils.Node{
					Id:  "3",
					Lat: "3.0",
					Lon: "3.0",
				},
				wantErr: false,
			},
		},
		{
			name: "downNode2 gets chosen for lower remDist and min dist to down1",
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
				},
				up1ID: "1", up2ID: "2", down1ID: "3", down2ID: "4",
				upDist1: 0, upDist2: 0, downDist1: 1, downDist2: 0,
			},
			want: want{
				node: &osmUtils.Node{
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
			closestMatchNode, err := findNodes.FindClosestMatch(tt.args.osm, tt.args.up1ID, tt.args.up2ID, tt.args.down1ID, tt.args.down2ID, tt.args.upDist1, tt.args.upDist2, tt.args.downDist1, tt.args.downDist2)

			if tt.want.wantErr {
				assert.NotNil(t, err)
			} else {
				assert.Nil(t, err)
			}

			assert.EqualValues(t, tt.want.node, closestMatchNode)
		})
	}
}
