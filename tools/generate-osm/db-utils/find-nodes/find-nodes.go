package findNodes

import (
	OSMUtil "transform-osm/osm-utils"

	"github.com/pkg/errors"
)

// findNodes finds the two Nodes in either direction, that are about 'dist' away from the provided 'node'.
// To find these Nodes, we need to follow the Ways in the provided 'osmData'.
// The starting Node must satisfy the condition that it appears in no more than 2 ways.
// If it appears in exactly 2 Ways, the position in both ways must fit, i.e. it is the i-th element in one and the i-th-last element in the other.
func findNodes(
	osmData *OSMUtil.Osm,
	node *OSMUtil.Node,
	dist float64,
) (string, float64, string, float64, error) {
	var upId, downId string
	var upDist, downDist float64

	waysForNode, err := OSMUtil.FindWaysByNodeId(osmData, node.Id)
	if err != nil {
		return "", 0, "", 0, errors.Wrap(err, "error with finding ways for node")
	}

	if len(waysForNode) > 2 {
		return "", 0, "", 0, errors.New("too many ways for node: " + node.Id)
	}

	if len(waysForNode) == 1 {
		runningWay := waysForNode[0]
		index, _ := OSMUtil.GetNodeIndexInWay(&runningWay, node.Id)

		upId, upDist, err = goDirection(osmData, runningWay, index, dist, true) // going "up" first
		if err != nil && errors.Cause(err) != endReachedError && errors.Cause(err) != junctionReachedError {
			return "", 0, "", 0, errors.Wrap(err, "failed going all the way up")
		}

		downId, downDist, err = goDirection(osmData, runningWay, index, dist, false) // then going "down"
		if err != nil && errors.Cause(err) != endReachedError && errors.Cause(err) != junctionReachedError {
			return "", 0, "", 0, errors.Wrap(err, "failed going all the way down")
		}
		return upId, upDist, downId, downDist, nil
	}

	var runningWayUp, runningWayDown OSMUtil.Way
	firstWay := waysForNode[0]
	secondWay := waysForNode[1]

	firstWayStartsWithNode := GetWayNdRef(firstWay, FirstNdRef) == node.Id
	firstWayEndsWithNode := GetWayNdRef(firstWay, LastNdRef) == node.Id
	secondWayStartsWithNode := GetWayNdRef(secondWay, FirstNdRef) == node.Id
	secondWayEndsWithNode := GetWayNdRef(secondWay, LastNdRef) == node.Id

	if firstWayStartsWithNode && secondWayEndsWithNode {
		runningWayUp = secondWay
		runningWayDown = firstWay
	} else if secondWayStartsWithNode && firstWayEndsWithNode {
		runningWayUp = firstWay
		runningWayDown = secondWay
	} else {
		return "", 0, "", 0, errors.New("error with finding runningWays for Node: " + node.Id)
	}

	upId, upDist, err = goDirection(osmData, runningWayUp, len(runningWayUp.Nd)-1, dist, true)
	if err != nil && errors.Cause(err) != endReachedError && errors.Cause(err) != junctionReachedError {
		return "", 0, "", 0, errors.Wrap(err, "failed going all the way up")
	}
	downId, downDist, err = goDirection(osmData, runningWayDown, 0, dist, false)
	if err != nil && errors.Cause(err) != endReachedError && errors.Cause(err) != junctionReachedError {
		return "", 0, "", 0, errors.Wrap(err, "failed going all the way down")
	}

	return upId, upDist, downId, downDist, nil
}
