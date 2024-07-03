import { Map } from 'maplibre-gl';
import {
  DisplayMode,
  DisplayModes
} from '@/components/infrastructure/display-modes';
import {
  ElementType,
  ElementTypes
} from '@/components/infrastructure/element-types';
import { IconMode, IconModes } from '@/components/infrastructure/icon-modes';

export function getLayer(
  iconMode: IconMode,
  elementType: ElementType,
  displayMode: DisplayMode
) {
  return iconMode + '-' + elementType + '-' + displayMode + '-layer';
}

function getDisplayMode(layer: string): DisplayMode {
  const lastDash = layer.lastIndexOf('-');
  const secondToLastDash = layer.lastIndexOf('-', lastDash - 1);
  return layer.substring(secondToLastDash + 1, lastDash) as DisplayMode;
}

export function isNodeLayer(layer: string) {
  return getDisplayMode(layer) === DisplayMode.Node;
}

export function isElementLayer(layer: string) {
  return getDisplayMode(layer) === DisplayMode.Element;
}

export const elementDotLayers = ElementTypes.map((elementType: ElementType) =>
  getLayer(IconMode.Dot, elementType, DisplayMode.Element)
);

export const elementIconLayers = ElementTypes.map((elementType: ElementType) =>
  getLayer(IconMode.Icon, elementType, DisplayMode.Element)
);

export const elementLayers = elementDotLayers.concat(elementIconLayers);

export const nodeDotLayers = ElementTypes.map((elementType: ElementType) =>
  getLayer(IconMode.Dot, elementType, DisplayMode.Node)
);

export const nodeIconLayers = ElementTypes.map((elementType: ElementType) =>
  getLayer(IconMode.Icon, elementType, DisplayMode.Node)
);

export const nodeLayers = nodeDotLayers.concat(nodeIconLayers);

export const stationLayer = 'station-layer';

export function* getIconLayers() {
  for (const elementType of ElementTypes) {
    for (const displayMode of DisplayModes) {
      yield getLayer(IconMode.Icon, elementType, displayMode);
    }
  }

  yield stationLayer;
}

export const IconLayers = [...getIconLayers()];

export function getLineLayer(displayMode: DisplayMode) {
  return displayMode + '-line-layer';
}

export function* getLineLayers() {
  for (const displayMode of DisplayModes) {
    yield getLineLayer(displayMode);
  }
}

export function* getLayersForType(
  elementType: ElementType,
  displayMode: DisplayMode
) {
  for (const iconMode of IconModes) {
    yield getLayer(iconMode, elementType, displayMode);
  }
}

export function* getLayersForTypes(
  elementTypes: Iterable<ElementType>,
  displayMode: DisplayMode
) {
  for (const iconMode of IconModes) {
    for (const elementType of elementTypes) {
      yield getLayer(iconMode, elementType, displayMode);
    }
  }
}

export function* getLayersForDisplayMode(displayMode: DisplayMode) {
  for (const elementType of ElementTypes) {
    for (const iconMode of IconModes) {
      yield getLayer(iconMode, elementType, displayMode);
    }
  }
}

export function* getAllLayers() {
  for (const iconMode of IconModes) {
    for (const elementType of ElementTypes) {
      for (const displayMode of DisplayModes) {
        yield getLayer(iconMode, elementType, displayMode);
      }
    }
  }

  yield stationLayer;

  for (const l of getLineLayers()) {
    yield l;
  }
}

export const AllLayers = [...getAllLayers()];

export function showLayer(map: Map, layer: string) {
  map.setLayoutProperty(layer, 'visibility', 'visible');
}

export function showLayers(map: Map, layers: Iterable<string>) {
  for (const layer of layers) {
    showLayer(map, layer);
  }
}

export function hideLayer(map: Map, layer: string) {
  map.setLayoutProperty(layer, 'visibility', 'none');
}

export function hideLayers(map: Map, layers: Iterable<string>) {
  for (const layer of layers) {
    hideLayer(map, layer);
  }
}

export function setLayerVisibility(map: Map, layer: string, visible: boolean) {
  map.setLayoutProperty(layer, 'visibility', visible ? 'visible' : 'none');
}

export function setLayersVisibilities(
  map: Map,
  layers: Iterable<string>,
  visible: boolean
) {
  for (const layer of layers) {
    setLayerVisibility(map, layer, visible);
  }
}

export function hideAllLayers(map: Map, displayMode: DisplayMode) {
  for (const layer of getLayersForDisplayMode(displayMode)) {
    hideLayer(map, layer);
  }
}

export function hideAllLayersExcept(map: Map, showDisplayMode: DisplayMode) {
  for (const displayMode of DisplayModes) {
    if (displayMode == showDisplayMode) continue;
    hideAllLayers(map, displayMode);
  }
}
