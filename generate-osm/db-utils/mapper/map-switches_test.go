package mapper_test

import (
	"testing"
	"transform-osm/db-utils/mapper"
	osmUtils "transform-osm/osm-utils"

	"github.com/stretchr/testify/assert"
)

func TestFindAndMapAnchorSwitches(t *testing.T) {
	type args struct {
		knoten           mapper.Spurplanknoten
		osm              *osmUtils.Osm
		anchors          map[float64][]*osmUtils.Node
		notFoundSwitches *[]*mapper.Weichenanfang
		foundAnchorCount *int
		nodeIdCounter    *int
	}
	type want struct {
		isError          bool
		notFoundSwitches int
		foundAnchors     map[float64][]*osmUtils.Node
	}
	tests := []struct {
		name string
		args args
		want want
	}{
		{
			name: "find and map anchor switches",
			args: args{
				knoten: mapper.Spurplanknoten{
					WeichenAnf: []*mapper.Weichenanfang{
						{
							Name: mapper.Wert{
								Value: "Weiche 1",
							},
							KnotenTyp: mapper.KnotenTyp{
								Kilometrierung: mapper.Wert{
									Value: "1.000",
								},
							},
						},
						{
							Name: mapper.Wert{
								Value: "Weiche 2",
							},
							KnotenTyp: mapper.KnotenTyp{
								Kilometrierung: mapper.Wert{
									Value: "2.000",
								},
							},
						},
					},
				},
				osm: &osmUtils.Osm{
					Node: []*osmUtils.Node{
						{
							Id:  "10",
							Lat: "1.0",
							Lon: "1.0",
							Tag: []*osmUtils.Tag{
								{
									K: "railway",
									V: "switch",
								},
								{
									K: "ref",
									V: "Weiche 1",
								},
							},
						},
						{
							Id:  "11",
							Lat: "2.0",
							Lon: "2.0",
							Tag: []*osmUtils.Tag{
								{
									K: "railway",
									V: "switch",
								},
								{
									K: "ref",
									V: "Weiche 2",
								},
							},
						},
					},
				},
				anchors:          map[float64][]*osmUtils.Node{},
				notFoundSwitches: &[]*mapper.Weichenanfang{},
				foundAnchorCount: new(int),
				nodeIdCounter:    new(int),
			},
			want: want{
				isError:          false,
				notFoundSwitches: 0,
				foundAnchors: map[float64][]*osmUtils.Node{
					1: {
						{
							Id:  "1",
							Lat: "1.0",
							Lon: "1.0",
							Tag: []*osmUtils.Tag{
								{
									K: "railway",
									V: "switch",
								},
								{
									K: "ref",
									V: "Weiche 1",
								},
							},
						},
					},
					2: {
						{
							Id:  "2",
							Lat: "2.0",
							Lon: "2.0",
							Tag: []*osmUtils.Tag{
								{
									K: "railway",
									V: "switch",
								},
								{
									K: "ref",
									V: "Weiche 2",
								},
							},
						},
					},
				},
			},
		},
		{
			name: "find and map anchor switche by name",
			args: args{
				knoten: mapper.Spurplanknoten{
					WeichenAnf: []*mapper.Weichenanfang{
						{
							Name: mapper.Wert{
								Value: "Weiche 1",
							},
							KnotenTyp: mapper.KnotenTyp{
								Kilometrierung: mapper.Wert{
									Value: "1.000",
								},
							},
						},
					},
				},
				osm: &osmUtils.Osm{
					Node: []*osmUtils.Node{
						{
							Id:  "10",
							Lat: "1.0",
							Lon: "1.0",
							Tag: []*osmUtils.Tag{
								{
									K: "railway",
									V: "switch",
								},
								{
									K: "name",
									V: "Weiche 1",
								},
							},
						},
					},
				},
				anchors:          map[float64][]*osmUtils.Node{},
				notFoundSwitches: &[]*mapper.Weichenanfang{},
				foundAnchorCount: new(int),
				nodeIdCounter:    new(int),
			},
			want: want{
				isError:          false,
				notFoundSwitches: 0,
				foundAnchors: map[float64][]*osmUtils.Node{
					1: {
						{
							Id:  "1",
							Lat: "1.0",
							Lon: "1.0",
							Tag: []*osmUtils.Tag{
								{
									K: "railway",
									V: "switch",
								},
								{
									K: "ref",
									V: "Weiche 1",
								},
							},
						},
					},
				},
			},
		},
		{
			name: "cannot find anchor switchese",
			args: args{
				knoten: mapper.Spurplanknoten{
					WeichenAnf: []*mapper.Weichenanfang{
						{
							Name: mapper.Wert{
								Value: "NotWeiche 1",
							},
							KnotenTyp: mapper.KnotenTyp{
								Kilometrierung: mapper.Wert{
									Value: "1.000",
								},
							},
						},
						{
							Name: mapper.Wert{
								Value: "NotWeiche 2",
							},
							KnotenTyp: mapper.KnotenTyp{
								Kilometrierung: mapper.Wert{
									Value: "2.000",
								},
							},
						},
					},
				},
				osm: &osmUtils.Osm{
					Node: []*osmUtils.Node{
						{
							Id:  "10",
							Lat: "1.0",
							Lon: "1.0",
							Tag: []*osmUtils.Tag{
								{
									K: "railway",
									V: "switch",
								},
								{
									K: "ref",
									V: "Weiche 1",
								},
							},
						},
						{
							Id:  "11",
							Lat: "2.0",
							Lon: "2.0",
							Tag: []*osmUtils.Tag{
								{
									K: "railway",
									V: "switch",
								},
								{
									K: "ref",
									V: "Weiche 2",
								},
							},
						},
					},
				},
				anchors:          map[float64][]*osmUtils.Node{},
				notFoundSwitches: &[]*mapper.Weichenanfang{},
				foundAnchorCount: new(int),
				nodeIdCounter:    new(int),
			},
			want: want{
				isError:          false,
				notFoundSwitches: 2,
				foundAnchors:     map[float64][]*osmUtils.Node{},
			},
		},
	}

	for _, tt := range tests {
		t.Run(tt.name, func(t *testing.T) {
			err := mapper.FindAndMapAnchorSwitches(
				tt.args.knoten,
				tt.args.osm,
				tt.args.anchors,
				tt.args.notFoundSwitches,
				tt.args.foundAnchorCount,
				tt.args.nodeIdCounter,
			)
			if tt.want.isError {
				assert.Error(t, err)
			} else {
				assert.NoError(t, err)
			}
			assert.Equal(t, tt.want.notFoundSwitches, len(*tt.args.notFoundSwitches))
			assert.Equal(t, len(tt.want.foundAnchors), *tt.args.foundAnchorCount)
		})
	}
}

