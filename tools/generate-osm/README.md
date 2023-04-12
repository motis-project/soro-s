## Generate OSM Setup

This README contains information on how to use the generate-osm build for production and development environments.

### Tools
- [Golang](https://golang.org/doc/install)
- [Osmium](https://osmcode.org/osmium-tool/) (Need to be in PATH variable as osmium)
- For documentation: run `go install golang.org/x/tools/cmd/godoc@latest` in the GO root directory

### Processing OSM files

1. Download the `.osm.pbf` file corresponding to your area from [Geofabrik](https://download.geofabrik.de/europe/germany.html) to the folder `generate-osm/temp`.
2. If you want to map any data in the DB-XMLIss format, you need to add all files to the folder `generate-osm/temp/DBResources`.
3. Run the following command to generate the annotated OSM file:
```bash 
go build
./transform-osm
```
Note that depending on the name of your file, you may need to use the CLI Flag `--input`. For further details see the CLI Flag reference down below.

The final OSM file as well as the generated search indices will be in the folder `generate-osm` and named `finalOsm.{xml,json}` (if not specified otherwise via `--output`).

Both generated files (OSM and JSON search indices) now have to be copied as by the following:
1. OSM file: `/soro-s/resources/osm`
2. JSON file: `/soro-s/resources/search_indices`

Note that it is crucial that the files have the same name (up to their file extension) in order for search indices to be discovered properly.
Also note that **you may have to rebuild your cmake project** to apply the changes.

#### CLI-Flags
```bash
--generate-lines, --gl    Generate lines all lines new (default: false)
--mapDB, --mdb            Generate lines all lines new as well as map any DB data (default: false)
--input value, -i value   The input file to read as OSM PBF file (default: "./temp/base.osm.pbf")
--output value, -o value  The output file to write annotated OSM result (filtered by rail) to as XML file (default: "./finalOsm.xml")
--additional-osm value,   An additional OSM file in XML format to add parsed DB data to (default: "" (do not add))
  --addOsm value  
--help, -h                Show help
```

### Miscellaneous

To access comprehensible documentation for all public methods, run 
```bash
godoc -http=:6060
```
in the `generate-osm` directory and access it via the browser under `localhost:6060/`. You will find it under the tab `standard library / transform-osm`.
