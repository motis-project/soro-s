package osmUtils

import (
	"sort"
	"strconv"
)

func SortOsm(osmData Osm) Osm {
	sort.Slice(osmData.Way, func(i, j int) bool {
		id1, _ := strconv.Atoi(osmData.Way[i].Id)
		id2, _ := strconv.Atoi(osmData.Way[j].Id)
		return id1 < id2
	})
	sort.Slice(osmData.Node, func(i, j int) bool {
		id1, _ := strconv.Atoi(osmData.Node[i].Id)
		id2, _ := strconv.Atoi(osmData.Node[j].Id)
		return id1 < id2
	})
	sort.Slice(osmData.Relation, func(i, j int) bool {
		id1, _ := strconv.Atoi(osmData.Relation[i].Id)
		id2, _ := strconv.Atoi(osmData.Relation[j].Id)
		return id1 < id2
	})

	// Check if there are any duplicate ids in the way, node and relation and remove them
	// This is a very simple check and does not check if the data is the same
	// It just checks if the id is the same
	var existingWayIds []string
	var existingNodeIds []string
	var existingRelationIds []string
	var newWays []*Way
	var newNodes []*Node
	var newRelations []*Relation
	for i := 0; i < len(osmData.Way); i++ {
		if len(existingWayIds) == 0 {
			existingWayIds = append(existingWayIds, osmData.Way[i].Id)
			newWays = append(newWays, osmData.Way[i])
		}

		id, _ := strconv.Atoi(osmData.Way[i].Id)
		found, _ := search(id, existingWayIds, 0, len(existingWayIds)-1)
		if !found {
			existingWayIds = append(existingWayIds, osmData.Way[i].Id)
			newWays = append(newWays, osmData.Way[i])
		}
	}
	for i := 0; i < len(osmData.Node); i++ {
		if len(existingNodeIds) == 0 {
			existingNodeIds = append(existingNodeIds, osmData.Node[i].Id)
			newNodes = append(newNodes, osmData.Node[i])
		}

		id, _ := strconv.Atoi(osmData.Node[i].Id)
		found, _ := search(id, existingNodeIds, 0, len(existingNodeIds)-1)
		if !found {
			existingNodeIds = append(existingNodeIds, osmData.Node[i].Id)
			newNodes = append(newNodes, osmData.Node[i])
		}
	}
	for i := 0; i < len(osmData.Relation); i++ {
		if len(existingRelationIds) == 0 {
			existingRelationIds = append(existingRelationIds, osmData.Relation[i].Id)
			newRelations = append(newRelations, osmData.Relation[i])
		}

		id, _ := strconv.Atoi(osmData.Relation[i].Id)
		found, _ := search(id, existingRelationIds, 0, len(existingRelationIds)-1)
		if !found {
			existingRelationIds = append(existingRelationIds, osmData.Relation[i].Id)
			newRelations = append(newRelations, osmData.Relation[i])
		}
	}

	osmData.Way = newWays
	osmData.Node = newNodes
	osmData.Relation = newRelations

	return osmData
}

func search(id int, arr []string, start, end int) (bool, int) {
	if end < 0 || start < 0 {
		return false, -1
	}
	if end-start == 0 {
		checkId, _ := strconv.Atoi(arr[end])
		if id == checkId {
			return true, end
		}
		return false, -1
	}
	pivot := (end + start) / 2
	checkId, _ := strconv.Atoi(arr[end])
	if id == checkId {
		return true, pivot
	} else if id > checkId {
		return search(id, arr, pivot+1, end)
	}
	return search(id, arr, start, pivot)
}
