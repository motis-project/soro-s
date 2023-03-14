package dbUtils

import (
	"math"
	"sort"
	"strconv"
	OSMUtil "transform-osm/osm-utils"

	"github.com/pkg/errors"
)

type nodePair struct {
	node1    *OSMUtil.Node
	node2    *OSMUtil.Node
	dist     float64
	remDist1 float64
	remDist2 float64
}

const EARTH_RADIUS_CONST = 6371.0

var endReached = errors.New("reached end of track")
var junctionReached = errors.New("reached a junction, cannot proceed")

// findNewNode takes two Nodes and a desired distance to both Nodes and computes an optimal Node in the provided 'osmData'.
// The returned Node satisfies the distance as best as possible.
func findNewNode(
	osmData *OSMUtil.Osm,
	node1 *OSMUtil.Node,
	node2 *OSMUtil.Node,
	dist1 float64,
	dist2 float64,
) (*OSMUtil.Node, error) {

	if dist1 == 0.0 {
		return node1, nil
	}

	up1, upDist1, down1, downDist1, err1 := findNodes(osmData, node1, dist1)
	up2, upDist2, down2, downDist2, err2 := findNodes(osmData, node2, dist2)

	if err1 != nil {
		return nil, errors.Wrap(err1, "insufficient anchor: "+node1.Id)
	}
	if err2 != nil {
		return nil, errors.Wrap(err2, "insufficient anchor: "+node2.Id)
	}

	if up1 == up2 || up1 == down2 {
		return OSMUtil.GetNodeById(osmData, up1)
	} else if down1 == up2 || down1 == down2 {
		return OSMUtil.GetNodeById(osmData, down1)
	} else {
		return getClosestMatch(osmData, up1, up2, down1, down2, upDist1, upDist2, downDist1, downDist2)
	}
}

