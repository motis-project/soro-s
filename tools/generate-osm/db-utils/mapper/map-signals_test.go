package mapper_test

import (
	"testing"
	"transform-osm/db-utils/mapper"
	osmUtils "transform-osm/osm-utils"

	"github.com/stretchr/testify/assert"
)

func TestFindAndMapAnchorMainSignals(t *testing.T) {
	type args struct {
		knoten                 mapper.Spurplanknoten
		osm                    *osmUtils.Osm
		anchors                map[float64][]*osmUtils.Node
		notFoundSignalsFalling *[]*mapper.NamedSimpleElement
		notFoundSignalsRising  *[]*mapper.NamedSimpleElement
		signalList             map[string]osmUtils.Signal
		foundAnchorCount       *int
		nodeIdCounter          *int
	}
	type want struct {
		isError                      bool
		notFoundSignalsFallingLength int
		notFoundSignalsRisingLength  int
		foundAnchors                 map[float64][]*osmUtils.Node
	}
	tests := []struct {
		name string
		args args
		want want
	}{
		{
			name: "find both main signals",
			args: args{
				knoten: mapper.Spurplanknoten{
					HauptsigF: []*mapper.NamedSimpleElement{
						{
							Name: mapper.Wert{
								Value: "HauptsignalF",
							},
							KnotenTyp: mapper.KnotenTyp{
								Kilometrierung: mapper.Wert{
									Value: "1,000",
								},
							},
						},
					},
					HauptsigS: []*mapper.NamedSimpleElement{
						{
							Name: mapper.Wert{
								Value: "HauptsignalS",
							},
							KnotenTyp: mapper.KnotenTyp{
								Kilometrierung: mapper.Wert{
									Value: "2,000",
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
									V: "signal",
								},
								{
									K: "ref",
									V: "HauptsignalF",
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
									V: "signal",
								},
								{
									K: "ref",
									V: "HauptsignalS",
								},
							},
						},
					},
				},
				anchors:                map[float64][]*osmUtils.Node{},
				notFoundSignalsFalling: &[]*mapper.NamedSimpleElement{},
				notFoundSignalsRising:  &[]*mapper.NamedSimpleElement{},
				signalList:             map[string]osmUtils.Signal{},
				foundAnchorCount:       new(int),
				nodeIdCounter:          new(int),
			},
			want: want{
				isError:                      false,
				notFoundSignalsFallingLength: 0,
				notFoundSignalsRisingLength:  0,
				foundAnchors: map[float64][]*osmUtils.Node{
					1: {
						{
							Id:  "1",
							Lat: "1.0",
							Lon: "1.0",
							Tag: []*osmUtils.Tag{
								{
									K: "railway",
									V: "signal",
								},
								{
									K: "ref",
									V: "HauptsignalF",
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
									V: "signal",
								},
								{
									K: "ref",
									V: "HauptsignalR",
								},
							},
						},
					},
				},
			},
		},
		{
			name: "find one main signals",
			args: args{
				knoten: mapper.Spurplanknoten{
					HauptsigF: []*mapper.NamedSimpleElement{
						{
							Name: mapper.Wert{
								Value: "HauptsignalF",
							},
							KnotenTyp: mapper.KnotenTyp{
								Kilometrierung: mapper.Wert{
									Value: "1,000",
								},
							},
						},
					},
					HauptsigS: []*mapper.NamedSimpleElement{
						{
							Name: mapper.Wert{
								Value: "NotHauptsignalS",
							},
							KnotenTyp: mapper.KnotenTyp{
								Kilometrierung: mapper.Wert{
									Value: "2,000",
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
									V: "signal",
								},
								{
									K: "ref",
									V: "HauptsignalF",
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
									V: "signal",
								},
								{
									K: "ref",
									V: "HauptsignalS",
								},
							},
						},
					},
				},
				anchors:                map[float64][]*osmUtils.Node{},
				notFoundSignalsFalling: &[]*mapper.NamedSimpleElement{},
				notFoundSignalsRising:  &[]*mapper.NamedSimpleElement{},
				signalList:             map[string]osmUtils.Signal{},
				foundAnchorCount:       new(int),
				nodeIdCounter:          new(int),
			},
			want: want{
				isError:                      false,
				notFoundSignalsFallingLength: 0,
				notFoundSignalsRisingLength:  1,
				foundAnchors: map[float64][]*osmUtils.Node{
					1: {
						{
							Id:  "1",
							Lat: "1.0",
							Lon: "1.0",
							Tag: []*osmUtils.Tag{
								{
									K: "railway",
									V: "signal",
								},
								{
									K: "ref",
									V: "HauptsignalF",
								},
							},
						},
					},
				},
			},
		},
		{
			name: "find no main signals",
			args: args{
				knoten: mapper.Spurplanknoten{
					HauptsigF: []*mapper.NamedSimpleElement{
						{
							Name: mapper.Wert{
								Value: "NotHauptsignalF",
							},
							KnotenTyp: mapper.KnotenTyp{
								Kilometrierung: mapper.Wert{
									Value: "1,000",
								},
							},
						},
					},
					HauptsigS: []*mapper.NamedSimpleElement{
						{
							Name: mapper.Wert{
								Value: "NotHauptsignalS",
							},
							KnotenTyp: mapper.KnotenTyp{
								Kilometrierung: mapper.Wert{
									Value: "1,000",
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
									V: "signal",
								},
								{
									K: "ref",
									V: "HauptsignalF",
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
									V: "signal",
								},
								{
									K: "ref",
									V: "HauptsignalS",
								},
							},
						},
					},
				},
				anchors:                map[float64][]*osmUtils.Node{},
				notFoundSignalsFalling: &[]*mapper.NamedSimpleElement{},
				notFoundSignalsRising:  &[]*mapper.NamedSimpleElement{},
				signalList:             map[string]osmUtils.Signal{},
				foundAnchorCount:       new(int),
				nodeIdCounter:          new(int),
			},
			want: want{
				isError:                      false,
				notFoundSignalsFallingLength: 1,
				notFoundSignalsRisingLength:  1,
				foundAnchors:                 map[float64][]*osmUtils.Node{},
			},
		},
	}

	for _, tt := range tests {
		t.Run(tt.name, func(t *testing.T) {
			err := mapper.FindAndMapAnchorMainSignals(tt.args.knoten, tt.args.osm, tt.args.anchors, tt.args.notFoundSignalsFalling, tt.args.notFoundSignalsRising, tt.args.signalList, tt.args.foundAnchorCount, tt.args.nodeIdCounter)
			if tt.want.isError {
				assert.Error(t, err)
			} else {
				assert.NoError(t, err)
			}

			assert.Equal(t, tt.want.notFoundSignalsFallingLength, len(*tt.args.notFoundSignalsFalling))
			assert.Equal(t, tt.want.notFoundSignalsRisingLength, len(*tt.args.notFoundSignalsRising))
			assert.Equal(t, len(tt.want.foundAnchors), *tt.args.foundAnchorCount)
			for km, nodes := range tt.want.foundAnchors {
				foundAnchor := tt.args.anchors[km]
				assert.Equal(t, len(nodes), len(foundAnchor))
				for i, node := range nodes {
					assert.Equal(t, node.Id, foundAnchor[i].Id)
				}
			}

			if tt.want.notFoundSignalsFallingLength > 0 {
				assert.Equal(t, tt.args.knoten.HauptsigF[0].Name.Value, (*tt.args.notFoundSignalsFalling)[0].Name.Value)
			}
			if tt.want.notFoundSignalsRisingLength > 0 {
				assert.Equal(t, tt.args.knoten.HauptsigS[0].Name.Value, (*tt.args.notFoundSignalsRising)[0].Name.Value)
			}
		})
	}
}

