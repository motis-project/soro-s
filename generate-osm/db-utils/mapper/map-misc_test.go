package mapper_test

import (
	"testing"
	"transform-osm/db-utils/mapper"
	osmUtils "transform-osm/osm-utils"

	"github.com/stretchr/testify/assert"
)

func TestMapSpeedLimits(t *testing.T) {
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
			name: "find and map speed limits",
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
					MaxSpeedF: []*mapper.MaxGeschwindigkeit{
						{
							Geschwindigkeit: mapper.Wert{
								Value: "100",
							},
							KnotenTyp: mapper.KnotenTyp{
								Kilometrierung: mapper.Wert{
									Value: "1,000",
								},
							},
						},
					},
					MaxSpeedS: []*mapper.MaxGeschwindigkeit{
						{
							Geschwindigkeit: mapper.Wert{
								Value: "100",
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
				elementsNotFound: map[mapper.ElementType](int){},
			},
		},
		{
			name: "could not find speed limits",
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
					MaxSpeedF: []*mapper.MaxGeschwindigkeit{
						{
							Geschwindigkeit: mapper.Wert{
								Value: "100",
							},
							KnotenTyp: mapper.KnotenTyp{
								Kilometrierung: mapper.Wert{
									Value: "2,000",
								},
							},
						},
					},
					MaxSpeedS: []*mapper.MaxGeschwindigkeit{
						{
							Geschwindigkeit: mapper.Wert{
								Value: "100",
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
				elementsNotFound: map[mapper.ElementType](int){
					mapper.SpeedLimits: 2,
				},
			},
		},
	}

	for _, tt := range tests {
		t.Run(tt.name, func(t *testing.T) {
			err := mapper.MapSpeedLimits(
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

			assert.Equal(t, tt.want.elementsNotFound[mapper.SpeedLimits], tt.args.tracker.GetNotFoundElemetsCount(mapper.SpeedLimits))
		})
	}
}

func TestMapSlopes(t *testing.T) {
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
			name: "find and map slopes",
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
					Neigung: []*mapper.Neigung{
						{
							Falling: mapper.Wert{
								Value: "1",
							},
							Rising: mapper.Wert{
								Value: "2",
							},
							KnotenTyp: mapper.KnotenTyp{
								Kilometrierung: mapper.Wert{
									Value: "1,000",
								},
							},
						},
					},
				},
				tracker: mapper.NewNotFoundElementTracker(),
			},
			want: want{
				isError:          false,
				elementsNotFound: map[mapper.ElementType](int){},
			},
		},
		{
			name: "could not find speed limits",
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
					Neigung: []*mapper.Neigung{
						{
							Falling: mapper.Wert{
								Value: "1",
							},
							Rising: mapper.Wert{
								Value: "2",
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
				elementsNotFound: map[mapper.ElementType](int){
					mapper.Slopes: 1,
				},
			},
		},
	}

	for _, tt := range tests {
		t.Run(tt.name, func(t *testing.T) {
			err := mapper.MapSlopes(
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

			assert.Equal(t, tt.want.elementsNotFound[mapper.Slopes], tt.args.tracker.GetNotFoundElemetsCount(mapper.Slopes))
		})
	}
}

func TestMapEoTDs(t *testing.T) {
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
			name: "find and map eotds",
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
					FstrZugschlussstelleF: []*mapper.SimpleElement{
						{
							KnotenTyp: mapper.KnotenTyp{
								Kilometrierung: mapper.Wert{
									Value: "1,000",
								},
							},
						},
					},
					SignalZugschlussstelleF: []*mapper.SimpleElement{
						{
							KnotenTyp: mapper.KnotenTyp{
								Kilometrierung: mapper.Wert{
									Value: "2,000",
								},
							},
						},
					},
					FstrZugschlussstelleS: []*mapper.SimpleElement{
						{
							KnotenTyp: mapper.KnotenTyp{
								Kilometrierung: mapper.Wert{
									Value: "1,000",
								},
							},
						},
					},
					SignalZugschlussstelleS: []*mapper.SimpleElement{
						{
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
				elementsNotFound: map[mapper.ElementType](int){},
			},
		},
		{
			name: "could not find eotds",
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
					FstrZugschlussstelleF: []*mapper.SimpleElement{
						{
							KnotenTyp: mapper.KnotenTyp{
								Kilometrierung: mapper.Wert{
									Value: "2,000",
								},
							},
						},
					},
					SignalZugschlussstelleF: []*mapper.SimpleElement{
						{
							KnotenTyp: mapper.KnotenTyp{
								Kilometrierung: mapper.Wert{
									Value: "2,000",
								},
							},
						},
					},
					FstrZugschlussstelleS: []*mapper.SimpleElement{
						{
							KnotenTyp: mapper.KnotenTyp{
								Kilometrierung: mapper.Wert{
									Value: "2,000",
								},
							},
						},
					},
					SignalZugschlussstelleS: []*mapper.SimpleElement{
						{
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
				elementsNotFound: map[mapper.ElementType](int){
					mapper.Eotds: 4,
				},
			},
		},
	}

	for _, tt := range tests {
		t.Run(tt.name, func(t *testing.T) {
			err := mapper.MapEoTDs(
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

			assert.Equal(t, tt.want.elementsNotFound[mapper.Eotds], tt.args.tracker.GetNotFoundElemetsCount(mapper.Eotds))
		})
	}
}

func TestMapSimpleElement(t *testing.T) {
	type args struct {
		osm           *osmUtils.Osm
		anchors       map[float64][]*osmUtils.Node
		nodeIdCounter *int
		elements      []*mapper.SimpleElement
		elementType   mapper.ElementType
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
			name: "map line_switch",
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
				elements: []*mapper.SimpleElement{
					{
						KnotenTyp: mapper.KnotenTyp{
							Kilometrierung: mapper.Wert{
								Value: "1,000",
							},
						},
					},
				},
				elementType: mapper.LineSwitch,
				tracker:     mapper.NewNotFoundElementTracker(),
			},
			want: want{
				isError:          false,
				elementsNotFound: map[mapper.ElementType](int){},
			},
		},
		{
			name: "could not find line_switch",
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
				elements: []*mapper.SimpleElement{
					{
						KnotenTyp: mapper.KnotenTyp{
							Kilometrierung: mapper.Wert{
								Value: "2,000",
							},
						},
					},
				},
				elementType: mapper.LineSwitch,
				tracker:     mapper.NewNotFoundElementTracker(),
			},
			want: want{
				isError: false,
				elementsNotFound: map[mapper.ElementType](int){
					mapper.LineSwitch: 1,
				},
			},
		},
		{
			name: "map km_jump",
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
				elements: []*mapper.SimpleElement{
					{
						KnotenTyp: mapper.KnotenTyp{
							Kilometrierung: mapper.Wert{
								Value: "1,000",
							},
						},
					},
				},
				elementType: mapper.KilometrageJump,
				tracker:     mapper.NewNotFoundElementTracker(),
			},
			want: want{
				isError:          false,
				elementsNotFound: map[mapper.ElementType](int){},
			},
		},
		{
			name: "could not find km_jump",
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
				elements: []*mapper.SimpleElement{
					{
						KnotenTyp: mapper.KnotenTyp{
							Kilometrierung: mapper.Wert{
								Value: "2,000",
							},
						},
					},
				},
				elementType: mapper.KilometrageJump,
				tracker:     mapper.NewNotFoundElementTracker(),
			},
			want: want{isError: false,
				elementsNotFound: map[mapper.ElementType](int){
					mapper.KilometrageJump: 1,
				}},
		},
		{
			name: "map border",
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
				elements: []*mapper.SimpleElement{
					{
						KnotenTyp: mapper.KnotenTyp{
							Kilometrierung: mapper.Wert{
								Value: "1,000",
							},
						},
					},
				},
				elementType: mapper.Border,
				tracker:     mapper.NewNotFoundElementTracker(),
			},
			want: want{
				isError:          false,
				elementsNotFound: map[mapper.ElementType](int){},
			},
		},
		{
			name: "could not find border",
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
				elements: []*mapper.SimpleElement{
					{
						KnotenTyp: mapper.KnotenTyp{
							Kilometrierung: mapper.Wert{
								Value: "2,000",
							},
						},
					},
				},
				elementType: mapper.Border,
				tracker:     mapper.NewNotFoundElementTracker(),
			},
			want: want{
				isError: false,
				elementsNotFound: map[mapper.ElementType](int){
					mapper.Border: 1,
				},
			},
		},
		{
			name: "map bumper",
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
				elements: []*mapper.SimpleElement{
					{
						KnotenTyp: mapper.KnotenTyp{
							Kilometrierung: mapper.Wert{
								Value: "1,000",
							},
						},
					},
				},
				elementType: mapper.Bumper,
				tracker:     mapper.NewNotFoundElementTracker(),
			},
			want: want{
				isError:          false,
				elementsNotFound: map[mapper.ElementType](int){},
			},
		},
		{
			name: "could not find bumper",
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
				elements: []*mapper.SimpleElement{
					{
						KnotenTyp: mapper.KnotenTyp{
							Kilometrierung: mapper.Wert{
								Value: "2,000",
							},
						},
					},
				},
				elementType: mapper.Bumper,
				tracker:     mapper.NewNotFoundElementTracker(),
			},
			want: want{
				isError: false,
				elementsNotFound: map[mapper.ElementType](int){
					mapper.Bumper: 1,
				},
			},
		},
	}

	for _, tt := range tests {
		t.Run(tt.name, func(t *testing.T) {
			err := mapper.MapSimpleElement(
				tt.args.osm,
				tt.args.anchors,
				tt.args.nodeIdCounter,
				tt.args.elements,
				tt.args.elementType,
				tt.args.tracker,
			)

			if tt.want.isError {
				assert.Error(t, err)
			} else {
				assert.NoError(t, err)
			}

			assert.Equal(t, tt.want.elementsNotFound[tt.args.elementType], tt.args.tracker.GetNotFoundElemetsCount(tt.args.elementType))
		})
	}
}

