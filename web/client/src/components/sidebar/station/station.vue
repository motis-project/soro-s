<template>
  <div>
    <div v-if="id !== -1" id="stationDetail" class="station-detail hidden">
      <div class="station-detail-name">
        {{ ds100 }}
      </div>
      <div class="station-detail-routes">
        <soro-collapsible label="Station Routes">
          <template v-for="sr in stationRoutes" :key="sr.id">
            <soro-switch
              :internal-value="sr.id"
              :label="sr.name"
              :initially-checked="
                isInitiallyChecked(highlightedStationRoutes, sr.id)
              "
              :on-checked="addHighlightedStationRoute"
              :on-unchecked="deleteHighlightedStationRoute"
            />
          </template>
        </soro-collapsible>
      </div>
      <div class="station-detail-signal-routes">
        <soro-collapsible label="Interlocking Routes">
          <template v-for="irId in interlockingRouteIds" :key="irId">
            <soro-switch
              :internal-value="irId"
              :label="String(irId)"
              :initially-checked="
                isInitiallyChecked(highlightedInterlockingRoutes, irId)
              "
              :on-checked="addHighlightedInterlockingRoute"
              :on-unchecked="deleteHighlightedInterlockingRoute"
            />
          </template>
        </soro-collapsible>
      </div>
    </div>
  </div>
</template>

<script setup lang="ts">
import SoroCollapsible from '@/components/base/soro-collapsible.vue';
import SoroSwitch from '@/components/base/soro-switch.vue';
</script>

<script lang="ts">
import { mapActions, mapState } from 'vuex';
import { InfrastructureNamespace } from '@/stores/infrastructure-store';
import { defineComponent } from 'vue';
import { Station } from '@/util/SoroClient';
import { GeoJSONFeature } from 'maplibre-gl';

export default defineComponent({
  name: 'SidebarStation',

  data(): Station {
    return {
      id: -1,
      ds100: '',
      stationRoutes: [],
      interlockingRouteIds: []
    };
  },

  computed: {
    ...mapState(InfrastructureNamespace, [
      'currentInfrastructure',
      'currentStation',
      'highlightedStationRoutes',
      'highlightedInterlockingRoutes'
    ])
  },

  watch: {
    async currentStation(newStationId: number) {
      await this.fetchStation(newStationId);
    }
  },

  async mounted() {
    if (this.currentStation !== undefined) {
      await this.fetchStation(this.currentStation);
    }
  },

  methods: {
    ...mapActions(InfrastructureNamespace, [
      'addHighlightedStationRoute',
      'deleteHighlightedStationRoute',
      'addHighlightedInterlockingRoute',
      'deleteHighlightedInterlockingRoute'
    ]),

    async fetchStation(stationId: number) {
      const station = await this.$store.state.soroClient
        .infrastructure(this.currentInfrastructure)
        .station(stationId);

      this.id = station.id;
      this.ds100 = station.ds100;
      this.stationRoutes = station.stationRoutes;
      this.interlockingRouteIds = station.interlockingRouteIds;
    },

    isInitiallyChecked(array: GeoJSONFeature[], routeId: number) {
      return array.some((f: GeoJSONFeature) => f.properties.id === routeId);
    }
  }
});
</script>

<style>
.station-detail {
  margin-top: 10px;
  background: transparent;
  color: #777;
  width: 100%;
  height: 100%;
  box-shadow: 3px 3px 2px rgb(0 0 0 / 20%);
  cursor: pointer;
  display: flex;
  align-items: center;
  justify-content: center;
  opacity: 1;
  transition: all 0.2s ease;
  flex-direction: column;
  visibility: visible;
}

.station-detail.hidden {
  left: calc(0px - calc(var(--overlay-width) + var(--overlay-padding-left)));
}

.station-detail-name {
  font-family: var(--main-font-family);
  font-weight: bold;
  font-size: 20px;
  justify-content: center;
  align-items: center;
  color: var(--text-color);
  flex-direction: column;
}

.station-detail-routes {
  display: flex;
  flex-direction: column;
  width: 100%;
  height: 90%;
}

.station-detail-route {
  width: calc(100% - 1em);
  background: var(--dialog-color);
  border: 0.4em solid var(--dialog-color);
}

.station-detail-route:nth-child(even) {
  background: var(--overlay-color);
  border: 0.4em solid var(--overlay-color);
  border-right: 0;
}

.station-detail-signal-routes {
  display: flex;
  flex-direction: column;
  width: 100%;
  height: 90%;
}

.station-detail-signal-route {
  width: calc(100% - 1em);
  background: var(--dialog-color);
  border: 0.4em solid var(--dialog-color);
}

.station-detail-signal-route:nth-child(even) {
  background: var(--overlay-color);
  border: 0.4em solid var(--overlay-color);
  border-right: 0;
}
</style>
