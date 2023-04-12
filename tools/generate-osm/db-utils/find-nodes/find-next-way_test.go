package findNodes_test

import (
	"testing"
	findNodes "transform-osm/db-utils/find-nodes"
	osmUtils "transform-osm/osm-utils"

	"github.com/stretchr/testify/assert"
)

func TestFindNextWay(t *testing.T) {
	type args struct {
		osm         *osmUtils.Osm
		wayDirUp    bool
		index       int
		runningNode *osmUtils.Node
		oldNode     *osmUtils.Node
		runningWay  osmUtils.Way
	}
	type want struct {
		nextWay  osmUtils.Way
		newIndex int
		wayDirUp bool
		errNil   bool
	}

	testNodes := []osmUtils.Node{
		{Id: "1"},
		{Id: "2"},
		{Id: "3"},
		{Id: "4"},
		{Id: "5"},
		{Id: "6"},
		{Id: "7"},
	}
	testWays := []osmUtils.Way{
		{
			Id: "101",
			Nd: []*osmUtils.Nd{
				{Ref: "1"},
				{Ref: "2"},
				{Ref: "3"},
			},
		},
		{
			Id: "102",
			Nd: []*osmUtils.Nd{
				{Ref: "3"},
				{Ref: "4"},
			},
		},
		{
			Id: "103",
			Nd: []*osmUtils.Nd{
				{Ref: "5"},
				{Ref: "4"},
			},
		},
		{
			Id: "104",
			Nd: []*osmUtils.Nd{
				{Ref: "5"},
				{Ref: "6"},
				{Ref: "7"},
			},
		},
		{
			Id: "105",
			Nd: []*osmUtils.Nd{
				{Ref: "6"},
				{Ref: "5"},
				{Ref: "7"},
			},
		},
		{
			Id: "106",
			Nd: []*osmUtils.Nd{
				{Ref: "3"},
				{Ref: "4"},
				{Ref: "2"},
			},
		},
	}
	testData := []osmUtils.Osm{
		{
			Node: []*osmUtils.Node{
				&testNodes[0],
				&testNodes[1],
				&testNodes[2],
				&testNodes[3],
				&testNodes[4],
				&testNodes[5],
				&testNodes[6],
			},
			Way: []*osmUtils.Way{
				&testWays[0],
				&testWays[1],
				&testWays[2],
				&testWays[3],
			},
		},
		{
			Node: []*osmUtils.Node{
				&testNodes[0],
				&testNodes[1],
				&testNodes[2],
				&testNodes[3],
				&testNodes[4],
				&testNodes[5],
				&testNodes[6],
			},
			Way: []*osmUtils.Way{
				&testWays[3],
				&testWays[2],
				&testWays[1],
				&testWays[0],
			},
		},
		{
			Node: []*osmUtils.Node{
				&testNodes[3],
				&testNodes[4],
				&testNodes[5],
				&testNodes[6],
			},
			Way: []*osmUtils.Way{
				&testWays[4],
				&testWays[2],
			},
		},
		{
			Node: []*osmUtils.Node{
				&testNodes[1],
				&testNodes[2],
				&testNodes[3],
			},
			Way: []*osmUtils.Way{
				&testWays[1],
				&testWays[5],
			},
		},
	}

	tests := []struct {
		name string
		args args
		want want
	}{
		{
			name: "can only find one way with node",
			args: args{
				osm:         &testData[0],
				wayDirUp:    true,
				index:       0,
				runningNode: &testNodes[0],
				oldNode:     &testNodes[1],
				runningWay:  testWays[0],
			},
			want: want{
				nextWay:  osmUtils.Way{},
				newIndex: 0,
				wayDirUp: false,
				errNil:   false,
			},
		},
		{
			name: "going up and not change direction 1",
			args: args{
				osm:         &testData[0],
				wayDirUp:    true,
				index:       0,
				runningNode: &testNodes[2],
				oldNode:     &testNodes[3],
				runningWay:  testWays[1],
			},
			want: want{
				nextWay:  testWays[0],
				newIndex: 2,
				wayDirUp: true,
				errNil:   true,
			},
		},
		{
			name: "going up and not change direction 2",
			args: args{
				osm:         &testData[1],
				wayDirUp:    true,
				index:       0,
				runningNode: &testNodes[2],
				oldNode:     &testNodes[3],
				runningWay:  testWays[1],
			},
			want: want{
				nextWay:  testWays[0],
				newIndex: 2,
				wayDirUp: true,
				errNil:   true,
			},
		},
		{
			name: "going down and not change direction 1",
			args: args{
				osm:         &testData[0],
				wayDirUp:    false,
				index:       2,
				runningNode: &testNodes[2],
				oldNode:     &testNodes[1],
				runningWay:  testWays[0],
			},
			want: want{
				nextWay:  testWays[1],
				newIndex: 0,
				wayDirUp: false,
				errNil:   true,
			},
		},
		{
			name: "going down and not change direction 2",
			args: args{
				osm:         &testData[1],
				wayDirUp:    false,
				index:       2,
				runningNode: &testNodes[2],
				oldNode:     &testNodes[1],
				runningWay:  testWays[0],
			},
			want: want{
				nextWay:  testWays[1],
				newIndex: 0,
				wayDirUp: false,
				errNil:   true,
			},
		},
		{
			name: "going up and change direction to down 1",
			args: args{
				osm:         &testData[0],
				wayDirUp:    true,
				index:       0,
				runningNode: &testNodes[4],
				oldNode:     &testNodes[5],
				runningWay:  testWays[3],
			},
			want: want{
				nextWay:  testWays[2],
				newIndex: 0,
				wayDirUp: false,
				errNil:   true,
			},
		},
		{
			name: "going up and change direction to down 2",
			args: args{
				osm:         &testData[1],
				wayDirUp:    true,
				index:       0,
				runningNode: &testNodes[4],
				oldNode:     &testNodes[5],
				runningWay:  testWays[3],
			},
			want: want{
				nextWay:  testWays[2],
				newIndex: 0,
				wayDirUp: false,
				errNil:   true,
			},
		},
		{
			name: "going down and change direction to up 1",
			args: args{
				osm:         &testData[0],
				wayDirUp:    false,
				index:       1,
				runningNode: &testNodes[3],
				oldNode:     &testNodes[2],
				runningWay:  testWays[1],
			},
			want: want{
				nextWay:  testWays[2],
				newIndex: 1,
				wayDirUp: true,
				errNil:   true,
			},
		},
		{
			name: "going down and change direction to up 2",
			args: args{
				osm:         &testData[1],
				wayDirUp:    false,
				index:       1,
				runningNode: &testNodes[3],
				oldNode:     &testNodes[2],
				runningWay:  testWays[1],
			},
			want: want{
				nextWay:  testWays[2],
				newIndex: 1,
				wayDirUp: true,
				errNil:   true,
			},
		},
		{
			name: "failing to link up ways going up",
			args: args{
				osm:         &testData[2],
				wayDirUp:    true,
				index:       0,
				runningNode: &testNodes[4],
				oldNode:     &testNodes[5],
				runningWay:  testWays[2],
			},
			want: want{
				nextWay:  osmUtils.Way{},
				newIndex: 0,
				wayDirUp: false,
				errNil:   false,
			},
		},
		{
			name: "failing to link up ways going down",
			args: args{
				osm:         &testData[3],
				wayDirUp:    false,
				index:       1,
				runningNode: &testNodes[3],
				oldNode:     &testNodes[2],
				runningWay:  testWays[1],
			},
			want: want{
				nextWay:  osmUtils.Way{},
				newIndex: 0,
				wayDirUp: false,
				errNil:   false,
			},
		},
	}

	for _, tt := range tests {
		t.Run(tt.name, func(t *testing.T) {
			nextWay, newIndex, wayDirUp, err := findNodes.FindNextWay(tt.args.osm, tt.args.wayDirUp, tt.args.index, tt.args.runningNode, tt.args.oldNode, tt.args.runningWay)
			if tt.want.errNil {
				assert.Nil(t, err)
			} else {
				assert.NotNil(t, err)
			}
			assert.Equal(t, tt.want.nextWay, nextWay)
			assert.Equal(t, tt.want.newIndex, newIndex)
			assert.Equal(t, tt.want.wayDirUp, wayDirUp)
		})
	}

}

