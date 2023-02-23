package dbUtils

import (
	"bytes"
	"encoding/xml"
	"fmt"
	"os"
	"path/filepath"

	"golang.org/x/net/html/charset"
)

type XmlIssBookeeping struct {
	XmlIssData         *XmlIssDaten
	NumBetriebsstellen int
	Used               bool
}

func Parse(refs []string, tempDBLinesPath string, dbResourcesPath string) []string {
	combinedDBIss, err := readDBFiles(dbResourcesPath)
	if err != nil {
		panic(err)
	}

	lineMap := make(map[string]*XmlIssBookeeping)
	// in missingMap, all lines, for which DB-data exists but no OSM-data (i.e. not appearing in refs) are listed
	missingMap := []string{}

	// all datastructures are being intialized
	for _, line := range refs {
		lineMap[line] = &XmlIssBookeeping{&XmlIssDaten{xml.Name{" ", "XmlIssDaten"}, []*Spurplanbetriebsstelle{}}, 0, false}
	}

	// main work-loop: For all "Betriebsstellen" and for all "Spurplanabschnitte" of these, we check, whether the respective line
	// appears in refs and if so add the "Abschnitt" to the "Betriebsstelle" in the respective line
	for _, stelle := range combinedDBIss.Betriebsstellen {
		for _, abschnitt := range stelle.Abschnitte {
			streckenNummer := (*abschnitt.StreckenNr[0]).Nummer
			lineInfo, lineExists := lineMap[streckenNummer]

			if !lineExists {
				missingMap = append(missingMap, streckenNummer)
				continue
			}

			lineData := lineInfo.XmlIssData
			numBetriebsstellen := lineInfo.NumBetriebsstellen

			// if no "Abschnitt" has yet been added to this particular "Betriebsstelle", we must first create one
			if len(lineData.Betriebsstellen) == numBetriebsstellen {
				lineData.Betriebsstellen = append(lineData.Betriebsstellen, &Spurplanbetriebsstelle{stelle.XMLName, stelle.Name, []*Spurplanabschnitt{}})
				lineInfo.Used = true
			}
			lineData.Betriebsstellen[numBetriebsstellen].Abschnitte = append(lineData.Betriebsstellen[numBetriebsstellen].Abschnitte, abschnitt)
		}

		// final increment of all "Betriebsstellen"-counters where neccessary
		for _, lineInfo := range lineMap {
			if lineInfo.Used {
				lineInfo.NumBetriebsstellen += 1
				lineInfo.Used = false
			}
		}
	}

	relevantRefs := []string{}
	os.Mkdir(tempDBLinesPath, 0755)
	//final work-loop: For all collected lines, .xml-files must be marshelled
	for line, lineInfo := range lineMap {
		if len(lineInfo.XmlIssData.Betriebsstellen) == 0 {
			continue
		}

		newIssBytes, err := xml.MarshalIndent(*lineInfo.XmlIssData, "", "	")
		if err != nil {
			panic(err)
		}

		tempLinePath := filepath.Join(tempDBLinesPath, line+"_DB.xml")
		err = os.WriteFile(tempLinePath, []byte(xml.Header+string(newIssBytes)), 0644)
		if err != nil {
			panic(err)
		}

		relevantRefs = append(relevantRefs, line)
	}

	return relevantRefs
}

func readDBFiles(dbResourcesPath string) (XmlIssDaten, error) {

	// read all files and unmarshal them into one XmlIssDaten-struct
	files, err := os.ReadDir(dbResourcesPath)
	if err != nil {
		return XmlIssDaten{}, err
	}
	var xmlIssComplete XmlIssDaten

	for _, file := range files {
		fmt.Printf("Processing %s... \r", file.Name())
		data, _ := os.ReadFile(dbResourcesPath + "/" + file.Name())
		reader := bytes.NewReader(data)
		decoder := xml.NewDecoder(reader)
		decoder.CharsetReader = charset.NewReaderLabel
		err = decoder.Decode(&xmlIssComplete)
		if err != nil {
			return XmlIssDaten{}, err
		}
	}

	return xmlIssComplete, nil
}
