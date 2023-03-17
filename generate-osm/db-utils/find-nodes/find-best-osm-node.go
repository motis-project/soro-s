package findNodes

import (
	"math"
	"strings"
	OSMUtil "transform-osm/osm-utils"

	"github.com/pkg/errors"
)

var ErrNoSuitableAnchors = errors.New("failed to find suitable anchors")

// FindBestOSMNode determines a pair of anchors based on which a new Node is searched.
// Based on those, a new Node is then determined.
// len(anchors) >= 2 must hold!
func FindBestOSMNode(
	osmData *OSMUtil.Osm,
	anchors map[float64]([]*OSMUtil.Node),
	kilometrage float64,
) (*OSMUtil.Node, error) {
	sortedAnchors := SortAnchors(anchors, kilometrage)
	nearest, secondNearest := sortedAnchors[0], sortedAnchors[1]

	anchor1 := (anchors[nearest])[0]
	anchor2 := (anchors[secondNearest])[0]
	distance1 := math.Abs(nearest - kilometrage)
	distance2 := math.Abs(secondNearest - kilometrage)

	newNode, err := findNewNode(
		osmData,
		anchor1,
		anchor2,
		distance1,
		distance2,
	)
	if err == nil {
		return newNode, nil
	}

	newAnchorCounter := 2
	for err != nil && newAnchorCounter < len(sortedAnchors) {
		innerError := errors.Unwrap(err)
		errorParts := strings.Split(innerError.Error(), ": ")
		faultyNodeID := strings.ReplaceAll(errorParts[1], " ", "")

		if faultyNodeID == anchor1.Id {
			nearest = sortedAnchors[newAnchorCounter]
			anchor1 = (anchors[nearest])[0]
			distance1 = math.Abs(nearest - kilometrage)
			newAnchorCounter++
		} else {
			secondNearest = sortedAnchors[newAnchorCounter]
			anchor2 = (anchors[secondNearest])[0]
			distance2 = math.Abs(secondNearest - kilometrage)
			newAnchorCounter++
		}
		newNode, err = findNewNode(
			osmData,
			anchor1,
			anchor2,
			distance1,
			distance2,
		)
	}

	if err != nil && newAnchorCounter == len(sortedAnchors) {
		return nil, ErrNoSuitableAnchors
	}

	return newNode, nil
}