// getClosestMatch solves the following problem:
// In the calling 'findNewNodes' no two pairs are identical. Thus no clear Node could be determined.
// The method computes the Node-pair with the least distance to each other and choses the Node of the two that has the least deviation to the desired overall distance.
func getClosestMatch(
	osmData *OSMUtil.Osm,
	up1, up2, down1, down2 string,
	upDist1, upDist2, downDist1, downDist2 float64,
) (*OSMUtil.Node, error) {
	upNode1, upNode1Lat, upNode1Lon, err := computeNodeInformation(osmData, up1)
	if err != nil {
		return nil, errors.Wrap(err, "failed to retrieve information for upNode1: "+up1)
	}
	upNode2, upNode2Lat, upNode2Lon, err := computeNodeInformation(osmData, up2)
	if err != nil {
		return nil, errors.Wrap(err, "failed to retrieve information for upNode2: +"+up2)
	}
	downNode1, downNode1Lat, downNode1Lon, err := computeNodeInformation(osmData, down1)
	if err != nil {
		return nil, errors.Wrap(err, "failed to retrieve information for downNode1: "+down1)
	}
	downNode2, downNode2Lat, downNode2Lon, err := computeNodeInformation(osmData, down2)
	if err != nil {
		return nil, errors.Wrap(err, "failed to retrieve information for downNode2: "+down2)
	}

	distUp1Up2 := distance(upNode1Lat, upNode2Lat, upNode1Lon, upNode2Lon)
	distUp1Down2 := distance(upNode1Lat, downNode2Lat, upNode1Lon, downNode2Lon)
	distDown1Up2 := distance(downNode1Lat, upNode2Lat, downNode1Lon, upNode2Lon)
	distDown1Down2 := distance(downNode1Lat, downNode2Lat, downNode1Lon, downNode2Lon)

	var allPairs = []nodePair{
		{upNode1, upNode2, distUp1Up2, upDist1, upDist2},
		{upNode1, downNode1, distUp1Down2, upDist1, downDist2},
		{downNode1, upNode2, distDown1Up2, downDist1, upDist2},
		{downNode1, downNode2, distDown1Down2, downDist1, downDist2}}

	sort.SliceStable(allPairs, func(i, j int) bool {
		dist1 := allPairs[i].dist
		dist2 := allPairs[j].dist
		return dist1 < dist2
	})

	if allPairs[0].remDist1 <= allPairs[0].remDist2 {
		return allPairs[0].node1, nil
	}
	return allPairs[0].node2, nil
}

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

	startWay, err := OSMUtil.FindWaysByNodeId(osmData, node.Id)
	if err != nil {
		return "", 0, "", 0, errors.Wrap(err, "error with finding start way")
	}

	if len(startWay) > 2 {
		return "", 0, "", 0, errors.New("too many ways for node: " + node.Id)
	}

	if len(startWay) == 1 {
		runningWay := startWay[0]
		index, err := OSMUtil.GetNodeIndexInWay(&runningWay, node.Id)
		if err != nil {
			return "", 0, "", 0, errors.Wrap(err, "error with starting way")
		}

		upId, upDist, err = goDir(osmData, runningWay, index, dist, true) // going "up" first
		if err != nil && err != endReached && err != junctionReached {
			return "", 0, "", 0, errors.Wrap(err, "failed going all the way up")
		}

		downId, downDist, err = goDir(osmData, runningWay, index, dist, false) // then going "down"
		if err != nil && err != endReached && err != junctionReached {
			return "", 0, "", 0, errors.Wrap(err, "failed going all the way down")
		}
		return upId, upDist, downId, downDist, nil
	}

	var runningWayUp, runningWayDown OSMUtil.Way

	if startWay[0].Nd[0].Ref == node.Id && startWay[1].Nd[len(startWay[1].Nd)-1].Ref == node.Id {
		runningWayUp = startWay[1]
		runningWayDown = startWay[0]
	} else if startWay[1].Nd[0].Ref == node.Id && startWay[0].Nd[len(startWay[0].Nd)-1].Ref == node.Id {
		runningWayUp = startWay[0]
		runningWayDown = startWay[1]
	} else {
		return "", 0, "", 0, errors.New("error with finding runningWays for Node: " + node.Id)
	}

	upId, upDist, err = goDir(osmData, runningWayUp, len(runningWayUp.Nd)-1, dist, true)
	if err != nil && err != endReached && err != junctionReached {
		return "", 0, "", 0, errors.Wrap(err, "failed going all the way up")
	}
	downId, downDist, err = goDir(osmData, runningWayDown, 0, dist, false)
	if err != nil && err != endReached && err != junctionReached {
		return "", 0, "", 0, errors.Wrap(err, "failed going all the way down")
	}

	return upId, upDist, downId, downDist, nil
}

// goDir follows the 'runningWay' over common Node-references for a distance of 'dist'.
// 'initialWayDirUp' and respectively wayDirUp determine, whether the next Node-reference is currentIndex-1 (-> true) or currentIndex+1 (-> false).
func goDir(
	osmData *OSMUtil.Osm,
	runningWay OSMUtil.Way,
	index int,
	dist float64,
	initialWayDirUp bool,
) (string, float64, error) {
	runningNode, _ := OSMUtil.GetNodeById(osmData, runningWay.Nd[index].Ref)
	var oldNode *OSMUtil.Node
	var nextNode *OSMUtil.Node
	totalDist := 0.0
	wayDirUp := initialWayDirUp

	var err error

	for totalDist < dist {
		if (wayDirUp && index == 0) || (!wayDirUp && index == len(runningWay.Nd)-1) {
			runningWay, index, wayDirUp, err = findNextWay(osmData, wayDirUp, index, runningNode, oldNode, runningWay)
			if err == endReached || err == junctionReached {
				return runningNode.Id, totalDist, err
			}
			if err != nil {
				return "", 0, errors.Wrap(err, "faild finding next way")
			}
		}

		nextNode, err = getNextNode(osmData, wayDirUp, index, runningWay)
		if err != nil {
			return "", 0, errors.Wrap(err, "failed getting next node")
		}

		phi1, _ := strconv.ParseFloat(runningNode.Lat, 64)
		phi2, _ := strconv.ParseFloat(nextNode.Lat, 64)
		lambda1, _ := strconv.ParseFloat(runningNode.Lon, 64)
		lambda2, _ := strconv.ParseFloat(nextNode.Lon, 64)

		totalDist += distance(phi1, phi2, lambda1, lambda2)

		if totalDist == dist {
			return runningWay.Nd[index].Ref, totalDist, nil
		}

		if wayDirUp {
			index--
		} else {
			index++
		}
		oldNode = runningNode
		runningNode = nextNode
	}
	return runningNode.Id, totalDist, nil
}

