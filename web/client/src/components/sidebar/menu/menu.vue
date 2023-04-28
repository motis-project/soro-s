<template>
  <div>
    <div class="overlay-content">
      <div class="window-controls">
        <soro-button
          label="New Infrastructure Tab"
          :disabled="!currentInfrastructure"
          @click="addTab(ComponentTechnicalNames.INFRASTRUCTURE)"
        />
        <soro-button
          label="New Timetable Tab"
          :disabled="!currentTimetable"
          @click="addTab(ComponentTechnicalNames.TIMETABLE)"
        />
        <soro-button
          label="New OrderingGraph Tab"
          :disabled="!currentInfrastructure || !currentTimetable"
          @click="addTab(ComponentTechnicalNames.ORDERING_GRAPH)"
        />
        <soro-button
          label="New Simulation Tab"
          :disabled="!currentInfrastructure || !currentTimetable || true"
          @click="addTab(ComponentTechnicalNames.SIMULATION)"
        />
      </div>

      <div class="data-selects">
        <soro-select
          class="data-select"
          label="Select Infrastructure"
          :value="currentInfrastructure"
          :options="infrastructures"
          @select="loadInfrastructure"
        />
        <soro-select
          class="data-select"
          label="Select Timetable"
          :value="currentTimetable"
          :options="timetables"
          :disabled="!currentInfrastructure"
          @select="loadTimetable"
        />
      </div>

      <div class="date-range-select">
        <v-card-title class="text-subtitle-1">Select Time Range:</v-card-title>
        <soro-date-time-range
          :watch-value="currentDateRange"
          :disabled="!currentInfrastructure || !currentTimetable"
          @change="setCurrentDateRange"
        ></soro-date-time-range>
      </div>

      <search
        class="search-field"
        @change-to-extended="changeToSearchOverlay"
      />

      <menu-settings class="settings" />

      <soro-collapsible label="Dev Tools" class="dev-tools">
        <div>
          <soro-id-input label="Train IDs filter" @enter="setTrainIdsFilter" />
        </div>
        <soro-button label="Clear local storage" @click="clearLocalStorage" />
        <soro-button disabled label="Clear Cache" />
        <soro-button disabled label="Simulate" />
      </soro-collapsible>
    </div>
  </div>
</template>

<script setup lang="ts">
import SoroSelect from '@/components/base/soro-select.vue';
import SoroButton from '@/components/base/soro-button.vue';
import SoroCollapsible from '@/components/base/soro-collapsible.vue';
import SoroDateTimeRange from '@/components/base/soro-datetimerange.vue';
import SoroIdInput from '@/components/base/soro-id-input.vue';
import Search from '@/components/sidebar/search/search.vue';
import MenuSettings from '@/components/sidebar/menu/settings.vue';
</script>

<script lang="ts">
import { defineComponent } from 'vue';
import { mapActions, mapState } from 'vuex';
import { SidebarNamespace } from '@/stores/sidebar-store';
import { ComponentTechnicalName } from '@/golden-layout/golden-layout-constants';
import { GoldenLayoutNamespace } from '@/stores/golden-layout-store';

export default defineComponent({
  name: 'SidebarMenu',
  emits: ['change-overlay'],

  data() {
    return {
      ComponentTechnicalNames: ComponentTechnicalName
    };
  },

  computed: {
    ...mapState(SidebarNamespace, [
      'infrastructures',
      'currentInfrastructure',
      'timetables',
      'currentTimetable',
      'currentDateRange',
      'trainIdsFilter'
    ])
  },

  methods: {
    addTab(componentTechnicalName: ComponentTechnicalName) {
      this.addGoldenLayoutTab({ componentTechnicalName });
    },

    clearLocalStorage() {
      window.localStorage.clear();
    },

    changeToSearchOverlay() {
      this.$emit('change-overlay', 'extended-search.vue');
    },

    ...mapActions(SidebarNamespace, [
      'loadInfrastructure',
      'loadTimetable',
      'setCurrentDateRange',
      'setTrainIdsFilter'
    ]),
    ...mapActions(GoldenLayoutNamespace, ['addGoldenLayoutTab'])
  }
});
</script>

<style scoped>
.overlay-content {
  display: flex;
  flex-direction: column;
  pointer-events: auto;
  position: relative;
}

.window-controls {
  display: flex;
  flex-flow: column wrap;
  justify-content: space-around;
  padding: 3%;
  margin-top: 0.5em;
  margin-bottom: 0.5em;
}

.window-controls > .soro-button {
  margin-top: 0.2em;
  margin-bottom: 0.2em;
}

.data-selects {
  display: flex;
  flex-flow: column wrap;
  justify-content: space-around;
  padding: 3%;
  margin-top: 0.5em;
  margin-bottom: 0.5em;
}

.date-range-select {
  display: flex;
  flex-flow: column wrap;
  justify-content: space-around;
  padding: 3%;
  margin-top: 0.5em;
  margin-bottom: 0.5em;
}

.search-field {
  padding: 3%;
}

.settings,
.dev-tools {
  padding: 3%;
}

.dev-tools .soro-button {
  margin-top: 0.2em;
  margin-bottom: 0.2em;
}
</style>
