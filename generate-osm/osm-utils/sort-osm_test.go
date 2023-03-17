package osmUtils_test

import (
	"testing"
	osmUtils "transform-osm/osm-utils"

	"github.com/stretchr/testify/assert"
)

func TestSortAndRemoveDuplicates(t *testing.T) {
	type args struct {
		osm *osmUtils.Osm
	}
	tests := []struct {
		name string
		args args
		want *osmUtils.Osm
	}{
		{
			name: "test sort and remove duplicates",
			args: args{
				osm: &osmUtils.Osm{
					Relation: []*osmUtils.Relation{
						{
							Id: "2",
						},
						{
							Id: "1",
						},
						{
							Id: "1",
						},
					},
					Way: []*osmUtils.Way{
						{
							Id: "2",
						},
						{
							Id: "1",
						},
						{
							Id: "1",
						},
					},
					Node: []*osmUtils.Node{
						{
							Id: "2",
						},
						{
							Id: "1",
						},
						{
							Id: "1",
						},
					},
				},
			},
			want: &osmUtils.Osm{
				Relation: []*osmUtils.Relation{
					{
						Id: "1",
					},
					{
						Id: "2",
					},
				},
				Node: []*osmUtils.Node{
					{
						Id: "1",
					},
					{
						Id: "2",
					},
				},
				Way: []*osmUtils.Way{
					{
						Id: "1",
					},
					{
						Id: "2",
					},
				},
			},
		},
	}
	for _, tt := range tests {
		t.Run(tt.name, func(t *testing.T) {
			osmUtils.SortAndRemoveDuplicatesOsm(tt.args.osm)
			for i := 0; i < len(tt.args.osm.Relation); i++ {
				assert.Equal(t, tt.want.Relation[i].Id, tt.args.osm.Relation[i].Id, "Expected %v, got %v", tt.want.Relation[i].Id, tt.args.osm.Relation[i].Id)
			}

			for i := 0; i < len(tt.args.osm.Way); i++ {
				assert.Equal(t, tt.want.Way[i].Id, tt.args.osm.Way[i].Id, "Expected %v, got %v", tt.want.Way[i].Id, tt.args.osm.Way[i].Id)
			}

			for i := 0; i < len(tt.args.osm.Node); i++ {
				assert.Equal(t, tt.want.Node[i].Id, tt.args.osm.Node[i].Id, "Expected %v, got %v", tt.want.Node[i].Id, tt.args.osm.Node[i].Id)
			}
		})
	}
}
