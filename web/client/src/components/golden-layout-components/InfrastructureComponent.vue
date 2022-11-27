<template>
  <div ref="mapContainer">
    <div class="map" id="map" />
    <div class="map-overlay" id="mapLegend" />

    <div class="infrastructureTooltip" id="infrastructureTooltip">
      <ul id="infrastructureTooltipList">
        <li id="kilometerPoint" />
        <li id="risingOrFalling" />
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
import { mapState } from 'vuex';
import { InfrastructureNameSpace } from "../../stores/infrastructure-store.js"; // TODO rewrite with global namespace
import { defineComponent } from "vue";
import { ComponentContainer } from "golden-layout";

export default defineComponent({
  name: "InfrastructureComponent",

  props: {
    container: {
      type: ComponentContainer,
      required: false,
      default: undefined,
    },
  },

  data() {
    return {
      _libreGLMap: undefined,
      _tooltip: undefined,
    }
  },

  computed: mapState(InfrastructureNameSpace, [
      'currentInfrastructure',
      'highlightedSignalStationRouteID',
      'highlightedStationRouteID',
  ]),

  created() { this.configureContainer() },

  watch: {
    currentInfrastructure(newInfrastructure) {
      this._libreGLMap = newInfrastructure
          ? createMap(this.$refs.mapContainer, newInfrastructure, this._tooltip)
          : undefined;
    },

    highlightedSignalStationRouteID(newID, oldID) {
      if (newID) {
        highlightSignalStationRoute(this._libreGLMap, this.currentInfrastructure, newID);
      } else {
        deHighlightSignalStationRoute(this._libreGLMap, this.currentInfrastructure, oldID);
      }
    },

    highlightedStationRouteID(newID, oldID) {
      if (newID) {
        highlightStationRoute(this._libreGLMap, this.currentInfrastructure, newID);
      } else {
        deHighlightStationRoute(this._libreGLMap, oldID);
      }
    },
  },

  methods: {
    configureContainer() {
      if (!this.container)
        return

      this.container.on('resize', () => this._libreGLMap?.resize());
      this._tooltip = new ClickTooltip(this.container.element, 'infrastructureTooltip');
    }
  }
})
</script>

<style>
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