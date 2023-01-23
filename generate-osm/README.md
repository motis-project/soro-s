<p align="center"><img src="logo.png" width="350"></p>

## Generate OSM Setup

To setup generate-osm for developing purposes please follow one of these instructions.

Needed dependencies:
- [Golang](https://golang.org/doc/install)
- [Osmium](https://osmcode.org/osmium-tool/) (Need to be in PATH variable as osmium)

You need to add the osm file from [Geofabrik](https://download.geofabrik.de/europe/germany.html) to the folder `generate-osm\temp` and rename it to `base.osm.pbf`. Then you can run the following command to generate the osm file:
```bash 
go build
./transform-osm
```

The final osm file will be in the folder `generate-osm\temp` and named `finalOsm.xml`. The JSON station and halt file will be in the folder `generate-osm\temp` and named `stations.json`. 
