package findNodes

import (
	"math"
	"sort"
	"strconv"
	"strings"

	OSMUtil "transform-osm/osm-utils"

	"github.com/pkg/errors"
)

const EARTH_RADIUS_CONST = 6371.0

type NdRefPosition int

const (
	FirstNdRef NdRefPosition = iota
	SecondNdRef
	SecondLastNdRef
	LastNdRef
)

// GetWayNdRef returns the reference of the node at the given index. The index can be one of the following:
// FirstNdRef, SecondNdRef, SecondLastNdRef, LastNdRef
func GetWayNdRef(
	way OSMUtil.Way,
	ndRefPosition NdRefPosition,
) string {
	if ndRefPosition == FirstNdRef {
		return way.Nd[0].Ref
	}
	if ndRefPosition == SecondNdRef {
		return way.Nd[1].Ref
	}
	if ndRefPosition == SecondLastNdRef {
		return way.Nd[len(way.Nd)-2].Ref
	}

	return way.Nd[len(way.Nd)-1].Ref
}

// SortAnchors sorts the kilometrage-keys of the anchor-map.
// The sort is based on least distance to the provided 'kilometrage'.
func SortAnchors(
	anchors map[float64]([]*OSMUtil.Node),
	kilometrage float64,
) []float64 {
	anchorKeys := []float64{}
	for anchorKey := range anchors {
		anchorKeys = append(anchorKeys, anchorKey)
	}

	sort.SliceStable(anchorKeys, func(i, j int) bool {
		floatKilometrage1, floatKilometrage2 := anchorKeys[i], anchorKeys[j]
		return math.Abs(kilometrage-floatKilometrage1) < math.Abs(kilometrage-floatKilometrage2)
	})

	return anchorKeys
}

// FormatKilometrageStringInFloat formats the kilometrage and parses them to a float64.
// Kilometrages in the DB-data always use a comma for decimal separation and sometimes, a plus sign is present.
// This plus sign is parsed in a way, that e.g. '5,000+0,150' becomes '5.15'.
func FormatKilometrageStringInFloat(
	kilometrageString string,
) (float64, error) {
	split := strings.Split(kilometrageString, "+")
	if len(split) == 1 {
		return strconv.ParseFloat(
			strings.ReplaceAll(kilometrageString, ",", "."),
			64)
	}

	kilometrage := 0.0
	for _, splitPart := range split {
		floatPart, err := strconv.ParseFloat(
			strings.ReplaceAll(splitPart, ",", "."),
			64)
		if err != nil {
			return 0, errors.Wrap(err, "failed to parse kilometrage: "+kilometrageString)
		}
		kilometrage += floatPart
	}

	return kilometrage, nil
}

// ComputeNodeInformation returns the Node, its Latitude and Longitude provided with the desired Node-ID.
func ComputeNodeInformation(osmData *OSMUtil.Osm, nodeID string) (*OSMUtil.Node, float64, float64, error) {
	node, err := OSMUtil.GetNodeById(osmData, nodeID)
	if err != nil {
		return nil, 0, 0, errors.Wrap(err, "failed to find node")
	}
	nodeLat, _ := strconv.ParseFloat(node.Lat, 64)
	nodeLon, _ := strconv.ParseFloat(node.Lon, 64)

	return node, nodeLat, nodeLon, nil
}

// ComputeHaversineDistance computes the Haversine distance between the two points (phi1, lambda1) and (phi2, lambda2) in (Latitude, Longitude).
func ComputeHaversineDistance(phi1 float64, phi2 float64, lambda1 float64, lambda2 float64) float64 {
	phi1, phi2, lambda1, lambda2 = phi1*(math.Pi/180.0), phi2*(math.Pi/180.0), lambda1*(math.Pi/180.0), lambda2*(math.Pi/180.0)
	return 2.0 * EARTH_RADIUS_CONST * math.Asin(
		math.Sqrt(
			math.Pow(math.Sin((phi2-phi1)/2), 2)+
				math.Cos(phi1)*math.Cos(phi2)*math.Pow(math.Sin((lambda2-lambda1)/2), 2)))
}

// FindNextRunningNode returns the next Node in the provided 'runningWay' depending on the direction provided.
func FindNextRunningNode(
	osmData *OSMUtil.Osm,
	wayDirUp bool,
	index int,
	runningWay OSMUtil.Way,
) (*OSMUtil.Node, error) {
	nextIndex := index + 1
	if wayDirUp {
		nextIndex = index - 1
	}

	nextNode, err := OSMUtil.GetNodeById(osmData, runningWay.Nd[nextIndex].Ref)
	if err != nil {
		return nil, errors.Wrap(err, "failed to find node")
	}
	return nextNode, nil
}