func TestMapNamedSimpleElement(t *testing.T) {
	type args struct {
		osm           *osmUtils.Osm
		anchors       map[float64][]*osmUtils.Node
		nodeIdCounter *int
		elements      []*mapper.NamedSimpleElement
		elementType   mapper.ElementType
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
			name: "map tunnel",
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
				elements: []*mapper.NamedSimpleElement{
					{
						Name: mapper.Wert{
							Value: "Tunnel 1",
						},
						KnotenTyp: mapper.KnotenTyp{
							Kilometrierung: mapper.Wert{
								Value: "1,000",
							},
						},
					},
				},
				elementType: mapper.Tunnel,
				tracker:     mapper.NewNotFoundElementTracker(),
			},
			want: want{
				isError:          false,
				elementsNotFound: map[mapper.ElementType](int){},
			},
		},
		{
			name: "could not find tunnel",
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
				elements: []*mapper.NamedSimpleElement{
					{
						Name: mapper.Wert{
							Value: "Tunnel 1",
						},
						KnotenTyp: mapper.KnotenTyp{
							Kilometrierung: mapper.Wert{
								Value: "2,000",
							},
						},
					},
				},
				elementType: mapper.Tunnel,
				tracker:     mapper.NewNotFoundElementTracker(),
			},
			want: want{
				isError: false,
				elementsNotFound: map[mapper.ElementType](int){
					mapper.Tunnel: 1,
				},
			},
		},
		{
			name: "map track end",
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
				elements: []*mapper.NamedSimpleElement{
					{
						Name: mapper.Wert{
							Value: "Track End 1",
						},
						KnotenTyp: mapper.KnotenTyp{
							Kilometrierung: mapper.Wert{
								Value: "1,000",
							},
						},
					},
				},
				elementType: mapper.TrackEnd,
				tracker:     mapper.NewNotFoundElementTracker(),
			},
			want: want{
				isError:          false,
				elementsNotFound: map[mapper.ElementType](int){},
			},
		},
		{
			name: "could not find track end",
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
				elements: []*mapper.NamedSimpleElement{
					{
						Name: mapper.Wert{
							Value: "Track End 1",
						},
						KnotenTyp: mapper.KnotenTyp{
							Kilometrierung: mapper.Wert{
								Value: "2,000",
							},
						},
					},
				},
				elementType: mapper.TrackEnd,
				tracker:     mapper.NewNotFoundElementTracker(),
			},
			want: want{
				isError: false,
				elementsNotFound: map[mapper.ElementType](int){
					mapper.TrackEnd: 1,
				},
			},
		},
	}

	for _, tt := range tests {
		t.Run(tt.name, func(t *testing.T) {
			err := mapper.MapNamedSimpleElement(
				tt.args.osm,
				tt.args.anchors,
				tt.args.nodeIdCounter,
				tt.args.elementType,
				tt.args.elements,
				tt.args.tracker,
			)

			if tt.want.isError {
				assert.Error(t, err)
			} else {
				assert.NoError(t, err)
			}

			assert.Equal(t, tt.want.elementsNotFound[tt.args.elementType], tt.args.tracker.GetNotFoundElemetsCount(tt.args.elementType))
		})
	}
}
