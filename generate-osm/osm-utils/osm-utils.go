package osmUtils

import (
	"encoding/xml"
	"fmt"
	"os"
	"os/exec"
)

type Tag struct {
	XMLName xml.Name `xml:"tag"`
	K       string   `xml:"k,attr"`
	V       string   `xml:"v,attr"`
}

type Nd struct {
	XMLName xml.Name `xml:"nd"`
	Ref     string   `xml:"ref,attr"`
}

type Way struct {
	XMLName   xml.Name `xml:"way"`
	Tag       []*Tag   `xml:"tag"`
	Id        string   `xml:"id,attr"`
	Version   string   `xml:"version,attr"`
	Timestamp string   `xml:"timestamp,attr"`
	Nd        []*Nd    `xml:"nd"`
}

type Node struct {
	XMLName   xml.Name `xml:"node"`
	Tag       []*Tag   `xml:"tag"`
	Id        string   `xml:"id,attr"`
	Version   string   `xml:"version,attr"`
	Timestamp string   `xml:"timestamp,attr"`
	Lat       string   `xml:"lat,attr"`
	Lon       string   `xml:"lon,attr"`
}

type Member struct {
	XMLName xml.Name `xml:"member"`
	Type    string   `xml:"type,attr"`
	Ref     string   `xml:"ref,attr"`
	Role    string   `xml:"role,attr"`
}

type Relation struct {
	XMLName   xml.Name  `xml:"relation"`
	Member    []*Member `xml:"member"`
	Tag       []*Tag    `xml:"tag"`
	Id        string    `xml:"id,attr"`
	Version   string    `xml:"version,attr"`
	Timestamp string    `xml:"timestamp,attr"`
}

type Osm struct {
	XMLName   xml.Name    `xml:"osm"`
	Version   string      `xml:"version,attr"`
	Generator string      `xml:"generator,attr"`
	Way       []*Way      `xml:"way"`
	Node      []*Node     `xml:"node"`
	Relation  []*Relation `xml:"relation"`
}

func ExecuteOsmFilterCommand(args []string) {
	logOsmCommands := false
	osmExecutable, _ := exec.LookPath("osmium")
	argsArray := []string{
		osmExecutable,
		"tags-filter",
	}
	argsArray = append(argsArray, args...)
	if logOsmCommands {
		fmt.Println(argsArray)
	}

	cmd := &exec.Cmd{
		Path:   osmExecutable,
		Args:   argsArray,
		Stdout: os.Stdout,
		Stderr: os.Stdout,
	}

	if err := cmd.Run(); err != nil {
		fmt.Println("Error:", err)
	}
}