func TestMapUnanchoredMainSignals(t *testing.T) {
	type args struct {
		osm           *osmUtils.Osm
		anchors       map[float64][]*osmUtils.Node
		signalList    map[string]osmUtils.Signal
		nodeIdCounter *int
		knoten        mapper.Spurplanknoten
		tracker       mapper.NotFoundElementTracker
	}
	type want struct {
		isError          bool
		signalList       map[string]osmUtils.Signal
		elementsNotFound map[mapper.ElementType](int)
	}
	tests := []struct {
		name string
		args args
		want want
	}{
		{
			name: "map unanchored main signals",
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
				signalList:    map[string]osmUtils.Signal{},
				nodeIdCounter: new(int),
				knoten: mapper.Spurplanknoten{
					HauptsigF: []*mapper.NamedSimpleElement{
						{
							Name: mapper.Wert{
								Value: "HauptsignalF",
							},
							KnotenTyp: mapper.KnotenTyp{
								Kilometrierung: mapper.Wert{
									Value: "1,000",
								},
							},
						},
					},
					HauptsigS: []*mapper.NamedSimpleElement{
						{
							Name: mapper.Wert{
								Value: "HauptsignalS",
							},
							KnotenTyp: mapper.KnotenTyp{
								Kilometrierung: mapper.Wert{
									Value: "2,000",
								},
							},
						},
					},
				},
				tracker: mapper.NewNotFoundElementTracker(),
			},
			want: want{
				isError: false,
				signalList: map[string]osmUtils.Signal{
					"1": {
						Name: "HauptsignalF",
						Lat:  "1.0",
						Lon:  "1.0",
					},
					"2": {
						Name: "HauptsignalS",
						Lat:  "1.0",
						Lon:  "1.0",
					},
				},
				elementsNotFound: map[mapper.ElementType](int){},
			},
		},
		{
			name: "map unanchored main signals with not found signals",
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
				signalList:    map[string]osmUtils.Signal{},
				nodeIdCounter: new(int),
				knoten: mapper.Spurplanknoten{
					HauptsigF: []*mapper.NamedSimpleElement{
						{
							Name: mapper.Wert{
								Value: "HauptsignalF",
							},
							KnotenTyp: mapper.KnotenTyp{
								Kilometrierung: mapper.Wert{
									Value: "2,000",
								},
							},
						},
					},
					HauptsigS: []*mapper.NamedSimpleElement{
						{
							Name: mapper.Wert{
								Value: "HauptsignalS",
							},
							KnotenTyp: mapper.KnotenTyp{
								Kilometrierung: mapper.Wert{
									Value: "2,000",
								},
							},
						},
					},
				},
				tracker: mapper.NewNotFoundElementTracker(),
			},
			want: want{
				isError:          false,
				signalList:       map[string]osmUtils.Signal{},
				elementsNotFound: map[mapper.ElementType](int){mapper.MainSignal: 2},
			},
		},
	}

	for _, tt := range tests {
		t.Run(tt.name, func(t *testing.T) {
			err := mapper.MapUnanchoredMainSignals(tt.args.osm, tt.args.anchors, tt.args.signalList, tt.args.nodeIdCounter, tt.args.knoten, tt.args.tracker)
			if tt.want.isError {
				assert.NotNil(t, err)
			} else {
				assert.Nil(t, err)
			}

			assert.Equal(t, len(tt.want.signalList), len(tt.args.signalList))
			for key, value := range tt.want.signalList {
				assert.Equal(t, value, tt.args.signalList[key])
			}

			assert.Equal(t, tt.want.elementsNotFound[mapper.MainSignal], tt.args.tracker.GetNotFoundElemetsCount(mapper.MainSignal))
		})
	}
}

