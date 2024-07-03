<template>
  <div ref="container">
    <div ref="map" class="map infrastructure-map" />

    <infrastructure-legend
      ref="legend"
      class="infrastructure-legend"
      @legend-control-change="onLegendControlChange"
      @element-type-change="onElementTypeChange"
      @show-station-change="onShowStationChange"
      @mileage-direction-change="onMileageDirectionChange"
      @display-mode-change="onDisplayModeChange"
      @reset="onLegendReset"
    />
  </div>
</template>

<script lang="ts">
import { mapMutations, mapState } from 'vuex';
import { SidebarNamespace } from '@/stores/sidebar-store';
import {
  FilterSpecification,
  GeoJSONFeature,
  LngLatBounds,
  Map,
  MapMouseEvent,
  Popup,
  RequestParameters,
  ResourceTypeEnum,
  Source
} from 'maplibre-gl';
import { getInfrastructureMapStyle } from './map-style';
import { addIcons } from './add-icons';
import { ElementType } from './element-types';
import { defineComponent } from 'vue';
import { useTheme } from 'vuetify';
import InfrastructureLegend, {
  LegendControls
} from '@/components/infrastructure/infrastructure-legend.vue';
import {
  getAllLayers,
  getLayersForType,
  getLayersForTypes,
  getLineLayer,
  hideLayers,
  IconLayers,
  isNodeLayer,
  isElementLayer,
  setLayersVisibilities,
  setLayerVisibility,
  showLayer,
  showLayers,
  stationLayer
} from '@/components/infrastructure/map-layers';
import { MileageDirection } from '@/components/infrastructure/mileage-direction';

function mapSourceHelper(
  source: Source | undefined,
  geoJson: GeoJSONFeature[]
) {
  if (!source) {
    return;
  }

  console.log('changing source', source);

  // illegal move
  // @ts-ignore
  const newData = source._data;
  newData.features = geoJson;

  // @ts-ignore
  source.setData(newData);
}

