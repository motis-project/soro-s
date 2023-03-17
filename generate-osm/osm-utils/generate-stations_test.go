package osmUtils_test

import (
	"testing"
	osmUtils "transform-osm/osm-utils"

	"github.com/stretchr/testify/assert"
)

func TestGenerateSearchFile(t *testing.T) {
	type args struct {
		osm osmUtils.Osm
	}
	type want struct {
		stations map[string]osmUtils.Station
		osm      osmUtils.Osm
		err      error
	}
	tests := []struct {
		name string
		args args
		want want
	}{
		{
			name: "test the generation of the search file and adding the stations to the osm data",
			args: args{
				osmUtils.Osm{
					Node: []*osmUtils.Node{
						{
							Id:  "1",
							Lat: "1",
							Lon: "2",
							Tag: []*osmUtils.Tag{
								{
									K: "name",
									V: "testStation",
								},
								{
									K: "railway",
									V: "station",
								},
							},
						},
						{
							Id:  "2",
							Lat: "3",
							Lon: "4",
							Tag: []*osmUtils.Tag{
								{
									K: "name",
									V: "testHalt",
								},
								{
									K: "railway",
									V: "halt",
								},
							},
						},
					},
				},
			},
			want: want{
				stations: map[string]osmUtils.Station{
					"1": {
						Name: "testStation",
						Lat:  "1",
						Lon:  "2",
					},
					"2": {
						Name: "testHalt",
						Lat:  "3",
						Lon:  "4",
					},
				},
				osm: osmUtils.Osm{
					Node: []*osmUtils.Node{
						{
							Id:  "1",
							Lat: "1",
							Lon: "2",
							Tag: []*osmUtils.Tag{
								{
									K: "name",
									V: "testStation",
								},
								{
									K: "railway",
									V: "station",
								},
								{
									K: "type",
									V: "station",
								},
							},
						},
						{
							Id:  "2",
							Lat: "3",
							Lon: "4",
							Tag: []*osmUtils.Tag{
								{
									K: "name",
									V: "testHalt",
								},
								{
									K: "railway",
									V: "halt",
								},
								{
									K: "type",
									V: "station",
								},
							},
						},
					},
				},
				err: nil,
			},
		},
	}

	for _, test := range tests {
		t.Run(test.name, func(t *testing.T) {
			stations, stationHaltsOsm := osmUtils.GenerateOsmAndSearchFile(test.args.osm)

			assert.Equal(t, test.want.stations, stations, "Expected %v, got %v", test.want.stations, stations)
			assert.EqualValues(t, test.want.osm, stationHaltsOsm, "Expected %v, got %v", test.want.osm, stationHaltsOsm)
		})
	}
}
