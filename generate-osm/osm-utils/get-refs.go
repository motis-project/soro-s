package osmUtils

import (
	"encoding/xml"
	"errors"
	"os"
	"path/filepath"
)

func GenerateOsmTrackRefs(inputFilePath string, tempFilePath string) (refs []string, err error) {
	refsPath, _ := filepath.Abs(tempFilePath + "/refs.xml")

	ExecuteOsmFilterCommand([]string{
		"-R",
		inputFilePath,
		"-o",
		refsPath,
		"r/route=tracks,railway",
		"--overwrite",
	})

	var data []byte
	if data, err = os.ReadFile(refsPath); err != nil {
		return nil, errors.New("Failed to read track ref file: " + err.Error())
	}
	var osmData Osm
	if err := xml.Unmarshal([]byte(data), &osmData); err != nil {
		return nil, err
	}

	return getRefIds(osmData)
}

func getRefIds(trackRefOsm Osm) (refs []string, err error) {
	for _, s := range trackRefOsm.Relation {
		for _, m := range s.Tag {
			if m.K == "ref" &&
				len(m.V) == 4 {
				refs = append(refs, m.V)
			}
		}
	}

	return refs, nil
}
