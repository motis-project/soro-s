package osmUtils_test

import (
	"encoding/xml"
	"testing"
	osmUtils "transform-osm/osm-utils"

	"github.com/stretchr/testify/assert"
)

func TestGetRefIds(t *testing.T) {
	type args struct {
		trackRefOsm osmUtils.Osm
	}
	tests := []struct {
		name string
		args args
		want []string
	}{
		{
			name: "test the generation of ref ids based on the osm data",
			args: args{
				trackRefOsm: osmUtils.Osm{
					XMLName:   xml.Name{},
					Version:   "0.6",
					Generator: "osmium/1.13.0",
					Way:       nil,
					Node:      nil,
					Relation: []*osmUtils.Relation{
						{
							XMLName: xml.Name{},
							Tag: []*osmUtils.Tag{
								{
									XMLName: xml.Name{},
									K:       "ref",
									V:       "1234",
								},
							},
						},
						{
							XMLName: xml.Name{},
							Tag: []*osmUtils.Tag{
								{
									XMLName: xml.Name{},
									K:       "ref",
									V:       "12345",
								},
							},
						},
						{
							XMLName: xml.Name{},
							Tag: []*osmUtils.Tag{
								{
									XMLName: xml.Name{},
									K:       "ref",
									V:       "123",
								},
							},
						},
					},
				},
			},
			want: []string{"1234"},
		},
	}

	for _, tt := range tests {
		t.Run(tt.name, func(t *testing.T) {
			got := osmUtils.GetRefIds(tt.args.trackRefOsm)
			assert.Equal(t, tt.want, got, "Expected %v, got %v", tt.want, got)
		})
	}
}
