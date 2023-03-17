package findNodes_test

import (
	"testing"
	findNodes "transform-osm/db-utils/find-nodes"
	osmUtils "transform-osm/osm-utils"

	"github.com/stretchr/testify/assert"
)

func TestFindNode(t *testing.T) {
	type args struct {
		osm        *osmUtils.Osm
		anchorNode *osmUtils.Node
		distance   float64
	}
	type want struct {
		upNodeID, downNodeID string
		upDist, downDist     float64
		wantErr              bool
	}
	tests := []struct {
		name string
		args args
		want want
	}{
		{
			name: "no starting ways exist",
			args: args{
				osm: &osmUtils.Osm{},
				anchorNode: &osmUtils.Node{
					Id: "1",
				},
				distance: 0,
			},
			want: want{
				upNodeID: "", downNodeID: "",
				upDist: 0, downDist: 0,
				wantErr: true,
			},
		},
		{
			name: "too many starting ways exist",
			args: args{
				osm: &osmUtils.Osm{
					Way: []*osmUtils.Way{
						{
							Nd: []*osmUtils.Nd{
								{Ref: "1"},
								{Ref: "2"},
							},
						},
						{
							Nd: []*osmUtils.Nd{
								{Ref: "1"},
								{Ref: "3"},
							},
						},
						{
							Nd: []*osmUtils.Nd{
								{Ref: "4"},
								{Ref: "1"},
							},
						},
					},
				},
				anchorNode: &osmUtils.Node{
					Id: "1",
				},
				distance: 0,
			},
			want: want{
				upNodeID: "", downNodeID: "",
				upDist: 0, downDist: 0,
				wantErr: true,
			},
		},
		{
			name: "one start way: Failing to go up",
			args: args{
				osm: &osmUtils.Osm{
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
				anchorNode: &osmUtils.Node{
					Id: "2",
				},
				distance: 1,
			},
			want: want{
				upNodeID: "", downNodeID: "",
				upDist: 0, downDist: 0,
				wantErr: true,
			},
		},
		{
			name: "one start way: Failing to go down",
			args: args{
				osm: &osmUtils.Osm{
					Node: []*osmUtils.Node{
						{
							Id:  "0",
							Lat: "0.0",
							Lon: "0.0",
						},
						{
							Id:  "1",
							Lat: "1.0",
							Lon: "1.0",
						},
					},
					Way: []*osmUtils.Way{
						{
							Nd: []*osmUtils.Nd{
								{Ref: "0"},
								{Ref: "1"},
								{Ref: "2"},
							},
						},
					},
				},
				anchorNode: &osmUtils.Node{
					Id: "1",
				},
				distance: 1,
			},
			want: want{
				upNodeID: "", downNodeID: "",
				upDist: 0, downDist: 0,
				wantErr: true,
			},
		},
		{
			name: "one start way: Successfully going up and down",
			args: args{
				osm: &osmUtils.Osm{
					Node: []*osmUtils.Node{
						{Id: "2"},
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
				anchorNode: &osmUtils.Node{
					Id: "2",
				},
				distance: 0,
			},
			want: want{
				upNodeID: "2", downNodeID: "2",
				upDist: 0, downDist: 0,
				wantErr: false,
			},
		},
		{
			name: "two start ways: Failing setting startingWays",
			args: args{
				osm: &osmUtils.Osm{
					Way: []*osmUtils.Way{
						{
							Nd: []*osmUtils.Nd{
								{Ref: "1"},
								{Ref: "2"},
							},
						}, {
							Nd: []*osmUtils.Nd{
								{Ref: "3"},
								{Ref: "2"},
							},
						},
					},
				},
				anchorNode: &osmUtils.Node{
					Id: "2",
				},
				distance: 0,
			},
			want: want{
				upNodeID: "", downNodeID: "",
				upDist: 0, downDist: 0,
				wantErr: true,
			},
		},
		{
			name: "two start ways: Failing to go up",
			args: args{
				osm: &osmUtils.Osm{
					Way: []*osmUtils.Way{
						{
							Nd: []*osmUtils.Nd{
								{Ref: "1"},
								{Ref: "2"},
							},
						},
						{
							Nd: []*osmUtils.Nd{
								{Ref: "2"},
								{Ref: "3"},
							},
						},
					},
				},
				anchorNode: &osmUtils.Node{
					Id: "2",
				},
				distance: 1,
			},
			want: want{
				upNodeID: "", downNodeID: "",
				upDist: 0, downDist: 0,
				wantErr: true,
			},
		},
		{
			name: "two start ways: Failing to go down",
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
					},
					Way: []*osmUtils.Way{
						{
							Nd: []*osmUtils.Nd{
								{Ref: "2"},
								{Ref: "3"},
							},
						},
						{
							Nd: []*osmUtils.Nd{
								{Ref: "1"},
								{Ref: "2"},
							},
						},
					},
				},
				anchorNode: &osmUtils.Node{
					Id: "2",
				},
				distance: 1,
			},
			want: want{
				upNodeID: "", downNodeID: "",
				upDist: 0, downDist: 0,
				wantErr: true,
			},
		},
		{
			name: "two start ways: Successfully go up and down",
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
							},
						},
						{
							Nd: []*osmUtils.Nd{
								{Ref: "2"},
								{Ref: "3"},
							},
						},
					},
				},
				anchorNode: &osmUtils.Node{
					Id: "2",
				},
				distance: 1,
			},
			want: want{
				upNodeID: "1", downNodeID: "3",
				upDist: 157.22543203807288, downDist: 157.17755181464074,
				wantErr: false,
			},
		},
	}

	for _, tt := range tests {
		t.Run(tt.name, func(t *testing.T) {
			upID, upDist, downID, downDist, err := findNodes.FindNodes(tt.args.osm, tt.args.anchorNode, tt.args.distance)

			if tt.want.wantErr {
				assert.NotNil(t, err)
			} else {
				assert.Nil(t, err)
			}

			assert.Equal(t, tt.want.upNodeID, upID)
			assert.Equal(t, tt.want.downNodeID, downID)
			assert.Equal(t, tt.want.upDist, upDist)
			assert.Equal(t, tt.want.downDist, downDist)
		})
	}
}