func TestMapUnanchoredProtectionSignals(t *testing.T) {
	type args struct {
		osm           *osmUtils.Osm
		anchors       map[float64]([]*osmUtils.Node)
		signalList    map[string]osmUtils.Signal
		nodeIdCounter *int
		knoten        mapper.Spurplanknoten
		tracker       mapper.NotFoundElementTracker
	}
	type want struct {
		isError          bool
		signalList       map[string]osmUtils.Signal
		elementsNotFound map[mapper.ElementType](int)
	}
	tests := []struct {
		name string
		args args
		want want
	}{
		{
			name: "map unanchored protection signals",
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
				signalList:    map[string]osmUtils.Signal{},
				nodeIdCounter: new(int),
				knoten: mapper.Spurplanknoten{
					SchutzsigF: []*mapper.NamedSimpleElement{
						{
							Name: mapper.Wert{
								Value: "SchutzsigF",
							},
							KnotenTyp: mapper.KnotenTyp{
								Kilometrierung: mapper.Wert{
									Value: "1,000",
								},
							},
						},
					},
					SchutzsigS: []*mapper.NamedSimpleElement{
						{
							Name: mapper.Wert{
								Value: "SchutzsigS",
							},
							KnotenTyp: mapper.KnotenTyp{
								Kilometrierung: mapper.Wert{
									Value: "2,000",
								},
							},
						},
					},
				},
				tracker: mapper.NewNotFoundElementTracker(),
			},
			want: want{
				isError: false,
				signalList: map[string]osmUtils.Signal{
					"1": {
						Name: "SchutzsigF",
						Lat:  "1.0",
						Lon:  "1.0",
					},
					"2": {
						Name: "SchutzsigS",
						Lat:  "1.0",
						Lon:  "1.0",
					},
				},
				elementsNotFound: map[mapper.ElementType](int){},
			},
		},
		{
			name: "map unanchored protection signals with no anchors",
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
				signalList:    map[string]osmUtils.Signal{},
				nodeIdCounter: new(int),
				knoten: mapper.Spurplanknoten{
					SchutzsigF: []*mapper.NamedSimpleElement{
						{
							Name: mapper.Wert{
								Value: "SchutzsigF",
							},
							KnotenTyp: mapper.KnotenTyp{
								Kilometrierung: mapper.Wert{
									Value: "2,000",
								},
							},
						},
					},
					SchutzsigS: []*mapper.NamedSimpleElement{
						{
							Name: mapper.Wert{
								Value: "SchutzsigS",
							},
							KnotenTyp: mapper.KnotenTyp{
								Kilometrierung: mapper.Wert{
									Value: "2,000",
								},
							},
						},
					},
				},
				tracker: mapper.NewNotFoundElementTracker(),
			},
			want: want{
				isError:          false,
				signalList:       map[string]osmUtils.Signal{},
				elementsNotFound: map[mapper.ElementType](int){mapper.ProtectionSignal: 2},
			},
		},
	}

	for _, tt := range tests {
		t.Run(tt.name, func(t *testing.T) {
			err := mapper.MapUnanchoredProtectionSignals(tt.args.osm, tt.args.anchors, tt.args.signalList, tt.args.nodeIdCounter, tt.args.knoten, tt.args.tracker)
			if tt.want.isError {
				assert.NotNil(t, err)
			} else {
				assert.Nil(t, err)
			}

			assert.Equal(t, len(tt.want.signalList), len(tt.args.signalList))
			for key, value := range tt.want.signalList {
				assert.Equal(t, value, tt.args.signalList[key])
			}

			assert.Equal(t, tt.want.elementsNotFound[mapper.ProtectionSignal], tt.args.tracker.GetNotFoundElemetsCount(mapper.ProtectionSignal))
		})
	}
}

