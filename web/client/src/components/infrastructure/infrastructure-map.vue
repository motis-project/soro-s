<template>
  <div ref="container">
    <div ref="map" class="map infrastructure-map" />
    <infrastructure-legend
      class="map-overlay"
      :checked-controls="checkedControls"
      @change="onLegendControlChanged"
      @reset="resetLegend"
    />
    <div
      ref="infrastructureTooltip"
      class="infrastructureTooltip infrastructure-tooltip"
    >
      <ul id="infrastructureTooltipList">
        <li id="kilometerPoint" />
        <li id="risingOrFalling" />
      </ul>
    </div>
  </div>
</template>

<script lang="ts">
import { mapMutations, mapState } from 'vuex';
import { InfrastructureNamespace } from '@/stores/infrastructure-store';
import {
  FilterSpecification,
  Map,
  MapMouseEvent,
  ResourceTypeEnum,
  RequestParameters,
  LngLatBounds,
  GeoJSONFeature,
  Source
} from 'maplibre-gl';
import { createInfrastructureMapStyle, mapLayers } from './map-style';
import { addIcons } from './add-icons';
import { ElementTypes, ElementType } from './element-types';
import { defineComponent } from 'vue';
import { ThemeInstance, useTheme } from 'vuetify';
import {
  SpecialLegendControls,
  SpecialLegendControl
} from '@/components/infrastructure/infrastructure-legend.vue';
import InfrastructureLegend from '@/components/infrastructure/infrastructure-legend.vue';

export const initiallyCheckedControls = [
  ElementType.STATION,
  ElementType.HALT,
  ElementType.MAIN_SIGNAL,
  ElementType.APPROACH_SIGNAL,
  ElementType.END_OF_TRAIN_DETECTOR,
  ...SpecialLegendControls
];

