## Generate OSM Setup

To setup generate-osm for developing purposes please follow one of these instructions.

Needed dependencies:
- [Golang](https://golang.org/doc/install)
- [Osmium](https://osmcode.org/osmium-tool/) (Need to be in PATH variable as osmium)

You need to add the osm file from [Geofabrik](https://download.geofabrik.de/europe/germany.html) to the folder `generate-osm/temp` and rename it to `base.osm.pbf`. If you want to map any data in the DB-XMLIss format, you need to add all files to the folder `generate-osm/temp/DBResources`. Then you can run the following command to generate the osm file:

```bash 
go build
./transform-osm
```

The final osm file will be in the folder `generate-osm` and named `finalOsm.xml`. The JSON station and halt file will be in the folder `generate-osm\temp` and named `stations.json`. 

CLI-Flags
```bash
--generate-lines, --gl   Generate lines all lines new (default: false)
--mapDB, --mdb   Generate lines all lines new as well as map an DB data (default: false)
--input value, -i value  The input file to read as OSM PBF file (default: "./temp/base.osm.pbf")
--output value, -o value  The output file to write result to as XML file (default: "./temp/finalOsm")
--help, -h               show help
```
