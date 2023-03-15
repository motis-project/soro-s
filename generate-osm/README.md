## Generate OSM Setup

To setup generate-osm for developing purposes please follow one of these instructions.

Needed dependencies:
- [Golang](https://golang.org/doc/install)
- [Osmium](https://osmcode.org/osmium-tool/) (Need to be in PATH variable as osmium)
- For documentation: run `go install golang.org/x/tools/cmd/godoc@latest` in the GO root directory

You need to add the osm file from [Geofabrik](https://download.geofabrik.de/europe/germany.html) to the folder `generate-osm/temp` and rename it to `base.osm.pbf`. If you want to map any data in the DB-XMLIss format, you need to add all files to the folder `generate-osm/temp/DBResources`. Then you can run the following command to generate the osm file:

```bash 
go build
./transform-osm
```

The final osm file will be in the folder `generate-osm` and named `finalOsmDB.xml`. The JSON search file will be in the folder `generate-osm\` and named the same as the output-`.xml`.
Alternatively you can provide an OSM-file, in which the generated data will be inserted. In this case, the new output-file will be named `[name provided file]DB.xml` and the JSON will also be named accordingly.

CLI-Flags
```bash
--generate-lines, --gl   Generate lines all lines new (default: false)
--mapDB, --mdb   Generate lines all lines new as well as map an DB data (default: false)
--input value, -i value  The input file to read as OSM PBF file (default: "./temp/base.osm.pbf")
--output value, -o value  The output file to write result to as XML file (default: "./finalOsm.xml")
--help, -h               show help
```

To access comprehensible documention for all public methods, run 
```bash
godoc -http=:6060
```
in the `generate-osm` directory and access it via the browser under `localhost:6060/`. You will find it under the tab `standard library / transform-osm`