function mapSourceHelper(
  source: Source | undefined,
  geoJson: GeoJSONFeature[]
) {
  if (!source) {
    return;
  }

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

  inject: {
    goldenLayoutKeyInjection: {
      default: ''
    }
  },

  setup() {
    return { currentTheme: useTheme().global };
  },

  data(): {
    libreGLMap?: Map;
    checkedControls: typeof initiallyCheckedControls;
  } {
    return {
      libreGLMap: undefined,
      checkedControls: Array.from(initiallyCheckedControls)
    };
  },

  computed: {
    checkedLegendControlLocalStorageKey() {
      return `infrastructure[${this.goldenLayoutKeyInjection}].checkedControls`;
    },

    ...mapState(InfrastructureNamespace, [
      'currentInfrastructure',
      'currentBoundingBox',
      'currentStation',
      'highlightedStationRoutes',
      'highlightedInterlockingRoutes'
    ])
  },

  watch: {
    currentInfrastructure(newInfrastructure: string | null) {
      if (this.libreGLMap) {
        this.libreGLMap.remove();
        this.libreGLMap = undefined;
      }

      if (!newInfrastructure) {
        return;
      }

      // re-instantiating the map on infrastructure change leads
      // to duplicated icon fetching on change.
      this.createMap(newInfrastructure);
    },

    currentBoundingBox(boundingBox: LngLatBounds) {
      if (!this.libreGLMap) {
        return;
      }

      this.libreGLMap.fitBounds(boundingBox, { padding: 100 });
    },

    highlightedStationRoutes: {
      handler(highlightedRoutes: GeoJSONFeature[]) {
        if (!this.libreGLMap) {
          return;
        }

        mapSourceHelper(
          this.libreGLMap.getSource('station-routes'),
          highlightedRoutes
        );
      },
      deep: true
    },

    highlightedInterlockingRoutes: {
      handler(highlightedRoutes: GeoJSONFeature[]) {
        if (!this.libreGLMap) {
          return;
        }

        mapSourceHelper(
          this.libreGLMap.getSource('interlocking-routes'),
          highlightedRoutes
        );
      },
      deep: true
    },

    currentTheme: {
      handler(newTheme: ThemeInstance) {
        if (!this.libreGLMap) {
          return;
        }

        this.libreGLMap.setStyle(
          createInfrastructureMapStyle({
            currentTheme: newTheme.current.value,
            activatedElements: this.checkedControls
          })
        );
      },
      deep: true
    }
  },

  created() {
    const checkedControlsString = window.localStorage.getItem(
      this.checkedLegendControlLocalStorageKey
    );
    if (checkedControlsString) {
      this.checkedControls = JSON.parse(checkedControlsString);
    }
  },

  mounted() {
    if (!this.currentInfrastructure) {
      return;
    }

    this.createMap(this.currentInfrastructure);
  },

  methods: {
    ...mapMutations(InfrastructureNamespace, ['setCurrentStation']),
    ...mapMutations(['setShowOverlay', 'setSelectedOverlay']),

    onLegendControlChanged(legendControl: string, checked: boolean) {
      if (checked) {
        this.checkedControls.push(legendControl);
      } else {
        this.checkedControls = this.checkedControls.filter(
          (control) => control !== legendControl
        );
      }

      this.saveControls();

      if (!this.libreGLMap) {
        return;
      }

      if (SpecialLegendControls.includes(legendControl)) {
        this.evaluateSpecialLegendControls();

        return;
      }

      this.setElementTypeVisibility(legendControl, checked);
    },

    resetLegend() {
      this.checkedControls = initiallyCheckedControls;
      this.saveControls();
      this.setVisibilityOfAllControls();
    },

    saveControls() {
      window.localStorage.setItem(
        this.checkedLegendControlLocalStorageKey,
        JSON.stringify(this.checkedControls)
      );
    },

    setVisibilityOfAllControls() {
      ElementTypes.forEach((type) =>
        this.setElementTypeVisibility(type, this.checkedControls.includes(type))
      );
      this.evaluateSpecialLegendControls();
    },

    setElementTypeVisibility(elementType: string, visible: boolean) {
      if (elementType !== ElementType.STATION) {
        this.libreGLMap?.setLayoutProperty(
          `circle-${elementType}-layer`,
          'visibility',
          visible ? 'visible' : 'none'
        );
      }

      this.libreGLMap?.setLayoutProperty(
        `${elementType}-layer`,
        'visibility',
        visible ? 'visible' : 'none'
      );
    },

    evaluateSpecialLegendControls() {
      const risingChecked = this.checkedControls.includes(
        SpecialLegendControl.RISING
      );
      const fallingChecked = this.checkedControls.includes(
        SpecialLegendControl.FALLING
      );

      let filter: FilterSpecification;
      if (!risingChecked && fallingChecked) {
        filter = ['!', ['get', 'rising']];
      } else if (risingChecked && !fallingChecked) {
        filter = ['get', 'rising'];
      } else if (!risingChecked && !fallingChecked) {
        filter = ['boolean', false];
      }

      ElementTypes.forEach((elementType) => {
        if (elementType === ElementType.STATION) {
          return;
        }

        this.libreGLMap?.setFilter(elementType + '-layer', filter);
        this.libreGLMap?.setFilter('circle-' + elementType + '-layer', filter);
      });
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
      this.libreGLMap = new Map({
        attributionControl: false,
        zoom: 10,
        hash: 'location',
        bearing: 0,
        container: this.$refs.map as HTMLElement,
        transformRequest: transformUrl,
        style: createInfrastructureMapStyle({
          currentTheme: this.$vuetify.theme.current,
          activatedElements: this.checkedControls
        })
      });

      this.libreGLMap.on('load', async () => {
        if (!this.libreGLMap) {
          return;
        }

        await addIcons(this.libreGLMap as Map);
        this.setVisibilityOfAllControls();

        // zoom to infrastructure
        const box = await this.$store.state.soroClient
          .infrastructure(infraName)
          .boundingBox();
        this.libreGLMap.fitBounds(box.boundingBox);
      });

      this.libreGLMap.dragPan.enable({
        linearity: 0.01,
        maxSpeed: 1400,
        deceleration: 2500
      });

      this.libreGLMap.on('click', (e: MapMouseEvent) => {
        const features = e.target.queryRenderedFeatures(e.point, {
          layers: mapLayers
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
        } else {
          // this.setSelectedOverlay('element');
          // this.setShowOverlay(true);
          // this.setCurrentElements(features);
          console.log('Clicked on element', clickedFeature.properties.id);
        }
      });
    }
  }
});
</script>

<style>
.infrastructure-map {
  padding: 0;
  margin: 0;
  position: absolute;
  height: 100%;
  width: 100%;
}

.infrastructure-tooltip {
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
  margin-right: 20px;
}
</style>

<style href="..e-gl.css" rel="stylesheet" />
<style href="..re.css" rel="stylesheet" />