func TestMapUnanchoredSwitches(t *testing.T) {
	type args struct {
		osm           *osmUtils.Osm
		anchors       map[float64][]*osmUtils.Node
		nodeIdCounter *int
		knoten        mapper.Spurplanknoten
		tracker       mapper.NotFoundElementTracker
	}
	type want struct {
		isError          bool
		elementsNotFound map[mapper.ElementType](int)
	}
	tests := []struct {
		name string
		args args
		want want
	}{
		{
			name: "find and map unanchored switch",
			args: args{
				osm: &osmUtils.Osm{
					Node: []*osmUtils.Node{
						{
							Id:  "10",
							Lat: "1.0",
							Lon: "1.0",
						},
						{
							Id:  "11",
							Lat: "2.0",
							Lon: "2.0",
						},
						{
							Id:  "12",
							Lat: "3.0",
							Lon: "3.0",
						},
						{
							Id:  "13",
							Lat: "4.0",
							Lon: "4.0",
						},
					},
					Way: []*osmUtils.Way{
						{
							Id: "100",
							Nd: []*osmUtils.Nd{
								{
									Ref: "10",
								},
								{
									Ref: "11",
								},
							},
						},
						{
							Id: "101",
							Nd: []*osmUtils.Nd{
								{
									Ref: "11",
								},
								{
									Ref: "12",
								},
							},
						},
						{
							Id: "102",
							Nd: []*osmUtils.Nd{
								{
									Ref: "12",
								},
								{
									Ref: "13",
								},
							},
						},
					},
				},
				anchors: map[float64]([]*osmUtils.Node){
					1: {
						{
							Id:  "11",
							Lat: "1.0",
							Lon: "1.0",
						},
					},
					3: {
						{
							Id:  "12",
							Lat: "2.0",
							Lon: "2.0",
						},
					},
				},
				nodeIdCounter: new(int),
				knoten: mapper.Spurplanknoten{
					WeichenAnf: []*mapper.Weichenanfang{
						{
							Name: mapper.Wert{
								Value: "Weiche 1",
							},
							KnotenTyp: mapper.KnotenTyp{
								Kilometrierung: mapper.Wert{
									Value: "1.000",
								},
							},
						},
					},
				},
				tracker: mapper.NewNotFoundElementTracker(),
			},
			want: want{
				isError: false,
				elementsNotFound: map[mapper.ElementType](int){
					mapper.Switches: 0,
				},
			},
		},
		{
			name: "could not find unanchored switch",
			args: args{
				osm: &osmUtils.Osm{
					Node: []*osmUtils.Node{
						{
							Id:  "10",
							Lat: "1.0",
							Lon: "1.0",
						},
						{
							Id:  "11",
							Lat: "2.0",
							Lon: "2.0",
						},
						{
							Id:  "12",
							Lat: "3.0",
							Lon: "3.0",
						},
						{
							Id:  "13",
							Lat: "4.0",
							Lon: "4.0",
						},
					},
					Way: []*osmUtils.Way{
						{
							Id: "100",
							Nd: []*osmUtils.Nd{
								{
									Ref: "10",
								},
								{
									Ref: "11",
								},
							},
						},
						{
							Id: "101",
							Nd: []*osmUtils.Nd{
								{
									Ref: "11",
								},
								{
									Ref: "12",
								},
							},
						},
						{
							Id: "102",
							Nd: []*osmUtils.Nd{
								{
									Ref: "11",
								},
								{
									Ref: "12",
								},
								{
									Ref: "13",
								},
							},
						},
					},
				},
				anchors: map[float64]([]*osmUtils.Node){
					1: {
						{
							Id:  "11",
							Lat: "1.0",
							Lon: "1.0",
						},
					},
					3: {
						{
							Id:  "12",
							Lat: "2.0",
							Lon: "2.0",
						},
					},
				},
				nodeIdCounter: new(int),
				knoten: mapper.Spurplanknoten{
					WeichenAnf: []*mapper.Weichenanfang{
						{
							Name: mapper.Wert{
								Value: "Weiche 1",
							},
							KnotenTyp: mapper.KnotenTyp{
								Kilometrierung: mapper.Wert{
									Value: "1.000",
								},
							},
						},
					},
				},
				tracker: mapper.NewNotFoundElementTracker(),
			},
			want: want{
				isError: false,
				elementsNotFound: map[mapper.ElementType](int){
					mapper.Switches: 0,
				},
			},
		},
	}

	for _, tt := range tests {
		t.Run(tt.name, func(t *testing.T) {
			err := mapper.MapUnanchoredSwitches(
				tt.args.osm,
				tt.args.anchors,
				tt.args.nodeIdCounter,
				tt.args.knoten,
				tt.args.tracker,
			)
			if tt.want.isError {
				assert.Error(t, err)
			} else {
				assert.NoError(t, err)
			}
			assert.Equal(t, tt.want.elementsNotFound[mapper.Switches], tt.args.tracker.GetNotFoundElemetsCount(mapper.Switches))
		})
	}
}

