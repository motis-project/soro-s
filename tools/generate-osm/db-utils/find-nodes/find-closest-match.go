package findNodes

import (
	"sort"
	OSMUtil "transform-osm/osm-utils"

	"github.com/pkg/errors"
)

type NodePair struct {
	node1    *OSMUtil.Node
	node2    *OSMUtil.Node
	dist     float64
	remDist1 float64
	remDist2 float64
}

// findClosestMatch solves the following problem:
// In the calling 'findNewNodes' no two pairs are identical. Thus no clear Node could be determined.
// The method computes the Node-pair with the least distance to each other and choses the Node of the two that has the least deviation to the desired overall distance.
func findClosestMatch(
	osmData *OSMUtil.Osm,
	up1, up2, down1, down2 string,
	upDist1, upDist2, downDist1, downDist2 float64,
) (*OSMUtil.Node, error) {
	upNode1, upNode1Lat, upNode1Lon, err := ComputeNodeInformation(osmData, up1)
	if err != nil {
		return nil, errors.Wrap(err, "failed to retrieve information for upNode1: "+up1)
	}
	upNode2, upNode2Lat, upNode2Lon, err := ComputeNodeInformation(osmData, up2)
	if err != nil {
		return nil, errors.Wrap(err, "failed to retrieve information for upNode2: +"+up2)
	}
	downNode1, downNode1Lat, downNode1Lon, err := ComputeNodeInformation(osmData, down1)
	if err != nil {
		return nil, errors.Wrap(err, "failed to retrieve information for downNode1: "+down1)
	}
	downNode2, downNode2Lat, downNode2Lon, err := ComputeNodeInformation(osmData, down2)
	if err != nil {
		return nil, errors.Wrap(err, "failed to retrieve information for downNode2: "+down2)
	}

	distUp1Up2 := ComputeHaversineDistance(upNode1Lat, upNode2Lat, upNode1Lon, upNode2Lon)
	distUp1Down2 := ComputeHaversineDistance(upNode1Lat, downNode2Lat, upNode1Lon, downNode2Lon)
	distDown1Up2 := ComputeHaversineDistance(downNode1Lat, upNode2Lat, downNode1Lon, upNode2Lon)
	distDown1Down2 := ComputeHaversineDistance(downNode1Lat, downNode2Lat, downNode1Lon, downNode2Lon)

	var allPairs = []NodePair{
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
