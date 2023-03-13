package combineLines

import (
	"encoding/xml"
	"fmt"
	"math"
	"math/rand"
	"os"
	"strconv"
	osmUtils "transform-osm/osm-utils"

	"github.com/pkg/errors"
)

var ErrLinesDirNotFound = errors.New("lines directory not found")

// CombineAllLines reads all files from the provided 'tempLineDir' and tries to combine them into one OSM-data-set.
// Therefore, all files in the tempLineDir must be OSM-files.
func CombineAllLines(tempLineDir string) (osmUtils.Osm, error) {
	files, err := os.ReadDir(tempLineDir)

	if err != nil {
		return osmUtils.Osm{}, errors.Wrap(err, ErrLinesDirNotFound.Error())
	}

	var osmData osmUtils.Osm

	for _, file := range files {
		fmt.Printf("Processing %s... ", file.Name())
		data, _ := os.ReadFile(tempLineDir + "/" + file.Name())

		var fileOsmData osmUtils.Osm
		if err := xml.Unmarshal([]byte(data), &fileOsmData); err != nil {
			return osmUtils.Osm{}, errors.Wrap(err, "could not unmarshal osm-data from file: "+file.Name())
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

// getRandomColor generates a bright and saturated colour and returns its RGB-Hex value.
func getRandomColor() string {

	var h, s, v float64

	//s=1 and v=1 leads to bright and saturated colors
	h = (float64)(rand.Intn((360)))
	s = 1
	v = 1

	return hsvToHexRgb(h, s, v)
}

// hsvToHexRgb converts a colour from the hsv colour space into a RGB-hex string, starting with '#'.
func hsvToHexRgb(h float64, s float64, v float64) string {

	var r, g, b int64

	//convert HSV to RGB using the standard formula
	h_i := math.Floor(h / 60)

	f := (h / 60) - h_i

	p := v * (1 - s)
	q := v * (1 - s*f)
	t := v * (1 - s*(1-f))

	switch {
	case h_i == 0 || h_i == 6:
		r = int64(math.Floor(v * 255))
		g = int64(math.Floor(t * 255))
		b = int64(math.Floor(p * 255))
	case h_i == 1:
		r = int64(math.Floor(q * 255))
		g = int64(math.Floor(v * 255))
		b = int64(math.Floor(p * 255))
	case h_i == 2:
		r = int64(math.Floor(p * 255))
		g = int64(math.Floor(v * 255))
		b = int64(math.Floor(t * 255))
	case h_i == 3:
		r = int64(math.Floor(p * 255))
		g = int64(math.Floor(q * 255))
		b = int64(math.Floor(v * 255))
	case h_i == 4:
		r = int64(math.Floor(t * 255))
		g = int64(math.Floor(p * 255))
		b = int64(math.Floor(v * 255))
	case h_i == 5:
		r = int64(math.Floor(v * 255))
		g = int64(math.Floor(p * 255))
		b = int64(math.Floor(q * 255))
	}

	color := "#"

	//translate rgb values to hex
	//this is done by translating the int64 to a hexadecimal string and padding it to a length of two if needed  (11 -> "b" -> "0b")
	color += leftPad(strconv.FormatInt((r), 16), 2, "0") + leftPad(strconv.FormatInt((g), 16), 2, "0") + leftPad(strconv.FormatInt((b), 16), 2, "0")

	return color
}

// leftPad appends 'padding' as many times to the front of 'stringToPad' until the desired length is reached.
func leftPad(stringToPad string, length int, padding string) string {
	for len(stringToPad) < length {
		stringToPad = padding + stringToPad
	}
	return stringToPad
}
