package osmUtils

import (
	"os"
	"os/exec"

	"github.com/pkg/errors"
)

func nodeNotFound(id string) error       { return errors.New("failed to find node: " + id) }
func tagOnNodeNotFound(key string) error { return errors.New("failed to find tag on node: " + key) }
func wayNotFound(id string) error        { return errors.New("failed to find way: " + id) }

// FindTagOnNode searches through the Tag-list of an OSM-node to look for a tag with key 'key'.
// If the tag is existant, the value of this tag is returned, otherwise an error.
func FindTagOnNode(node *Node, key string) (string, error) {
	for _, tag := range node.Tag {
		if tag.K == key {
			return tag.V, nil
		}
	}

	return "", tagOnNodeNotFound(key)
}

// GetNodeById searches the Node-list of the provided OSM-data for the node with ID 'id'.
// If a node with that ID exists, the node is returned, otherwise an error.
func GetNodeById(osm *Osm, id string) (*Node, error) {
	for _, node := range osm.Node {
		if node.Id == id {
			return node, nil
		}
	}
	return nil, nodeNotFound(id)
}

// GetNodeIndexInWay searches the Nd-reference-list of the provided for the node-ID 'id'.
// If the ID is present, the index in the Way-reference-list is returned, otherwise an error.
func GetNodeIndexInWay(way *Way, id string) (int, error) {
	for i, nd := range way.Nd {
		if nd.Ref == id {
			return i, nil
		}
	}
	return -1, nodeNotFound(id)
}

// FindWaysByNodeId returns all Ways in the provided OSM-data, that reference the node with ID 'id' in their Nd-list.
// If no way references this Node, an error will be returned.
func FindWaysByNodeId(osm *Osm, id string) ([]Way, error) {
	ways := []Way{}
	for _, way := range osm.Way {
		for _, node := range way.Nd {
			if node.Ref == id {
				ways = append(ways, *way)
				break
			}
		}
	}
	if len(ways) == 0 {
		return []Way{}, wayNotFound(id)
	}
	return ways, nil
}

// InsertNewNodeWithReferenceNode inserts a new Node into the provided OSM-data.
// First of course, the Node will be added to the Node-list.
// Secondly however, a Node-reference (via ID) will be inserted in all ways, that also contain a reference to the 'refenceNode'.
// The 'newNode'-reference is always insert after the 'referenceNode'-reference, except the 'referenceNode' is the last Node in that Way.
func InsertNewNodeWithReferenceNode(
	osm *Osm,
	newNode *Node,
	referenceNode *Node,
) {
	for _, way := range osm.Way {
		index, err := GetNodeIndexInWay(way, referenceNode.Id)
		if err == nil {
			if index == len(way.Nd)-1 {
				way.Nd = append(way.Nd[:index], []*Nd{{Ref: newNode.Id}, {Ref: referenceNode.Id}}...)
			} else {
				way.Nd = append(way.Nd[:index+1], append([]*Nd{{Ref: newNode.Id}}, way.Nd[(index+1):]...)...)
			}
		}
	}
	osm.Node = append(osm.Node, newNode)
}

// ExecuteOsmFilterCommand executes an 'osmium tags-filter' command externally.
// In 'args' all flags and parameters should be provided (like -i, -o, ...)
func ExecuteOsmFilterCommand(args []string) error {
	osmExecutable, _ := exec.LookPath("osmium")
	argsArray := []string{
		osmExecutable,
		"tags-filter",
	}
	argsArray = append(argsArray, args...)

	cmd := &exec.Cmd{
		Path:   osmExecutable,
		Args:   argsArray,
		Stdout: os.Stdout,
		Stderr: os.Stdout,
	}

	if err := cmd.Run(); err != nil {
		return errors.Wrap(err, "osmium command failed")
	}

	return nil
}