// getNextNode returns the next Node in the provided 'runningWay' depending on the direction provided.
func getNextNode(
	osmData *OSMUtil.Osm,
	wayDirUp bool,
	index int,
	runningWay OSMUtil.Way,
) (*OSMUtil.Node, error) {
	if wayDirUp {
		nextNode, err := OSMUtil.GetNodeById(osmData, runningWay.Nd[index-1].Ref)
		if err != nil {
			return nil, errors.Wrap(err, "failed to find node")
		}
		return nextNode, nil
	}

	nextNode, err := OSMUtil.GetNodeById(osmData, runningWay.Nd[index+1].Ref)
	if err != nil {
		return nil, errors.Wrap(err, "failed to find node")
	}
	return nextNode, nil
}

// findNextWay returns the next Way depending on the direction ('wayDirUp').
// It requires one of two combinations: ('index'=0, 'wayDirUp'=true) or ('index'=len(runningWay.Nd)-1, 'wayDirUp'=false)
// Due to bad osmData, the Node-reference-list of some ways are ordered the wrong way around.
// Thus, the direction can change, when going to the next way.
func findNextWay(
	osmData *OSMUtil.Osm,
	wayDirUp bool,
	index int,
	runningNode *OSMUtil.Node,
	oldNode *OSMUtil.Node,
	runningWay OSMUtil.Way,
) (OSMUtil.Way, int, bool, error) {
	nextWays, err := OSMUtil.FindWaysByNodeId(osmData, runningNode.Id)
	if err != nil || len(nextWays) == 0 {
		return OSMUtil.Way{}, 0, false, errors.Wrap(err, "no ways found")
	}
	if len(nextWays) == 1 {
		return OSMUtil.Way{}, 0, false, endReached
	}
	if len(nextWays) > 2 {
		return OSMUtil.Way{}, 0, false, junctionReached
	}

	// Ways can be "linked" in different ways. The usual ones are:
	// Index0 beginning links with index1 end
	wayConnection01 := nextWays[0].Nd[0].Ref == nextWays[1].Nd[len(nextWays[1].Nd)-1].Ref && nextWays[0].Nd[0].Ref == runningNode.Id
	// or Index1 beginning links with index0 end
	wayConnection10 := nextWays[1].Nd[0].Ref == nextWays[0].Nd[len(nextWays[0].Nd)-1].Ref && nextWays[1].Nd[0].Ref == runningNode.Id

	if wayDirUp && index == 0 {
		// More complicated ways, Ways can be "linked" include:
		// Index0 beginning links with Index1 beginning and Index0 is the way we are currently climbing up, as the second item is the old node
		wayConnection00comingFrom0 := nextWays[0].Nd[0].Ref == nextWays[1].Nd[0].Ref && nextWays[0].Nd[0].Ref == runningNode.Id && nextWays[0].Nd[1].Ref == oldNode.Id
		// or both beginnings link up, however we come from index1 [second item is also known!]
		wayConnection00comingFrom1 := nextWays[1].Nd[0].Ref == nextWays[0].Nd[0].Ref && nextWays[1].Nd[0].Ref == runningNode.Id && nextWays[1].Nd[1].Ref == oldNode.Id
		if wayConnection01 {
			runningWay = nextWays[1]
			index = len(nextWays[1].Nd) - 1
			wayDirUp = true
		} else if wayConnection10 {
			runningWay = nextWays[0]
			index = len(nextWays[0].Nd) - 1
			wayDirUp = true
		} else if wayConnection00comingFrom0 {
			runningWay = nextWays[1]
			index = 0
			wayDirUp = false
		} else if wayConnection00comingFrom1 {
			runningWay = nextWays[0]
			index = 0
			wayDirUp = false
		} else {
			return OSMUtil.Way{}, 0, false, errors.New("failed to find way up for node: " + runningNode.Id)
		}
	} else if !wayDirUp && index == len(runningWay.Nd)-1 {
		// In the downward direction, ways can also be linked via the ends:
		// End-linkage and coming from Index0, as second-to-last is known as oldNode
		wayConnectionEndEndComingFrom0 := nextWays[0].Nd[len(nextWays[0].Nd)-1].Ref == nextWays[1].Nd[len(nextWays[1].Nd)-1].Ref && nextWays[0].Nd[len(nextWays[0].Nd)-1].Ref == runningNode.Id && nextWays[0].Nd[len(nextWays[0].Nd)-2].Ref == oldNode.Id
		// or end-linkage and coming from Index1
		wayConnectionEndEndComingFrom1 := nextWays[1].Nd[len(nextWays[1].Nd)-1].Ref == nextWays[0].Nd[len(nextWays[0].Nd)-1].Ref && nextWays[1].Nd[len(nextWays[1].Nd)-1].Ref == runningNode.Id && nextWays[1].Nd[len(nextWays[1].Nd)-2].Ref == oldNode.Id
		if wayConnection01 {
			runningWay = nextWays[0]
			index = 0
			wayDirUp = false
		} else if wayConnection10 {
			runningWay = nextWays[1]
			index = 0
			wayDirUp = false
		} else if wayConnectionEndEndComingFrom0 {
			runningWay = nextWays[1]
			index = len(nextWays[1].Nd) - 1
			wayDirUp = true
		} else if wayConnectionEndEndComingFrom1 {
			runningWay = nextWays[0]
			index = len(nextWays[0].Nd) - 1
			wayDirUp = true
		} else {
			return OSMUtil.Way{}, 0, false, errors.New("failed to find way down for node: " + runningNode.Id)
		}
	}

	return runningWay, index, wayDirUp, nil
}

