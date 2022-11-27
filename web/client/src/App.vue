<template>
  <div class="full-height">
    <div :class="overlayContainerClasses" ref="overlayContainer">
      <div class="overlay">
        <div class="overlay-content">
          <div class="window-controls">
            <button class="matter-button-contained window-button" @click="addInfrastructureTab">
              Infrastructure
            </button>
            <button class="matter-button-contained window-button" @click="addSimulationTab">
              Simulation
            </button>
            <button class="matter-button-contained window-button" @click="addTimetableTab">
              Timetable
            </button>
          </div>
          <div class="data-selects">
            <soro-select
                label="Select Infrastructure"
                :value="currentInfrastructure"
                :options="infrastructures"
                @select="loadInfrastructure"
            />
            <soro-select
                label="Select Timetable"
                :value="currentTimetable"
                :options="timetables"
                @select="loadTimetable"
            />
          </div>
          <div class="dev-tools">
            <button class="matter-button-contained window-button" @click="deleteAllFiles">Clear Cache</button>
            <button class="matter-button-contained window-button" @click="triggerSimulation">Simulate</button>
          </div>
        </div>
        <div :class="subOverlayClasses" ref="subOverlay">
          <div class="sub-overlay-content" id="subOverlayContent">
            <disruption ref="disruption" />
          </div>
          <div class="sub-overlay-close" ref="subOverlayClose" @click="onSubOverlayCloseClicked">
            <i class="material-icons">close</i>
          </div>
        </div>
      </div>
      <div class="overlay-tabs" ref="subOverlayTabs">
        <div class="overlay-toggle">
          <button class="matter-button-contained overlay-toggle-button" @click="toggleOverlay">
            <i class="material-icons" style="display: flex; justify-content:center;">menu</i>
          </button>
        </div>
        <div class="sub-overlay-tab-button" ref="stationDetailButton" @click="onStationDetailClicked">
          <i class="material-icons">home</i>
        </div>
        <div class="sub-overlay-tab-button" ref="disruptionDetailButton" @click="onDisruptionDetailClicked">
          <i class="material-icons">train</i>
        </div>
      </div>
    </div>
    <glayout
        ref="GLayoutRoot"
        style="width: 100%; height: calc(100% - 90px)"
    ></glayout>
  </div>
</template>

<script setup>
  // TODO make this TS
  // import { Module, FS, IDBFS } from "./soro-client.js";
  import Glayout from './golden-layout/Glayout.vue';
  import Disruption from "./components/disruption.vue";
  import SoroSelect from "./components/select/soro-select.vue";

  // import {
  //   saveToPersistent,
  //   loadFromPersistent,
  //   exists,
  //   saveFileToIDBFS,
  //   createFolders,
  //   deleteAllFiles,
  //   getStationCoordPath
  // } from "./utl/IDBFSHelper.js";

  // function mountIDBFS(callback) {
  //   FS.mkdir('/idbfs');
  //   FS.mount(IDBFS, {}, '/idbfs');
  //
  //   loadFromPersistent(function () {
  //     createFolders();
  //     callback();
  //     saveToPersistent();
  //   });
  // }

  /*
    Run the setup routines after emscripten has done all of its own initialization
   */
  // Module.onRuntimeInitialized = async function () {
  //   mountIDBFS(() => {
  //
  //     fetch(window.origin + '/misc/btrs_geo.csv')
  //       .then(response => response.arrayBuffer())
  //       .then(buf => saveFileToIDBFS('btrs_geo.csv', buf))
  //       .catch(e => console.error(e));
  //   })
  //
  // }
</script>

<script>
import { mapActions, mapState } from 'vuex';
import {InfrastructureNameSpace} from "./stores/infrastructure-store.js";
import {TimetableNamespace} from "./stores/timetable-store.js";

const initLayout = {
  root: {
    type: 'row',
    content: [
      {
        type: 'column',
        content: [
          {
            title: 'Infrastructure',
            type: 'component',
            componentType: 'InfrastructureComponent',
          },
          {
            title: 'Simulation',
            type: 'component',
            componentType: 'SimulationComponent',
          },
        ]
      }
    ]
  }
  /* If these are not shown it is not possible to rearrange GoldenLayout windows.
     Instead of disabling them the images in goldenlayout-mdl-theme.css are removed
     and all click events disabled ..
  settings: {
    showPopoutIcon: false,
    showMaximiseIcon: false,
    showCloseIcon: false
  }
  */
};

export default {
  data() {
    return {
      overlay: false,
      subOverlay: false,
    }
  },

  mounted() {
    this.loadInfrastructures();
    this.loadTimetables();
    this.$refs.GLayoutRoot.loadGLLayout(initLayout);
  },

  computed: {
    overlayContainerClasses() {
      return `overlay-container ${this.overlay ? '' : 'hidden'}`
    },

    subOverlayClasses() {
      return `sub-overlay ${this.subOverlay ? '' : 'hidden'}`
    },

    ...mapState(InfrastructureNameSpace, [
        'currentInfrastructure',
        'infrastructures',
    ]),
    ...mapState(TimetableNamespace, [
        'currentTimetable',
        'timetables',
    ]),
  },

  methods: {
    toggleOverlay() {
      this.overlay = !this.overlay;
    },

    showSubOverlay() {
      this.overlay = true;
      this.subOverlay = true;
    },

    hideSubOverlay() {
      this.subOverlay = false;
    },

    toggleSubOverlay() {
      if (!this.subOverlay) {
        this.showSubOverlay();
      } else {
        this.hideSubOverlay();
      }
    },

    onStationDetailClicked() {
      this.$refs.stationDetailButton.classList.toggle('enabled');
      this.showSubOverlay();
    },

    onDisruptionDetailClicked() {
      this.$refs.disruptionDetailButton.classList.add('enabled');
      this.$refs.disruption.showDetail();
      this.showSubOverlay();
    },

    onSubOverlayCloseClicked() {
      this.hideSubOverlay();

      for (let overlayTab of this.$refs.subOverlayTabs.children) {
        overlayTab.classList.remove('enabled');
      }
    },

    addInfrastructureTab() {
      this.$refs.GLayoutRoot.addGLComponent('InfrastructureComponent', 'Infra');
    },

    addSimulationTab() {
      this.$refs.GLayoutRoot.addGLComponent('SimulationComponent', 'Simulation');
    },

    addTimetableTab() {
      this.$refs.GLayoutRoot.addGLComponent('TimetableComponent', 'Timetable');
    },

    triggerSimulation() {
      // TODO trigger simulation in store (has to move first)
    },

    ...mapActions(InfrastructureNameSpace, {
      loadInfrastructures: 'initialLoad',
      loadInfrastructure: 'load',
    }),

    ...mapActions(TimetableNamespace, {
      loadTimetables: 'initialLoad',
      loadTimetable: 'load',
    }),
  },
}
</script>

<style>
html {
  height: 100%;
}
body {
  height: 100%;
  margin: 0;
  overflow: hidden;
}
.full-height, #app {
  height: 100%;
}
#app {
  font-family: Avenir, Helvetica, Arial, sans-serif;
  -webkit-font-smoothing: antialiased;
  -moz-osx-font-smoothing: grayscale;
}
</style>
