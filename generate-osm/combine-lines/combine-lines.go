package combineLines

import (
	"encoding/xml"
	"errors"
	"fmt"
	"math/rand"
	"os"
	osmUtils "transform-osm/osm-utils"
)

var ErrLinesDirNotFound = errors.New("lines directory not found")

func CombineAllLines() (osmData osmUtils.Osm, err error) {
	files, err := os.ReadDir("./temp/lines")
	if err != nil {
		return osmUtils.Osm{}, ErrLinesDirNotFound
	}

	for _, file := range files {
		fmt.Printf("Processing %s... ", file.Name())
		data, _ := os.ReadFile("./temp/lines/" + file.Name())
		var fileOsmData osmUtils.Osm
		if err := xml.Unmarshal([]byte(data), &fileOsmData); err != nil {
			panic(err)
		}
		// Gernerate random colours for the lines
		var color = getRandomColor()
		for i := range fileOsmData.Way {
			fileOsmData.Way[i].Tag = append(fileOsmData.Way[i].Tag, &osmUtils.Tag{
				K: "color",
				V: color,
			})
		}

		osmData.Node = append(osmData.Node, fileOsmData.Node...)
		osmData.Way = append(osmData.Way, fileOsmData.Way...)
		osmData.Relation = append(osmData.Relation, fileOsmData.Relation...)
		fmt.Println("Done")
	}
	fmt.Println("Done processing files")

	return osmData, nil
}

func getRandomColor() string {
	letters := "0123456789ABCDEF"
	color := "#"
	for i := 0; i < 6; i++ {
		color += string(letters[rand.Intn(len(letters))])
	}
	return color
}
