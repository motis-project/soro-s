package osmUtils

import (
	"encoding/xml"
	"os"
	"path/filepath"

	"github.com/pkg/errors"
)

// GenerateOSMTrackRefs generates separate files for every line existent in the OSM-file under 'inputFilePath'.
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
		return nil, errors.Wrap(err, "failed reading track ref file")
	}
	var osmData Osm
	if err := xml.Unmarshal([]byte(data), &osmData); err != nil {
		return nil, errors.Wrap(err, "failed unmarshalling osm file: "+refsPath)
	}

	return getRefIds(osmData), nil
}

// getRefIds extracts the line-names from all line-relations in the 'trackRefOsm'.
func getRefIds(trackRefOsm Osm) (refs []string) {
	for _, s := range trackRefOsm.Relation {
		for _, m := range s.Tag {
			if m.K == "ref" &&
				len(m.V) == 4 {
				refs = append(refs, m.V)
			}
		}
	}

	return refs
}