func TestGetBothCorrectWays(t *testing.T) {
	type args struct {
		osm           *osmUtils.Osm
		runningNodeId string
	}
	type want struct {
		firstWay  osmUtils.Way
		secondWay osmUtils.Way
		errNil    bool
	}
	tests := []struct {
		name string
		args args
		want want
	}{
		{
			name: "get both correct ways",
			args: args{
				osm: &osmUtils.Osm{
					Way: []*osmUtils.Way{
						{
							Id: "1",
							Nd: []*osmUtils.Nd{
								{
									Ref: "1",
								},
								{
									Ref: "2",
								},
							},
						},
						{
							Id: "2",
							Nd: []*osmUtils.Nd{
								{
									Ref: "2",
								},
								{
									Ref: "3",
								},
							},
						},
						{
							Id: "3",
							Nd: []*osmUtils.Nd{
								{
									Ref: "3",
								},
								{
									Ref: "4",
								},
							},
						},
					},
				},
				runningNodeId: "2",
			},
			want: want{
				firstWay: osmUtils.Way{
					Id: "1",
					Nd: []*osmUtils.Nd{
						{
							Ref: "1",
						},
						{
							Ref: "2",
						},
					},
				},
				secondWay: osmUtils.Way{
					Id: "2",
					Nd: []*osmUtils.Nd{
						{
							Ref: "2",
						},
						{
							Ref: "3",
						},
					},
				},
				errNil: true,
			},
		},
		{
			name: "throws error if no way found",
			args: args{
				osm: &osmUtils.Osm{
					Way: []*osmUtils.Way{
						{
							Id: "1",
							Nd: []*osmUtils.Nd{
								{
									Ref: "1",
								},
								{
									Ref: "2",
								},
							},
						},
						{
							Id: "2",
							Nd: []*osmUtils.Nd{
								{
									Ref: "2",
								},
								{
									Ref: "3",
								},
							},
						},
						{
							Id: "3",
							Nd: []*osmUtils.Nd{
								{
									Ref: "3",
								},
								{
									Ref: "4",
								},
							},
						},
					},
				},
				runningNodeId: "5",
			},
			want: want{
				firstWay:  osmUtils.Way{},
				secondWay: osmUtils.Way{},
				errNil:    false,
			},
		},
		{
			name: "throws error if only one way found",
			args: args{
				osm: &osmUtils.Osm{
					Way: []*osmUtils.Way{
						{
							Id: "1",
							Nd: []*osmUtils.Nd{
								{
									Ref: "1",
								},
								{
									Ref: "2",
								},
							},
						},
						{
							Id: "2",
							Nd: []*osmUtils.Nd{
								{
									Ref: "2",
								},
								{
									Ref: "3",
								},
							},
						},
						{
							Id: "3",
							Nd: []*osmUtils.Nd{
								{
									Ref: "3",
								},
								{
									Ref: "4",
								},
							},
						},
					},
				},
				runningNodeId: "1",
			},
			want: want{
				firstWay:  osmUtils.Way{},
				secondWay: osmUtils.Way{},
				errNil:    false,
			},
		},
		{
			name: "throws error if more than two ways found",
			args: args{
				osm: &osmUtils.Osm{
					Way: []*osmUtils.Way{
						{
							Id: "1",
							Nd: []*osmUtils.Nd{
								{
									Ref: "1",
								},
								{
									Ref: "3",
								},
							},
						},
						{
							Id: "2",
							Nd: []*osmUtils.Nd{
								{
									Ref: "2",
								},
								{
									Ref: "3",
								},
							},
						},
						{
							Id: "3",
							Nd: []*osmUtils.Nd{
								{
									Ref: "3",
								},
								{
									Ref: "4",
								},
							},
						},
					},
				},
				runningNodeId: "3",
			},
			want: want{
				firstWay:  osmUtils.Way{},
				secondWay: osmUtils.Way{},
				errNil:    false,
			},
		},
	}

	for _, tt := range tests {
		t.Run(tt.name, func(t *testing.T) {
			firstWay, secondWay, err := findNodes.GetBothCorrectWays(tt.args.osm, tt.args.runningNodeId)

			if tt.want.errNil {
				assert.Nil(t, err)
			} else {
				assert.NotNil(t, err)
			}

			assert.Equal(t, tt.want.firstWay, firstWay)
			assert.Equal(t, tt.want.secondWay, secondWay)
		})
	}
}
