package osmUtils_test

import (
	"testing"
	osmUtils "transform-osm/osm-utils"

	"github.com/stretchr/testify/assert"
)

func TestFindTagOnNode(t *testing.T) {
	type args struct {
		node *osmUtils.Node
		key  string
	}
	type want struct {
		value string
		err   error
	}
	tt := []struct {
		name string
		args args
		want want
	}{
		{
			name: "returns value of tag on node",
			args: args{
				node: &osmUtils.Node{
					Tag: []*osmUtils.Tag{
						{
							K: "name",
							V: "test",
						},
						{
							K: "someOtherKey",
							V: "notTheValueWeWant",
						},
					},
				},
				key: "name",
			},
			want: want{
				value: "test",
				err:   nil,
			},
		},
		{
			name: "throws error if tag is not found",
			args: args{
				node: &osmUtils.Node{
					Tag: []*osmUtils.Tag{
						{
							K: "name",
							V: "test",
						},
					},
				},
				key: "someOtherKey",
			},
			want: want{
				value: "",
				err:   osmUtils.TagOnNodeNotFound("someOtherKey"),
			},
		},
	}

	for _, tc := range tt {
		t.Run(tc.name, func(t *testing.T) {
			val, err := osmUtils.FindTagOnNode(tc.args.node, tc.args.key)
			assert.Equal(t, tc.want.value, val, "Expected %v, got %v", tc.want.value, val)
			if err != nil {
				assert.Equal(t, tc.want.err.Error(), err.Error(), "Expected %v, got %v", tc.want.err, err)
			} else {
				assert.Equal(t, tc.want.err, err, "Expected %v, got %v", tc.want.err, err)
			}
		})
	}
}

func TestGetNodeById(t *testing.T) {
	type args struct {
		osm *osmUtils.Osm
		id  string
	}
	type want struct {
		node *osmUtils.Node
		err  error
	}
	tt := []struct {
		name string
		args args
		want want
	}{
		{
			name: "returns node if found",
			args: args{
				osm: &osmUtils.Osm{
					Node: []*osmUtils.Node{
						{
							Id: "123",
						},
						{
							Id: "456",
						},
					},
				},
				id: "123",
			},
			want: want{
				node: &osmUtils.Node{
					Id: "123",
				},
				err: nil,
			},
		},
		{
			name: "throws error if node is not found",
			args: args{
				osm: &osmUtils.Osm{
					Node: []*osmUtils.Node{
						{
							Id: "456",
						},
					},
				},
				id: "123",
			},
			want: want{
				node: nil,
				err:  osmUtils.NodeNotFound("123"),
			},
		},
	}

	for _, tc := range tt {
		t.Run(tc.name, func(t *testing.T) {
			node, err := osmUtils.GetNodeById(tc.args.osm, tc.args.id)
			assert.Equal(t, tc.want.node, node, "Expected %v, got %v", tc.want.node, node)
			if err != nil {
				assert.Equal(t, tc.want.err.Error(), err.Error(), "Expected %v, got %v", tc.want.err, err)
			} else {
				assert.Equal(t, tc.want.err, err, "Expected %v, got %v", tc.want.err, err)
			}
		})
	}
}

func TestGetNodeIndexInWay(t *testing.T) {
	type args struct {
		way *osmUtils.Way
		id  string
	}
	type want struct {
		index int
		err   error
	}
	tt := []struct {
		name string
		args args
		want want
	}{
		{
			name: "returns index of node in way",
			args: args{
				way: &osmUtils.Way{
					Nd: []*osmUtils.Nd{
						{
							Ref: "123",
						},
						{
							Ref: "456",
						},
					},
				},
				id: "123",
			},
			want: want{
				index: 0,
				err:   nil,
			},
		},
		{
			name: "throws error if node is not found",
			args: args{
				way: &osmUtils.Way{
					Nd: []*osmUtils.Nd{
						{
							Ref: "456",
						},
					},
				},
				id: "123",
			},
			want: want{
				index: -1,
				err:   osmUtils.NodeNotFound("123"),
			},
		},
	}

	for _, tc := range tt {
		t.Run(tc.name, func(t *testing.T) {
			index, err := osmUtils.GetNodeIndexInWay(tc.args.way, tc.args.id)
			assert.Equal(t, tc.want.index, index, "Expected %v, got %v", tc.want.index, index)
			if err != nil {
				assert.Equal(t, tc.want.err.Error(), err.Error(), "Expected %v, got %v", tc.want.err, err)
			} else {
				assert.Equal(t, tc.want.err, err, "Expected %v, got %v", tc.want.err, err)
			}
		})
	}
}