export default defineComponent({
  name: 'InfrastructureMap',

  components: { InfrastructureLegend },

  setup() {
    return { currentTheme: useTheme().global };
  },

  data(): {
    map?: Map;
    showMapTooltip: boolean;
  } {
    return {
      map: undefined,
      showMapTooltip: false
    };
  },

  computed: {
    ...mapState(SidebarNamespace, [
      'currentInfrastructure',
      'currentBoundingBox',
      'currentStation',
      'highlightedStationRoutes',
      'highlightedInterlockingRoutes'
    ])
  },

  watch: {
    currentInfrastructure(newInfrastructure: string | null) {
      if (this.map) {
        this.map.remove();
        this.map = undefined;
      }

      if (!newInfrastructure) {
        return;
      }

      // re-instantiating the map on infrastructure change leads
      // to duplicated icon fetching on change.
      this.createMap(newInfrastructure);
    },

    currentBoundingBox(boundingBox: LngLatBounds) {
      if (!this.map) {
        return;
      }

      this.map.fitBounds(boundingBox, { padding: 100 });
    },

    highlightedStationRoutes: {
      handler(highlightedRoutes: GeoJSONFeature[]) {
        if (!this.map) {
          return;
        }

        mapSourceHelper(
          this.map.getSource('station-routes'),
          highlightedRoutes
        );
      },
      deep: true
    },

    highlightedInterlockingRoutes: {
      handler(highlightedRoutes: GeoJSONFeature[]) {
        if (!this.map) {
          return;
        }

        mapSourceHelper(
          this.map.getSource('interlocking-routes'),
          highlightedRoutes
        );
      },
      deep: true
    },

    currentTheme: {
      handler() {
        if (!this.map) {
          return;
        }
      },
      deep: true
    }
  },

  mounted() {
    if (!this.currentInfrastructure) {
      return;
    }

    this.createMap(this.currentInfrastructure);
  },

  methods: {
    ...mapMutations(SidebarNamespace, ['setCurrentStation']),
    ...mapMutations(['setShowOverlay', 'setSelectedOverlay']),

    // applying all legend control settings to the map
    onLegendControlChange(current: LegendControls) {
      if (!this.map) return;
      // @ts-ignore
      hideLayers(this.map, getAllLayers());

      // @ts-ignore
      setLayerVisibility(this.map, stationLayer, current.showStationIcons);

      // @ts-ignore
      showLayer(this.map, getLineLayer(current.selectedDisplayMode));

      const layers = getLayersForTypes(
        current.selectedElementTypes,
        current.selectedDisplayMode
      );
      // @ts-ignore
      showLayers(this.map, layers);

      // const directionFilter: FilterSpecification = [
      //   'in',
      //   ['get', 'mileageDirection'],
      //   legendControls.selectedMileageDirections
      // ];
      //
      // nodeLayers.map((nodeLayer) => map.setFilter(nodeLayer, directionFilter));
      // elementLayers.map((nodeLayer) => map.setFilter(nodeLayer, directionFilter));
    },

    onElementTypeChange(
      newType: ElementType,
      checked: boolean,
      current: LegendControls
    ) {
      if (!this.map) return;
      const layers = getLayersForType(newType, current.selectedDisplayMode);
      // @ts-ignore
      setLayersVisibilities(this.map, layers, checked);
    },

    onShowStationChange(targetValue: string, checked: boolean) {
      if (!this.map) return;

      // @ts-ignore
      setLayerVisibility(this.map, stationLayer, checked);
    },

    onMileageDirectionChange(newDir: MileageDirection, checked: boolean) {
      if (!this.map) return;
      // setLayerVisibility(this.map, stationLayer, checked);
    },

    onDisplayModeChange(current: LegendControls) {
      if (!this.map) return;
      this.onLegendControlChange(current);
    },

    onLegendReset(currentLegendControls: LegendControls) {
      this.onLegendControlChange(currentLegendControls);
    },

    prettify(json: any) {
      let result = '';

      for (const key in json) {
        result += key + ': ' + json[key] + '<br>';
      }

      return result;
    },

    async createMap(infraName: string) {
      const transformUrl = (
        relativeUrl: string,
        resourceType?: ResourceTypeEnum
      ): RequestParameters => {
        if (!resourceType) {
          return { url: window.origin + relativeUrl };
        }

        switch (resourceType) {
          case 'Tile':
            return {
              url: window.origin + '/infrastructure/' + infraName + relativeUrl
            };
          case 'Image':
          case 'Glyphs':
            return { url: window.origin + relativeUrl };

          default:
            console.error(
              'requesting resource type',
              resourceType,
              'but no handler available.'
            );
            return { url: window.origin + relativeUrl };
        }
      };

      // @ts-ignore
      this.map = new Map({
        attributionControl: false,
        zoom: 10,
        hash: 'location',
        bearing: 0,
        container: this.$refs.map as HTMLElement,
        transformRequest: transformUrl,
        style: getInfrastructureMapStyle(
          // @ts-ignore
          this.$refs.legend.currentLegendControls
        )
      });

      this.map.on('load', async () => {
        if (!this.map) return;

        await addIcons(this.map as Map);

        // zoom to infrastructure
        const box = await this.$store.state.soroClient
          .infrastructure(infraName)
          .boundingBox();
        this.map.fitBounds(box.boundingBox);
      });

      this.map.dragPan.enable({
        linearity: 0.01,
        maxSpeed: 1400,
        deceleration: 2500
      });

      this.map.on('click', async (e: MapMouseEvent) => {
        const features = e.target.queryRenderedFeatures(e.point, {
          layers: IconLayers
        });

        if (features.length === 0) {
          return;
        }

        if (features.length > 1) {
          console.warn(
            'More than one feature found after click event! Using the first one.',
            features
          );
        }

        const clickedFeature = features[0];
        const clickedId = clickedFeature.properties.id;

        if (clickedFeature.sourceLayer === 'station') {
          this.setSelectedOverlay('station');
          this.setShowOverlay(true);
          this.setCurrentStation(clickedId);
        }

        if (isNodeLayer(clickedFeature.layer.id)) {
          const nodeJson = await this.$store.state.soroClient
            .infrastructure(infraName)
            .node(clickedId);

          new Popup()
            .setLngLat(e.lngLat)
            .setHTML(this.prettify(nodeJson))
            .addTo(this.map as Map);
        }

        if (isElementLayer(clickedFeature.layer.id)) {
          const nodeJson = await this.$store.state.soroClient
            .infrastructure(infraName)
            .element(clickedId);

          new Popup()
            .setLngLat(e.lngLat)
            .setHTML(this.prettify(nodeJson))
            .addTo(this.map as Map);
        }
      });
    }
  }
});
</script>

<style>
@import 'maplibre-gl/dist/maplibre-gl.css';

.infrastructure-map {
  padding: 0;
  margin: 0;
  position: absolute;
  height: 100%;
  width: 100%;
}

.infrastructure-legend {
  position: absolute;
  bottom: 0;
  right: 0;
  margin-right: 20px;
}
</style>
