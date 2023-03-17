package findNodes

import (
	OSMUtil "transform-osm/osm-utils"

	"github.com/pkg/errors"
)

var endReachedError = errors.New("reached end of track")
var junctionReachedError = errors.New("reached a junction, cannot proceed")

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
	firstWay, secondWay, err := getBothCorrectWays(osmData, runningNode.Id)
	if err != nil {
		return OSMUtil.Way{}, 0, false, errors.Wrap(err, "failed getting both ways")
	}

	firstWayMatches := GetWayNdRef(firstWay, FirstNdRef) == runningNode.Id
	secondWayMatches := GetWayNdRef(secondWay, FirstNdRef) == runningNode.Id

	firstMatchesSecond := GetWayNdRef(firstWay, FirstNdRef) == GetWayNdRef(secondWay, LastNdRef)
	secondMatchesFirst := GetWayNdRef(secondWay, FirstNdRef) == GetWayNdRef(firstWay, LastNdRef)

	// Ways can be "linked" in different ways. The usual ones are:
	// first begins and links with second end
	firstLinksWithSecond := firstWayMatches && firstMatchesSecond
	// or second begins and links with first end
	secondLinksWithFirst := secondWayMatches && secondMatchesFirst

	if wayDirUp && index == 0 {
		waysBeginMatch := GetWayNdRef(firstWay, FirstNdRef) == GetWayNdRef(secondWay, FirstNdRef)
		firstWayMatchesOldNode := GetWayNdRef(firstWay, SecondNdRef) == oldNode.Id
		secondWayMatchesOldNode := GetWayNdRef(secondWay, SecondNdRef) == oldNode.Id

		// More complicated ways, Ways can be "linked" include:
		// fisrt beginning links with second beginning and first is the way we are currently climbing up, as the second item is the old node
		connectionComingFromFirst := waysBeginMatch && firstWayMatches && firstWayMatchesOldNode
		// or both beginnings link up, however we come from second [second item is also known!]
		connectionComingFromSecond := waysBeginMatch && secondWayMatches && secondWayMatchesOldNode

		if !firstLinksWithSecond && !secondLinksWithFirst && !connectionComingFromFirst && !connectionComingFromSecond {
			return OSMUtil.Way{}, 0, false, errors.New("failed to find way up for node: " + runningNode.Id)
		}

		if firstLinksWithSecond || secondLinksWithFirst {
			wayDirUp = true

			if firstLinksWithSecond {
				index = len(secondWay.Nd) - 1
				runningWay = secondWay
			} else if secondLinksWithFirst {
				index = len(firstWay.Nd) - 1
				runningWay = firstWay
			}

			return runningWay, index, wayDirUp, nil
		}

		if connectionComingFromFirst || connectionComingFromSecond {
			wayDirUp = false
			index = 0
			if connectionComingFromFirst {
				runningWay = secondWay
			} else if connectionComingFromSecond {
				runningWay = firstWay
			}
		}

		return runningWay, index, wayDirUp, nil
	}

	firstLastAndSecondLastMatch := GetWayNdRef(firstWay, LastNdRef) == GetWayNdRef(secondWay, LastNdRef)

	firstWayLastMatches := GetWayNdRef(firstWay, LastNdRef) == runningNode.Id
	secondWayLastMatches := GetWayNdRef(secondWay, LastNdRef) == runningNode.Id

	firstWayMatchesOldNode := GetWayNdRef(firstWay, SecondLastNdRef) == oldNode.Id
	secondWayMatchesOldNode := GetWayNdRef(secondWay, SecondLastNdRef) == oldNode.Id

	// In the downward direction, ways can also be linked via the ends:
	// End-linkage and coming from first, as second-to-last is known as oldNode
	connectionEndComingFromFirst := firstLastAndSecondLastMatch && firstWayLastMatches && firstWayMatchesOldNode
	// or end-linkage and coming from second
	connectionEndComingFromSecond := firstLastAndSecondLastMatch && secondWayLastMatches && secondWayMatchesOldNode

	if !firstLinksWithSecond && !secondLinksWithFirst && !connectionEndComingFromFirst && !connectionEndComingFromSecond {
		return OSMUtil.Way{}, 0, false, errors.New("failed to find way down for node: " + runningNode.Id)
	}

	if firstLinksWithSecond || secondLinksWithFirst {
		wayDirUp = false
		index = 0

		if firstLinksWithSecond {
			runningWay = firstWay
		} else if secondLinksWithFirst {
			runningWay = secondWay
		}

		return runningWay, index, wayDirUp, nil
	}

	if connectionEndComingFromFirst || connectionEndComingFromSecond {
		wayDirUp = true

		if connectionEndComingFromFirst {
			index = len(secondWay.Nd) - 1
			runningWay = secondWay
		} else if connectionEndComingFromSecond {
			index = len(firstWay.Nd) - 1
			runningWay = firstWay
		}
	}

	return runningWay, index, wayDirUp, nil
}

func getBothCorrectWays(
	osmData *OSMUtil.Osm,
	runningNodeId string,
) (OSMUtil.Way, OSMUtil.Way, error) {
	nextWays, err := OSMUtil.FindWaysByNodeId(osmData, runningNodeId)
	if err != nil || len(nextWays) == 0 {
		return OSMUtil.Way{}, OSMUtil.Way{}, errors.Wrap(err, "no ways found")
	}
	if len(nextWays) == 1 {
		return OSMUtil.Way{}, OSMUtil.Way{}, endReachedError
	}
	if len(nextWays) > 2 {
		return OSMUtil.Way{}, OSMUtil.Way{}, junctionReachedError
	}

	return nextWays[0], nextWays[1], nil
}
