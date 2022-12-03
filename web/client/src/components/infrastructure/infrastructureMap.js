import { iterate } from '../../util/iterate.js';

function highlightPath(mapSource, infrastructure, nodeID, nodePath) {
	let pathGeoJSON = {
		type: 'Feature',
		properties: { id: nodeID },
		geometry: {
			coordinates: [],
			type: 'LineString'
		}
	};

	for (const node of iterate(nodePath)) {
		const gps = infrastructure.element_coords.get(node.element.id);
		pathGeoJSON.geometry.coordinates.push([gps.lon, gps.lat]);
	}

	// this is probably illegal, but very convenient
	let newData = mapSource._data;

	newData.features.push(pathGeoJSON);
	mapSource.setData(newData);
}

function deHighlightPath(mapSource, pathID) {
	// again: probably quite unethical
	let newData = mapSource._data;
	const idx = newData.features.findIndex(f => f.properties.id === pathID);
	newData.features.splice(idx, 1);

	mapSource.setData(newData);
}

export function highlightSignalStationRoute(map, infrastructure, signalStationRouteID) {
	highlightPath(map.getSource('signal-station-routes'),
		infrastructure,
		signalStationRouteID,
		infrastructure.signal_station_routes.get(signalStationRouteID).nodes);
}

export function deHighlightSignalStationRoute(map, signalStationRouteID) {
	deHighlightPath(map.getSource('signal-station-routes'), signalStationRouteID);
}

export function highlightStationRoute(map, infrastructure, stationRouteID) {
	highlightPath(map.getSource('station-routes'),
		infrastructure,
		stationRouteID,
		infrastructure.station_routes.get(stationRouteID).nodes);
}

export function deHighlightStationRoute(map, stationRouteID) {
	deHighlightPath(map.getSource('station-routes'), stationRouteID);
}
