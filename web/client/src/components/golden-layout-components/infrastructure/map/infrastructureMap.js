import { infrastructureMapStyle, mapLayers } from './mapStyle.js'
import * as maplibre from "../deps/maplibre-gl.js";
import { addIcons } from "./addIcons.js";
import { iconUrl } from "./addIcons.js";
import { iconExtension } from "./addIcons.js";
import { iterate } from "../../../../util/iterate.js";
import { elementTypes } from "./elementTypes.js";
import { elementTypesReadable } from "./elementTypes.js";

function getMaxBounds(infrastructure) {
  // lon_max, lon_min, lat_max, lat_min,
  let boundingBox = [[-360, 360], [-360, 360]];

  for (const gps of iterate(infrastructure.station_coords)) {
    boundingBox[0][0] = Math.max(boundingBox[0][0], gps.lon);
    boundingBox[0][1] = Math.min(boundingBox[0][1], gps.lon);
    boundingBox[1][0] = Math.max(boundingBox[1][0], gps.lat);
    boundingBox[1][1] = Math.min(boundingBox[1][1], gps.lat);
  }

  return boundingBox;
}

function getMiddle(infrastructure) {
  const boundingBox = getMaxBounds(infrastructure);

  return [
    boundingBox[0][1] + (boundingBox[0][0] - boundingBox[0][1]) / 2,
    boundingBox[1][1] + (boundingBox[1][0] - boundingBox[1][1]) / 2
  ];
}

function createLegend(map, rootElement) {
  const legendCallback = event => {
    map.setLayoutProperty(event.target.value + '-layer', 'visibility', event.target.checked ? 'visible' : 'none');

    if (event.target.value !== 'station') {
      map.setLayoutProperty('circle-' + event.target.value + '-layer', 'visibility', event.target.checked ? 'visible' : 'none');
    }
  };

  const initially_checked = new Set(['station', 'ms', 'as', 'eotd']);

  const legend = rootElement.querySelector('#mapLegend');
  for (const elementType of elementTypes) {
    const input = document.createElement('input');
    input.id = elementType;
    input.type = 'checkbox';
    input.value = elementType;
    input.checked = initially_checked.has(elementType);
    input.addEventListener('input', legendCallback);

    input.dispatchEvent(new Event('input'))

    const icon = document.createElement('img');
    icon.classList.add('legend-key-icon');
    icon.src = iconUrl + elementType + iconExtension;

    const label = document.createElement('label');
    label.classList.add('legend-key');
    label.htmlFor = elementType;
    label.appendChild(icon);
    label.append(elementTypesReadable[elementType]);

    const br = document.createElement('br');

    legend.appendChild(input);
    legend.appendChild(label);
    legend.appendChild(br);
  }

  const dirLegendCallback = event => {
    const rising_checked = event.target.value === 'Rising' ? event.target.checked :
      rootElement.querySelector('#Rising').checked;

    const falling_checked = event.target.value === 'Falling' ? event.target.checked :
      rootElement.querySelector('#Falling').checked;

    let filter = undefined;

    if (!rising_checked && falling_checked) {
      filter = ['!', ['get', 'rising']];
    } else if (rising_checked && !falling_checked) {
      filter = ['get', 'rising'];
    } else if (!rising_checked && !falling_checked) {
      filter = ['boolean', false];
    }

    for (const elementType of elementTypes) {
      if (elementType === 'station') {
        continue;
      }

      map.setFilter(elementType + '-layer', filter);
      map.setFilter('circle-' + elementType + '-layer', filter);
    }
  }

  for (const dir of ['Rising', 'Falling']) {
    const input = document.createElement('input');
    input.id = dir;
    input.type = 'checkbox';
    input.value = dir;
    input.checked = true;
    input.addEventListener('input', dirLegendCallback);

    const label = document.createElement('label');
    label.classList.add('legend-key');
    label.htmlFor = dir;
    label.append(dir);

    const br = document.createElement('br');

    legend.appendChild(input);
    legend.appendChild(label);
    legend.appendChild(br);
  }
}

export function createMap(rootElement, infrastructureName, tooltip) {
  let map = new maplibregl.Map({
    container: rootElement.querySelector('#map'),
    style: infrastructureMapStyle,
    attributionControl: false,
    zoom: 14,
    hash: "location",
    center: [14, 49],
    maxBounds: [[6, 45], [17, 55]], // [SW Point] [NE Point] in LonLat
    bearing: 0,
    transformRequest: (relative_url, _resourceType) => {
      if (relative_url.startsWith('/')) {
        const url = window.origin + '/' + infrastructureName + relative_url;
        return {url: url};
      }
    }
  });

  map.on('load', () => {
    addIcons(map);
    createLegend(map, rootElement);
  });

  map.dragPan.enable({
    linearity: 0.01,
    easing: t => t,
    maxSpeed: 1400,
    deceleration: 2500
  });

  map.on('click', e => {
    const features = e.target.queryRenderedFeatures(e.point, {layers: mapLayers});

    if (features.length === 0) {
      return;
    }

    if (features.length > 1) {
      console.warn("More than one feature found after click event! Using the first one.", features);
    }

    const clickedFeature = features[0];
    const clickedID = clickedFeature.properties.id;

    // if (clickedFeature.layer['source-layer'] === 'station') {
    //   const station = infrastructure.stations.get(clickedID);
    //   const gps = infrastructure.station_coords.get(clickedID);
    //   showStation(station, infrastructure, map.getSource('station-routes')._data.features,
    //     map.getSource('signal-station-routes')._data.features);
    // } else {
    //   console.log("clicked", clickedFeature);
    //
    //   const element = infrastructure.element(clickedFeature.properties.id);
    //
    //   if (element.is_track_element()) {
    //     const km = element.as_track_element().km;
    //     tooltip.select('#kilometerPoint').text((km * 1000).toFixed(0) + 'm');
    //     const rising = element.as_track_element().rising;
    //     tooltip.select('#risingOrFalling').text(rising ? 'Rising' : 'Falling');
    //   }
    //
    //   tooltip.click(e, e => e.point.x, e => e.point.y);
    // }
  });

  return map;
}

function highlightPath(mapSource, infrastructure, nodeID, nodePath) {
  let pathGeoJSON = {
    type: 'Feature',
    properties: {id: nodeID},
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