// computeNodeInformation returns the Node, its Latitude and Longitude provided with the desired Node-ID.
func computeNodeInformation(osmData *OSMUtil.Osm, nodeID string) (*OSMUtil.Node, float64, float64, error) {
	node, err := OSMUtil.GetNodeById(osmData, nodeID)
	if err != nil {
		return nil, 0, 0, errors.Wrap(err, "failed to find node")
	}
	nodeLat, _ := strconv.ParseFloat(node.Lat, 64)
	nodeLon, _ := strconv.ParseFloat(node.Lon, 64)

	return node, nodeLat, nodeLon, nil
}

// distance computes the Haversine distance between the two points (phi1, lambda1) and (phi2, lambda2) in (Latitude, Longitude).
func distance(phi1 float64, phi2 float64, lambda1 float64, lambda2 float64) float64 {
	phi1, phi2, lambda1, lambda2 = phi1*(math.Pi/180.0), phi2*(math.Pi/180.0), lambda1*(math.Pi/180.0), lambda2*(math.Pi/180.0)
	return 2.0 * EARTH_RADIUS_CONST * math.Asin(
		math.Sqrt(
			math.Pow(math.Sin((phi2-phi1)/2), 2)+
				math.Cos(phi1)*math.Cos(phi2)*math.Pow(math.Sin((lambda2-lambda1)/2), 2)))
}
