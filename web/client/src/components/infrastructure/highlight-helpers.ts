// @ts-nocheck as this is currently unused
import { iterate } from '@/util/iterate.js';
import { Map, Source } from 'maplibre-gl';

function highlightPath(mapSource: Source, infrastructure: string, nodeID: string, nodePath: never) {
    const pathGeoJSON = {
        type: 'Feature',
        properties: { id: nodeID },
        geometry: {
            coordinates: [],
            type: 'LineString',
        },
    };

    for (const node of iterate(nodePath)) {
        const gps = infrastructure.element_coords.get(node.element.id);
        pathGeoJSON.geometry.coordinates.push([gps.lon, gps.lat]);
    }

    // this is probably illegal, but very convenient
    const newData = mapSource._data;

    newData.features.push(pathGeoJSON);
    mapSource.setData(newData);
}

function deHighlightPath(mapSource: Source, pathID: string) {
    // again: probably quite unethical
    const newData = mapSource._data;
    const idx = newData.features.findIndex(f => f.properties.id === pathID);
    newData.features.splice(idx, 1);

    mapSource.setData(newData);
}

export function highlightSignalStationRoute(map: Map, infrastructure: string, signalStationRouteID: string) {
    highlightPath(
        map.getSource('signal-station-routes'),
        infrastructure,
        signalStationRouteID,
        infrastructure.signal_station_routes.get(signalStationRouteID).nodes,
    );
}

export function deHighlightSignalStationRoute(map: Map, signalStationRouteID: string) {
    deHighlightPath(map.getSource('signal-station-routes'), signalStationRouteID);
}

export function highlightStationRoute(map: Map, infrastructure: string, stationRouteID: string) {
    highlightPath(
        map.getSource('station-routes'),
        infrastructure,
        stationRouteID,
        infrastructure.station_routes.get(stationRouteID).nodes,
    );
}

export function deHighlightStationRoute(map: Map, stationRouteID: string) {
    deHighlightPath(map.getSource('station-routes'), stationRouteID);
}
