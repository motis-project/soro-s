package osmUtils

import (
	"encoding/xml"
	"errors"
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
	XMLName xml.Name `xml:"way"`
	Tag     []*Tag   `xml:"tag"`
	Id      string   `xml:"id,attr"`
	Nd      []*Nd    `xml:"nd"`
}

type Node struct {
	XMLName xml.Name `xml:"node"`
	Tag     []*Tag   `xml:"tag"`
	Id      string   `xml:"id,attr"`
	Lat     string   `xml:"lat,attr"`
	Lon     string   `xml:"lon,attr"`
}

type Member struct {
	XMLName xml.Name `xml:"member"`
	Type    string   `xml:"type,attr"`
	Ref     string   `xml:"ref,attr"`
	Role    string   `xml:"role,attr"`
}

type Relation struct {
	XMLName xml.Name  `xml:"relation"`
	Member  []*Member `xml:"member"`
	Tag     []*Tag    `xml:"tag"`
	Id      string    `xml:"id,attr"`
}

type Osm struct {
	XMLName   xml.Name    `xml:"osm"`
	Version   string      `xml:"version,attr"`
	Generator string      `xml:"generator,attr"`
	Node      []*Node     `xml:"node"`
	Way       []*Way      `xml:"way"`
	Relation  []*Relation `xml:"relation"`
}

func ExecuteOsmFilterCommand(args []string) error {
	osmExecutable, _ := exec.LookPath("osmium")
	argsArray := []string{
		osmExecutable,
		"tags-filter",
	}
	argsArray = append(argsArray, args...)

	cmd := &exec.Cmd{
		Path:   osmExecutable,
		Args:   argsArray,
		Stdout: os.Stdout,
		Stderr: os.Stdout,
	}

	if err := cmd.Run(); err != nil {
		return errors.New(fmt.Errorf("osmium command failed: %w", err).Error())
	}

	return nil
}
