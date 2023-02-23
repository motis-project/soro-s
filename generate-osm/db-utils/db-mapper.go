package dbUtils

import (
	"encoding/xml"
	"log"
	"os"
	OSMUtil "transform-osm/osm-utils"
)

func MapDB(refs []string, osmDir string, dbDir string) {
	for _, line := range refs {
		// mappedItems := make(map[string]OSMUtil.Node)

		var osmData OSMUtil.Osm
		var dbData XmlIssDaten

		osmFile, err := os.ReadFile(osmDir + "/" + line + ".xml")
		if err != nil {
			log.Fatal(err)
		}
		dbFile, err := os.ReadFile(osmDir + "/" + line + "_DB.xml")
		if err != nil {
			log.Fatal(err)
		}

		if err := xml.Unmarshal([]byte(osmFile), &osmData); err != nil {
			panic(err)
		}
		if err := xml.Unmarshal([]byte(dbFile), &dbData); err != nil {
			panic(err)
		}

		/*
			mapSignals(&osmData, dbData, &mappedItems)
			mapPoints(&osmData, dbData, &mappedItems)
			mapRest(&osmData, dbData, &mappedItems)
		*/
	}
}
