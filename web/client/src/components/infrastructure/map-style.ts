import { ElementType, ElementTypes } from './element-types';
import { LayerSpecification, StyleSpecification } from 'maplibre-gl';
import {
  DisplayMode,
  DisplayModes
} from '@/components/infrastructure/display-modes';
import {
  getLayer,
  getLineLayer,
  stationLayer
} from '@/components/infrastructure/map-layers';
import { IconMode } from '@/components/infrastructure/icon-modes';
import { LegendControls } from '@/components/infrastructure/infrastructure-legend.vue';

function getSourceLayer(elementType: ElementType, displayMode: DisplayMode) {
  return displayMode + '-' + elementType;
}

function getVisibilityForMode(
  displayMode: DisplayMode,
  current: LegendControls
) {
  return current.selectedDisplayMode == displayMode ? 'visible' : 'none';
}

function getVisibility(
  elementType: ElementType,
  displayMode: DisplayMode,
  current: LegendControls
) {
  return displayMode == current.selectedDisplayMode &&
    current.selectedElementTypes.includes(elementType)
    ? 'visible'
    : 'none';
}

function getBaseStyle(): StyleSpecification {
  return {
    version: 8,
    sources: {
      osm: {
        type: 'vector',
        tiles: ['/tiles/{z}/{x}/{y}.mvt'],
        maxzoom: 20
      },
      'station-routes': {
        type: 'geojson',
        data: {
          type: 'FeatureCollection',
          features: []
        }
      },
      'interlocking-routes': {
        type: 'geojson',
        data: {
          type: 'FeatureCollection',
          features: []
        }
      }
    },
    glyphs: '/font/{fontstack}/{range}.pbf',
    layers: [
      {
        id: 'background',
        type: 'background',
        paint: {
          'background-color': '#ECE9E9'
        }
      },
      {
        id: 'station-route-layer',
        type: 'line',
        source: 'station-routes',
        layout: {
          'line-join': 'round',
          'line-cap': 'round'
        },
        paint: {
          'line-color': '#ff0000',
          'line-width': 6
        }
      },
      {
        id: 'station-route-element-layer',
        type: 'circle',
        source: 'station-routes',
        minzoom: 16,
        maxzoom: 24,
        paint: {
          'circle-color': '#ff0000',
          'circle-radius': ['interpolate', ['linear'], ['zoom'], 10, 1, 20, 19]
        }
      },
      {
        id: 'interlocking-route-layer',
        type: 'line',
        source: 'interlocking-routes',
        layout: {
          'line-join': 'round',
          'line-cap': 'round'
        },
        paint: {
          'line-color': '#ff0000',
          'line-width': 6
        }
      },
      {
        id: 'interlocking-route-element-layer',
        type: 'circle',
        source: 'interlocking-routes',
        minzoom: 16,
        maxzoom: 24,
        paint: {
          'circle-color': '#ff0000',
          'circle-radius': ['interpolate', ['linear'], ['zoom'], 10, 1, 20, 19]
        }
      }
    ]
  };
}

function getLineLayerSpec(
  displayMode: DisplayMode,
  current: LegendControls
): LayerSpecification {
  return {
    id: getLineLayer(displayMode),
    type: 'line',
    source: 'osm',
    'source-layer': getLineLayer(displayMode),
    paint: {
      'line-color': '#444',
      'line-width': 2.0
    },
    layout: {
      visibility: getVisibilityForMode(displayMode, current)
    }
  };
}

function getStationLayer(current: LegendControls): LayerSpecification {
  return {
    id: stationLayer,
    source: 'osm',
    'source-layer': 'station',
    type: 'symbol',
    minzoom: 5,
    maxzoom: 24,
    paint: {
      'icon-color': '#ffffff',
      'text-halo-width': 1,
      'text-halo-color': '#ffffff'
    },
    layout: {
      visibility: current.showStationIcons ? 'visible' : 'none',
      'text-field': ['get', 'name'],
      'text-anchor': 'top',
      'text-offset': [0, 1],
      'text-font': ['Noto Sans Bold'],
      'icon-image': 'icon-station',
      'icon-size': ['interpolate', ['linear'], ['zoom'], 10, 0.8, 20, 1.0]
    }
  };
}

function getDotLayer(
  elementType: ElementType,
  displayMode: DisplayMode,
  current: LegendControls
): LayerSpecification {
  return {
    id: getLayer(IconMode.Dot, elementType, displayMode),
    source: 'osm',
    'source-layer': getSourceLayer(elementType, displayMode),
    type: 'circle',
    minzoom: 13,
    maxzoom: 24,
    paint: {
      'circle-radius': 3,
      'circle-color': '#000000',
      'circle-stroke-width': 1,
      'circle-stroke-color': '#FFFFFF'
    },
    layout: {
      visibility: getVisibility(elementType, displayMode, current)
    }
  };
}

function getIconLayer(
  elementType: ElementType,
  displayMode: DisplayMode,
  current: LegendControls
): LayerSpecification {
  return {
    id: getLayer(IconMode.Icon, elementType, displayMode),
    source: 'osm',
    'source-layer': getSourceLayer(elementType, displayMode),
    type: 'symbol',
    minzoom: 15,
    maxzoom: 24,
    paint: {
      'icon-color': '#ffffff',
      'text-color': '#000000'
    },
    layout: {
      visibility: getVisibility(elementType, displayMode, current),
      'text-field': ['get', 'id'],
      'text-anchor': 'top',
      'text-offset': [0, 1],
      'text-font': ['Noto Sans Regular'],
      'icon-image': 'icon-' + elementType,
      'icon-size': ['interpolate', ['linear'], ['zoom'], 10, 0.2, 20, 0.4]
    }
  };
}

export function getInfrastructureMapStyle(
  current: LegendControls
): StyleSpecification {
  const style = getBaseStyle();

  style.layers.push(getStationLayer(current));

  for (const displayMode of DisplayModes) {
    style.layers.push(getLineLayerSpec(displayMode, current));

    for (const elementType of ElementTypes) {
      // gives us the small black dots for node icon stand-ins
      style.layers.push(getDotLayer(elementType, displayMode, current));

      // gives us the nodes as icons
      style.layers.push(getIconLayer(elementType, displayMode, current));
    }
  }

  return style;
}
