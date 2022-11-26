<template>
  <div>
    <div class="map" id="map"></div>
    <div class="map-overlay" id="mapLegend">
    </div>

    <div class="infrastructureTooltip" id="infrastructureTooltip">
      <ul id="infrastructureTooltipList">
        <li id="kilometerPoint"></li>
        <li id="risingOrFalling"></li>
      </ul>
    </div>
  </div>
</template>

<script>
import { ClickTooltip } from "../../util/Tooltip.js";
import {
  createMap,
  deHighlightSignalStationRoute,
  deHighlightStationRoute,
  highlightSignalStationRoute,
  highlightStationRoute
} from "./infrastructure/map/infrastructureMap.js";

export default {
  name: "InfrastructureComponent",

  data() {
    return {
      container: undefined,
      componentState: undefined,
      _libreGLMap: undefined,
      _tooltip: undefined,
    }
  },

  methods: {
    setContainer(container) {
      this.container = container;
      this.container.on('resize', () => this._libreGLMap?.resize());
      this._tooltip = new ClickTooltip(container.element, 'infrastructureTooltip');
    },

    setComponentState(componentState) {
      this.componentState = componentState;
      this.changeInfrastructure(this.componentState.getCurrentInfrastructure());
    },

    changeInfrastructure(newInfrastructureName) {
      this._libreGLMap = newInfrastructureName
          ? createMap(this.container.element, newInfrastructureName, this._tooltip)
          : undefined;
    },

    highlightSignalStationRoute(signalStationRouteID) {
      highlightSignalStationRoute(this._libreGLMap, window.infrastructureManager.get(), signalStationRouteID);
    },

    deHighlightSignalStationRoute(signalStationRouteID) {
      deHighlightSignalStationRoute(this._libreGLMap, window.infrastructureManager.get(), signalStationRouteID);
    },

    highlightStationRoute(stationRouteID) {
      highlightStationRoute(this._libreGLMap, window.infrastructureManager.get(), stationRouteID);
    },

    deHighlightStationRoute(stationRouteID) {
      deHighlightStationRoute(this._libreGLMap, stationRouteID);
    },
  }
}
</script>

<style scoped>
#map {
  padding: 0;
  margin: 0;
  position: absolute;
  height: 100%;
  width: 100%;
}

#infrastructureTooltip {
  display: none;
  left: 0;
  top: 0;
  background: white;
  border: 2px;
  border-radius: 5px;
}

.map-overlay {
  position: absolute;
  bottom: 0;
  right: 0;
  background: var(--overlay-color);
  margin-right: 20px;
  font-family: var(--main-font-family);
  overflow: auto;
  border-radius: var(--border-radius)
}

#mapLegend {
  padding: 10px;
  box-shadow: 0 1px 2px rgba(0, 0, 0, 0.1);
  line-height: 18px;
  height: fit-content;
  margin-bottom: 40px;
  width: fit-content;
  background: var(--overlay-color);
}

.legend-key {
  height: 1em;
  margin-right: 5px;
  margin-left: 5px;
}

.legend-key-icon {
  margin-right: 7px;
  display: inline-block;
  height: 1em;
}
</style>

<style href="..e-gl.css" rel="stylesheet"/>
<style href="..re.css" rel="stylesheet"/>