func TestMapCrosses(t *testing.T) {
	type args struct {
		osm           *osmUtils.Osm
		anchors       map[float64][]*osmUtils.Node
		nodeIdCounter *int
		knoten        mapper.Spurplanknoten
		tracker       mapper.NotFoundElementTracker
	}
	type want struct {
		isError          bool
		elementsNotFound map[mapper.ElementType](int)
	}
	tests := []struct {
		name string
		args args
		want want
	}{
		{
			name: "find and map cross",
			args: args{
				osm: &osmUtils.Osm{
					Node: []*osmUtils.Node{
						{
							Id:  "10",
							Lat: "1.0",
							Lon: "1.0",
						},
						{
							Id:  "11",
							Lat: "2.0",
							Lon: "2.0",
						},
						{
							Id:  "12",
							Lat: "3.0",
							Lon: "3.0",
						},
						{
							Id:  "13",
							Lat: "4.0",
							Lon: "4.0",
						},
					},
					Way: []*osmUtils.Way{
						{
							Id: "100",
							Nd: []*osmUtils.Nd{
								{
									Ref: "10",
								},
								{
									Ref: "11",
								},
							},
						},
						{
							Id: "101",
							Nd: []*osmUtils.Nd{
								{
									Ref: "11",
								},
								{
									Ref: "12",
								},
							},
						},
						{
							Id: "102",
							Nd: []*osmUtils.Nd{
								{
									Ref: "12",
								},
								{
									Ref: "13",
								},
							},
						},
					},
				},
				anchors: map[float64]([]*osmUtils.Node){
					1: {
						{
							Id:  "11",
							Lat: "1.0",
							Lon: "1.0",
						},
					},
					3: {
						{
							Id:  "12",
							Lat: "2.0",
							Lon: "2.0",
						},
					},
				},
				nodeIdCounter: new(int),
				knoten: mapper.Spurplanknoten{
					KreuzungsweicheAnfangLinks: []*mapper.KreuzungsweicheAnfangLinks{
						{
							Name: mapper.Wert{
								Value: "Kreuzungsweiche 1",
							},
							KnotenTyp: mapper.KnotenTyp{
								Kilometrierung: mapper.Wert{
									Value: "1.000",
								},
							},
						},
					},
				},
				tracker: mapper.NewNotFoundElementTracker(),
			},
			want: want{
				isError: false,
				elementsNotFound: map[mapper.ElementType](int){
					mapper.Crosses: 0,
				},
			},
		},
		{
			name: "could not find cross",
			args: args{
				osm: &osmUtils.Osm{
					Node: []*osmUtils.Node{
						{
							Id:  "10",
							Lat: "1.0",
							Lon: "1.0",
						},
						{
							Id:  "11",
							Lat: "2.0",
							Lon: "2.0",
						},
						{
							Id:  "12",
							Lat: "3.0",
							Lon: "3.0",
						},
						{
							Id:  "13",
							Lat: "4.0",
							Lon: "4.0",
						},
					},
					Way: []*osmUtils.Way{
						{
							Id: "100",
							Nd: []*osmUtils.Nd{
								{
									Ref: "10",
								},
								{
									Ref: "11",
								},
							},
						},
						{
							Id: "101",
							Nd: []*osmUtils.Nd{
								{
									Ref: "11",
								},
								{
									Ref: "12",
								},
							},
						},
						{
							Id: "102",
							Nd: []*osmUtils.Nd{
								{
									Ref: "11",
								},
								{
									Ref: "12",
								},
								{
									Ref: "13",
								},
							},
						},
					},
				},
				anchors: map[float64]([]*osmUtils.Node){
					1: {
						{
							Id:  "11",
							Lat: "1.0",
							Lon: "1.0",
						},
					},
					3: {
						{
							Id:  "12",
							Lat: "2.0",
							Lon: "2.0",
						},
					},
				},
				nodeIdCounter: new(int),
				knoten: mapper.Spurplanknoten{
					KreuzungsweicheAnfangLinks: []*mapper.KreuzungsweicheAnfangLinks{
						{
							Name: mapper.Wert{
								Value: "Kreuzungsweiche 1",
							},
							KnotenTyp: mapper.KnotenTyp{
								Kilometrierung: mapper.Wert{
									Value: "1.000",
								},
							},
						},
					},
				},
				tracker: mapper.NewNotFoundElementTracker(),
			},
			want: want{
				isError: false,
				elementsNotFound: map[mapper.ElementType](int){
					mapper.Crosses: 0,
				},
			},
		},
	}

	for _, tt := range tests {
		t.Run(tt.name, func(t *testing.T) {
			err := mapper.MapCrosses(
				tt.args.osm,
				tt.args.anchors,
				tt.args.nodeIdCounter,
				tt.args.knoten,
				tt.args.tracker,
			)

			if tt.want.isError {
				assert.Error(t, err)
			} else {
				assert.NoError(t, err)
			}

			assert.Equal(t, tt.want.elementsNotFound[mapper.Crosses], tt.args.tracker.GetNotFoundElemetsCount(mapper.Crosses))
		})
	}
}