func TestFindWaysByNodeId(t *testing.T) {
	type args struct {
		osm *osmUtils.Osm
		id  string
	}
	type want struct {
		ways []osmUtils.Way
		err  error
	}
	tt := []struct {
		name string
		args args
		want want
	}{
		{
			name: "returns ways that contain node",
			args: args{
				osm: &osmUtils.Osm{
					Way: []*osmUtils.Way{
						{
							Nd: []*osmUtils.Nd{
								{
									Ref: "123",
								},
								{
									Ref: "456",
								},
							},
						},
						{
							Nd: []*osmUtils.Nd{
								{
									Ref: "789",
								},
								{
									Ref: "123",
								},
							},
						},
					},
				},
				id: "123",
			},
			want: want{
				ways: []osmUtils.Way{
					{
						Nd: []*osmUtils.Nd{
							{
								Ref: "123",
							},
							{
								Ref: "456",
							},
						},
					},
					{
						Nd: []*osmUtils.Nd{
							{
								Ref: "789",
							},
							{
								Ref: "123",
							},
						},
					},
				},
				err: nil,
			},
		},
		{
			name: "returns empty list if node is not found",
			args: args{
				osm: &osmUtils.Osm{
					Way: []*osmUtils.Way{
						{
							Nd: []*osmUtils.Nd{
								{
									Ref: "789",
								},
								{
									Ref: "456",
								},
							},
						},
					},
				},
				id: "123",
			},
			want: want{
				ways: []osmUtils.Way{},
				err:  osmUtils.WayNotFound("123"),
			},
		},
	}

	for _, tc := range tt {
		t.Run(tc.name, func(t *testing.T) {
			ways, err := osmUtils.FindWaysByNodeId(tc.args.osm, tc.args.id)
			assert.Equal(t, tc.want.ways, ways, "Expected %v, got %v", tc.want.ways, ways)
			if err != nil {
				assert.Equal(t, tc.want.err.Error(), err.Error(), "Expected %v, got %v", tc.want.err, err)
			} else {
				assert.Equal(t, tc.want.err, err, "Expected %v, got %v", tc.want.err, err)
			}
		})
	}
}

func TestInsertNewNodeWithReferenceNode(t *testing.T) {
	type args struct {
		osm           *osmUtils.Osm
		newNode       *osmUtils.Node
		referenceNode *osmUtils.Node
	}
	tt := []struct {
		name string
		args args
		want int
	}{
		{
			name: "inserts new node between two nodes",
			args: args{
				osm: &osmUtils.Osm{
					Node: []*osmUtils.Node{
						{
							Id: "123",
						},
						{
							Id: "456",
						},
						{
							Id: "789",
						},
					},
					Way: []*osmUtils.Way{
						{
							Id: "1",
							Nd: []*osmUtils.Nd{
								{
									Ref: "123",
								},
								{
									Ref: "456",
								},
								{
									Ref: "789",
								},
							},
						},
					},
				},
				newNode: &osmUtils.Node{
					Id: "555",
				},
				referenceNode: &osmUtils.Node{
					Id: "456",
				},
			},
			want: 2,
		},
	}

	for _, tc := range tt {
		t.Run(tc.name, func(t *testing.T) {
			osmUtils.InsertNewNodeWithReferenceNode(tc.args.osm, tc.args.newNode, tc.args.referenceNode)
			assert.Equal(t, 4, len(tc.args.osm.Node), "Expected %v, got %v", 4, len(tc.args.osm.Node))
			assert.Equal(t, 4, len(tc.args.osm.Way[0].Nd), "Expected %v, got %v", 4, len(tc.args.osm.Way[0].Nd))
			assert.Equal(t, tc.args.osm.Way[0].Nd[tc.want].Ref, tc.args.newNode.Id, "Expected %v, got %v", tc.args.newNode.Id, tc.args.osm.Way[0].Nd[tc.want].Ref)
		})
	}
}
