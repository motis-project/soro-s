package dbUtils

import (
	"math"
	"sort"
	"strconv"
	"strings"

	OSMUtil "transform-osm/osm-utils"

	"github.com/pkg/errors"
)

// findBestOSMNode determines a pair of anchors based on which a new Node is searched.
// Based on those, a new Node is then determined.
func findBestOSMNode(
	osmData *OSMUtil.Osm,
	anchors *map[float64]([]*OSMUtil.Node),
	kilometrage float64,
) (*OSMUtil.Node, error) {
	sortedAnchors := sortAnchors(anchors, kilometrage)

	nearest, secondNearest := sortedAnchors[0], sortedAnchors[1]

	anchor1 := ((*anchors)[nearest])[0]
	anchor2 := ((*anchors)[secondNearest])[0]
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
		if errorParts[0] != "insufficient anchor" {
			return nil, errors.Wrap(err, "failed to find OSM-node")
		}

		faultyNodeID := strings.ReplaceAll(errorParts[1], " ", "")

		if faultyNodeID == anchor1.Id {
			nearest = sortedAnchors[newAnchorCounter]
			anchor1 = ((*anchors)[nearest])[0]
			distance1 = math.Abs(nearest - kilometrage)
			newAnchorCounter++
		} else {
			secondNearest = sortedAnchors[newAnchorCounter]
			anchor2 = ((*anchors)[secondNearest])[0]
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

	if newAnchorCounter == len(sortedAnchors) {
		return nil, errors.New("failed to find suitable anchors")
	}

	return newNode, nil
}

// sortAnchors sorts the kilometrage-keys of the anchor-map.
// The sort is based on least distance to the provided 'kilometrage'.
func sortAnchors(
	anchors *map[float64]([]*OSMUtil.Node),
	kilometrage float64,
) []float64 {
	anchorKeys := []float64{}
	for anchorKey := range *anchors {
		anchorKeys = append(anchorKeys, anchorKey)
	}

	sort.SliceStable(anchorKeys, func(i, j int) bool {
		floatKilometrage1, floatKilometrage2 := anchorKeys[i], anchorKeys[j]
		return math.Abs(kilometrage-floatKilometrage1) < math.Abs(kilometrage-floatKilometrage2)
	})

	return anchorKeys
}

// formatKilometrageStringInFloat formats the kilometrage and parses them to a float64.
// Kilometrages in the DB-data always use a comma for decimal separation and sometimes, a plus sign is present.
// This plus sign is parsed in a way, that e.g. '5,000+0,150' becomes '5.15'.
func formatKilometrageStringInFloat(
	in string,
) (float64, error) {
	split := strings.Split(in, "+")
	if len(split) == 1 {
		return strconv.ParseFloat(
			strings.ReplaceAll(in, ",", "."),
			64)
	}

	out := 0.0
	for _, splitPart := range split {
		floatPart, err := strconv.ParseFloat(
			strings.ReplaceAll(splitPart, ",", "."),
			64)
		if err != nil {
			return 0, errors.Wrap(err, "failed to parse kilometrage: "+in)
		}
		out += floatPart
	}

	return out, nil
}