func TestMapUnanchoredApproachSignals(t *testing.T) {
	type args struct {
		osm           *osmUtils.Osm
		anchors       map[float64]([]*osmUtils.Node)
		signalList    map[string]osmUtils.Signal
		nodeIdCounter *int
		knoten        mapper.Spurplanknoten
		tracker       mapper.NotFoundElementTracker
	}
	type want struct {
		isError          bool
		signalList       map[string]osmUtils.Signal
		elementsNotFound map[mapper.ElementType](int)
	}
	tests := []struct {
		name string
		args args
		want want
	}{
		{
			name: "map unanchored protection signals",
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
				signalList:    map[string]osmUtils.Signal{},
				nodeIdCounter: new(int),
				knoten: mapper.Spurplanknoten{
					VorsigF: []*mapper.NamedSimpleElement{
						{
							Name: mapper.Wert{
								Value: "VorsigF",
							},
							KnotenTyp: mapper.KnotenTyp{
								Kilometrierung: mapper.Wert{
									Value: "1,000",
								},
							},
						},
					},
					VorsigS: []*mapper.NamedSimpleElement{
						{
							Name: mapper.Wert{
								Value: "VorsigS",
							},
							KnotenTyp: mapper.KnotenTyp{
								Kilometrierung: mapper.Wert{
									Value: "2,000",
								},
							},
						},
					},
				},
				tracker: mapper.NewNotFoundElementTracker(),
			},
			want: want{
				isError: false,
				signalList: map[string]osmUtils.Signal{
					"1": {
						Name: "VorsigF",
						Lat:  "1.0",
						Lon:  "1.0",
					},
					"2": {
						Name: "VorsigS",
						Lat:  "1.0",
						Lon:  "1.0",
					},
				},
				elementsNotFound: map[mapper.ElementType](int){},
			},
		},
		{
			name: "map unanchored approach signals with no anchors",
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
				signalList:    map[string]osmUtils.Signal{},
				nodeIdCounter: new(int),
				knoten: mapper.Spurplanknoten{
					VorsigF: []*mapper.NamedSimpleElement{
						{
							Name: mapper.Wert{
								Value: "VorsigF",
							},
							KnotenTyp: mapper.KnotenTyp{
								Kilometrierung: mapper.Wert{
									Value: "2,000",
								},
							},
						},
					},
					VorsigS: []*mapper.NamedSimpleElement{
						{
							Name: mapper.Wert{
								Value: "VorsigS",
							},
							KnotenTyp: mapper.KnotenTyp{
								Kilometrierung: mapper.Wert{
									Value: "2,000",
								},
							},
						},
					},
				},
				tracker: mapper.NewNotFoundElementTracker(),
			},
			want: want{
				isError:          false,
				signalList:       map[string]osmUtils.Signal{},
				elementsNotFound: map[mapper.ElementType](int){mapper.ApproachSignal: 2},
			},
		},
	}

	for _, tt := range tests {
		t.Run(tt.name, func(t *testing.T) {
			err := mapper.MapUnanchoredApproachSignals(tt.args.osm, tt.args.anchors, tt.args.signalList, tt.args.nodeIdCounter, tt.args.knoten, tt.args.tracker)
			if tt.want.isError {
				assert.NotNil(t, err)
			} else {
				assert.Nil(t, err)
			}

			assert.Equal(t, len(tt.want.signalList), len(tt.args.signalList))
			for key, value := range tt.want.signalList {
				assert.Equal(t, value, tt.args.signalList[key])
			}

			assert.Equal(t, tt.want.elementsNotFound[mapper.ApproachSignal], tt.args.tracker.GetNotFoundElemetsCount(mapper.ApproachSignal))
		})
	}
}
