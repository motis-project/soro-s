<template>
  <div class="infrastructure-legend">
    <v-btn
      class="infrastructure-legend-collapse-button"
      color="primary"
      @click="() => (isExpanded = !isExpanded)"
    >
      <i class="material-icons">{{ collapseIcon }}</i>
    </v-btn>
    <v-sheet
      ref="mapLegend"
      class="infrastructure-map-legend"
      :class="isExpanded ? '' : 'infrastructure-map-legend-collapsed'"
      :elevation="5"
    >
      <div class="infrastructure-map-reset">
        <a href="/" @click.prevent="emitReset()"> Reset </a>
      </div>

      <template v-for="(elementType, index) in ElementTypes()" :key="index">
        <v-checkbox
          :ref="elementType"
          v-model="currentLegendControls.selectedElementTypes"
          :name="elementType"
          :value="elementType"
          class="legend-key"
          color="primary"
          density="compact"
          min-height="0px"
          hide-details
          @input="emitElementTypeChange"
        >
          <template #label>
            <img
              class="legend-key-icon"
              :src="iconUrl() + elementType + iconExtension()"
              alt=""
            />
            {{ ElementTypeLabel[elementType] }}
          </template>
        </v-checkbox>
      </template>

      <v-checkbox
        ref="Station"
        v-model="currentLegendControls.showStationIcons"
        name="Station"
        class="legend-key"
        color="primary"
        density="compact"
        min-height="0px"
        hide-details
        @input="emitShowStationsChange"
      >
        <template #label>
          <img
            class="legend-key-icon"
            :src="iconUrl() + 'station' + iconExtension()"
            alt=""
          />
          {{ 'Station' }}
        </template>
      </v-checkbox>

      <template v-for="(mileageDir, index) in MileageDirections()" :key="index">
        <v-checkbox
          :ref="mileageDir"
          v-model="currentLegendControls.selectedMileageDirections"
          :name="mileageDir"
          :value="mileageDir"
          class="legend-key"
          color="primary"
          density="compact"
          min-height="0px"
          hide-details
          @input="emitMileageDirectionChange"
        >
          <template #label>
            {{ MileageDirectionLabel[mileageDir] }}
          </template>
        </v-checkbox>
      </template>

      <div>
        <v-radio-group
          v-model="currentLegendControls.selectedDisplayMode"
          inline
          label="Display Mode"
          @input="emitDisplayModeChange"
        >
          <template v-for="(displayMode, index) in DisplayModes()" :key="index">
            <v-radio
              :label="DisplayModeLabel[displayMode]"
              :value="displayMode"
            ></v-radio>
          </template>
        </v-radio-group>
      </div>
    </v-sheet>
  </div>
</template>

<script lang="ts">
import {
  ElementType,
  ElementTypeLabel,
  ElementTypes
} from '@/components/infrastructure/element-types.js';
import {
  DisplayMode,
  DisplayModeLabel,
  DisplayModes
} from '@/components/infrastructure/display-modes.js';
import { iconExtension, iconUrl } from './add-icons';
import { defineComponent } from 'vue';
import {
  MileageDirection,
  MileageDirectionLabel,
  MileageDirections
} from '@/components/infrastructure/mileage-direction';

export type LegendControls = {
  selectedElementTypes: ElementType[];
  showStationIcons: boolean;
  selectedMileageDirections: MileageDirection[];
  selectedDisplayMode: DisplayMode;
};

const initialLegendControls: LegendControls = {
  selectedElementTypes: [
    ElementType.Halt,
    ElementType.MainSignal,
    ElementType.ApproachSignal,
    ElementType.EndOfTrainDetector
  ],

  showStationIcons: true,

  selectedMileageDirections: [
    MileageDirection.Falling,
    MileageDirection.Rising,
    MileageDirection.Undirected
  ],

  selectedDisplayMode: DisplayMode.Element
};

export default defineComponent({
  name: 'InfrastructureLegend',

  inject: {
    goldenLayoutKeyInjection: {
      default: ''
    }
  },

  emits: [
    'legendControlChange',
    'elementTypeChange',
    'showStationChange',
    'mileageDirectionChange',
    'displayModeChange',
    'reset'
  ],

  data(): {
    isExpanded: boolean;
    currentLegendControls: LegendControls;
  } {
    return {
      isExpanded: false,
      currentLegendControls: initialLegendControls
    };
  },

  computed: {
    MileageDirectionLabel() {
      return MileageDirectionLabel;
    },

    legendStateLocalStorageKey() {
      return `infrastructure[${this.goldenLayoutKeyInjection}].legendState`;
    },

    ElementTypeLabel() {
      return ElementTypeLabel;
    },

    DisplayModeLabel() {
      return DisplayModeLabel;
    },

    collapseIcon() {
      return this.isExpanded ? 'chevron_right' : 'legend_toggle';
    }
  },

  created() {
    const storedLegendState = window.localStorage.getItem(
      this.legendStateLocalStorageKey
    );

    if (storedLegendState) {
      this.currentLegendControls = JSON.parse(storedLegendState);
    }
  },

  mounted() {
    this.$emit('legendControlChange', this.currentLegendControls);
  },

  methods: {
    ElementTypes() {
      return ElementTypes;
    },

    MileageDirections() {
      return MileageDirections;
    },

    iconExtension() {
      return iconExtension;
    },

    iconUrl() {
      return iconUrl;
    },

    DisplayModes() {
      return DisplayModes;
    },

    saveControls() {
      window.localStorage.setItem(
        this.legendStateLocalStorageKey,
        JSON.stringify(this.currentLegendControls)
      );
    },

    emit(s: any, t: HTMLInputElement) {
      this.$emit(s, t.value, t.checked, this.currentLegendControls);
    },

    emitElementTypeChange(event: Event) {
      this.saveControls();
      this.emit('elementTypeChange', event.target as HTMLInputElement);
    },

    emitShowStationsChange(event: Event) {
      this.saveControls();
      this.emit('showStationChange', event.target as HTMLInputElement);
    },

    emitMileageDirectionChange(event: Event) {
      this.saveControls();
      this.emit('mileageDirectionChange', event.target as HTMLInputElement);
    },

    emitDisplayModeChange() {
      this.saveControls();
      this.$emit('displayModeChange', this.currentLegendControls);
    },

    emitReset() {
      Object.assign(this.$data.currentLegendControls, initialLegendControls);
      this.saveControls();
      this.$emit('reset', this.currentLegendControls);
    }
  }
});
</script>

<style scoped>
.infrastructure-legend {
  display: flex;
  margin-bottom: 40px;
}

.infrastructure-legend-collapse-button {
  align-self: end;
}

.infrastructure-map-legend {
  padding: 10px 10px 20px;
  height: fit-content;
  width: fit-content;
}

.infrastructure-map-legend-collapsed {
  display: none;
}

.infrastructure-map-reset {
  display: flex;
  flex-direction: row;
  justify-content: end;
  width: 100%;
  color: rgb(var(--v-theme-primary));
}

a,
a:visited,
a:hover,
a:active {
  color: inherit;
}

.legend-key {
  height: 1.5em;
  margin-right: 5px;
  margin-left: 5px;
}

.legend-key-icon {
  margin-right: 7px;
  margin-left: 7px;
  display: inline-block;
  height: 1em;
}
</style>
