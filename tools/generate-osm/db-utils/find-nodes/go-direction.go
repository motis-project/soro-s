package findNodes

import (
	"strconv"
	OSMUtil "transform-osm/osm-utils"

	"github.com/pkg/errors"
)

// goDirection follows the 'runningWay' over common Node-references for a distance of 'dist'.
// 'initialWayDirUp' and respectively wayDirUp determine, whether the next Node-reference is currentIndex-1 (-> true) or currentIndex+1 (-> false).
func goDirection(
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

	if wayDirUp && index != len(runningWay.Nd)-1 {
		oldNode, _ = OSMUtil.GetNodeById(osmData, runningWay.Nd[index+1].Ref)
	} else if !wayDirUp && index != 0 {
		oldNode, _ = OSMUtil.GetNodeById(osmData, runningWay.Nd[index-1].Ref)
	}

	var err error

	for totalDist < dist {
		if (wayDirUp && index == 0) || (!wayDirUp && index == len(runningWay.Nd)-1) {
			runningWay, index, wayDirUp, err = findNextWay(osmData, wayDirUp, index, runningNode, oldNode, runningWay)
			if errors.Cause(err) == endReachedError || errors.Cause(err) == junctionReachedError {
				return runningNode.Id, totalDist, err
			}
			if err != nil {
				return "", 0, errors.Wrap(err, "failed finding next way")
			}
		}

		nextNode, err = FindNextRunningNode(osmData, wayDirUp, index, runningWay)
		if err != nil {
			return "", 0, errors.Wrap(err, "failed getting next node")
		}

		phi1, _ := strconv.ParseFloat(runningNode.Lat, 64)
		phi2, _ := strconv.ParseFloat(nextNode.Lat, 64)
		lambda1, _ := strconv.ParseFloat(runningNode.Lon, 64)
		lambda2, _ := strconv.ParseFloat(nextNode.Lon, 64)

		totalDist += ComputeHaversineDistance(phi1, phi2, lambda1, lambda2)

		if totalDist == dist {
			return nextNode.Id, totalDist, nil
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
