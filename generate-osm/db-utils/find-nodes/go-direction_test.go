package findNodes_test

import (
	"testing"
	findNodes "transform-osm/db-utils/find-nodes"
	osmUtils "transform-osm/osm-utils"

	"github.com/stretchr/testify/assert"
)

func TestGoDirection(t *testing.T) {
	type args struct {
		osm             *osmUtils.Osm
		runningWay      osmUtils.Way
		index           int
		dist            float64
		initialWayDirUp bool
	}
	type want struct {
		finalNodeID string
		totalDist   float64
		wantErr     bool
	}
	tests := []struct {
		name string
		args args
		want want
	}{
		{
			name: "failing to initialize oldNode",
			args: args{
				osm: &osmUtils.Osm{
					Node: []*osmUtils.Node{
						{Id: "1"},
					},
					Way: []*osmUtils.Way{
						{
							Nd: []*osmUtils.Nd{
								{Ref: "1"},
								{Ref: "2"},
							},
						},
					},
				},
				runningWay: osmUtils.Way{
					Nd: []*osmUtils.Nd{
						{Ref: "1"},
						{Ref: "2"},
					},
				},
				index:           0,
				dist:            5,
				initialWayDirUp: false,
			},
			want: want{
				finalNodeID: "",
				totalDist:   0,
				wantErr:     true,
			},
		},
		{
			name: "hit end right away",
			args: args{
				osm: &osmUtils.Osm{
					Node: []*osmUtils.Node{
						{Id: "1"},
					},
					Way: []*osmUtils.Way{
						{
							Nd: []*osmUtils.Nd{
								{Ref: "1"},
								{Ref: "2"},
							},
						},
					},
				},
				runningWay: osmUtils.Way{
					Nd: []*osmUtils.Nd{
						{Ref: "1"},
						{Ref: "2"},
					},
				},
				index:           0,
				dist:            5,
				initialWayDirUp: true,
			},
			want: want{
				finalNodeID: "1",
				totalDist:   0,
				wantErr:     true,
			},
		},
		{
			name: "go into other error right away",
			args: args{
				osm: &osmUtils.Osm{
					Node: []*osmUtils.Node{
						{Id: "1"},
					},
					Way: []*osmUtils.Way{
						{
							Nd: []*osmUtils.Nd{
								{Ref: "2"},
								{Ref: "3"},
							},
						},
					},
				},
				runningWay: osmUtils.Way{
					Nd: []*osmUtils.Nd{
						{Ref: "1"},
						{Ref: "2"},
					},
				},
				index:           0,
				dist:            5,
				initialWayDirUp: true,
			},
			want: want{
				finalNodeID: "",
				totalDist:   0,
				wantErr:     true,
			},
		},
		{
			name: "go into node finding error right away",
			args: args{
				osm: &osmUtils.Osm{
					Node: []*osmUtils.Node{
						{Id: "1"},
					},
					Way: []*osmUtils.Way{
						{
							Nd: []*osmUtils.Nd{
								{Ref: "2"},
								{Ref: "3"},
								{Ref: "4"},
							},
						},
					},
				},
				runningWay: osmUtils.Way{
					Nd: []*osmUtils.Nd{
						{Ref: "2"},
						{Ref: "3"},
						{Ref: "4"},
					},
				},
				index:           1,
				dist:            5,
				initialWayDirUp: true,
			},
			want: want{
				finalNodeID: "",
				totalDist:   0,
				wantErr:     true,
			},
		},
		{
			name: "actually go some distance: correct distance",
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
								{Ref: "1"},
								{Ref: "2"},
								{Ref: "3"},
							},
						},
					},
				},
				runningWay: osmUtils.Way{
					Nd: []*osmUtils.Nd{
						{Ref: "1"},
						{Ref: "2"},
						{Ref: "3"},
					},
				},
				index:           1,
				dist:            157.22543203807288,
				initialWayDirUp: true,
			},
			want: want{
				finalNodeID: "1",
				totalDist:   157.22543203807288,
				wantErr:     false,
			},
		},
		{
			name: "actually go some distance: overshoot a little 1",
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
								{Ref: "1"},
								{Ref: "2"},
								{Ref: "3"},
							},
						},
					},
				},
				runningWay: osmUtils.Way{
					Nd: []*osmUtils.Nd{
						{Ref: "1"},
						{Ref: "2"},
						{Ref: "3"},
					},
				},
				index:           1,
				dist:            5,
				initialWayDirUp: true,
			},
			want: want{
				finalNodeID: "1",
				totalDist:   157.22543203807288,
				wantErr:     false,
			},
		},
		{
			name: "actually go some distance: overshoot a little 2",
			args: args{
				osm: &osmUtils.Osm{
					Node: []*osmUtils.Node{
						{
							Id:  "2",
							Lat: "1.0",
							Lon: "1.0",
						},
						{
							Id:  "3",
							Lat: "2.0",
							Lon: "2.0",
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
				runningWay: osmUtils.Way{
					Nd: []*osmUtils.Nd{
						{Ref: "1"},
						{Ref: "2"},
						{Ref: "3"},
					},
				},
				index:           1,
				dist:            5,
				initialWayDirUp: false,
			},
			want: want{
				finalNodeID: "3",
				totalDist:   157.22543203807288,
				wantErr:     false,
			},
		},
	}

	for _, tt := range tests {
		t.Run(tt.name, func(t *testing.T) {
			resultNodeID, totalDist, err := findNodes.GoDirection(tt.args.osm, tt.args.runningWay, tt.args.index, tt.args.dist, tt.args.initialWayDirUp)

			if tt.want.wantErr {
				assert.NotNil(t, err)
			} else {
				assert.Nil(t, err)
			}

			assert.Equal(t, tt.want.finalNodeID, resultNodeID)
			assert.Equal(t, tt.want.totalDist, totalDist)
		})
	}
}
