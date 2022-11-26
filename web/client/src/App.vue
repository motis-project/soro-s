<template>
  <div>
    <div class="overlay-container hidden" id="overlayContainer">
      <div class="overlay">
        <div class="overlay-content">
          <div class="window-controls">
            <button class="matter-button-contained window-button"
                    id="addInfrastructureComponentButton">
              Infrastructure
            </button>
            <button class="matter-button-contained window-button"
                    id="addSimulationComponentButton">Simulation
            </button>
            <button class="matter-button-contained window-button"
                    id="addTimetableComponentButton">Timetable
            </button>
          </div>
          <div class="data-selects">
            <div class="material-select data-select">
              <select class="material-select-text"
                      id="infrastructureSelect" required>
                <option selected value=""></option>
              </select>
              <span class="material-select-highlight"></span>
              <span class="material-select-bar"></span>
              <label class="material-select-label">Select
                Infrastructure</label>
            </div>
            <div class="material-select data-select">
              <select class="material-select-text" id="timetableSelect"
                      required>
                <option selected value=""></option>
              </select>
              <span class="material-select-highlight"></span>
              <span class="material-select-bar"></span>
              <label class="material-select-label">Select
                Timetable</label>
            </div>
          </div>
          <div class="dev-tools">
            <button class="matter-button-contained window-button"
                    id="clearCacheButton">Clear
              Cache
            </button>
            <button class="matter-button-contained window-button"
                    id="simulateButton">Simulate
            </button>
          </div>
        </div>
        <div class="sub-overlay hidden" id="subOverlay">
          <div class="sub-overlay-content" id="subOverlayContent">
          </div>
          <div class="sub-overlay-close" id="subOverlayClose">
            <i class="material-icons">close</i>
          </div>
        </div>
      </div>
      <div class="overlay-tabs" id="subOverlayTabs">
        <div class="overlay-toggle">
          <button class="matter-button-contained overlay-toggle-button"
                  id="overlayToggleButton">
            <i class="material-icons"
               style="display: flex; justify-content:center;">menu</i>
          </button>
        </div>
        <div class="sub-overlay-tab-button" id="stationDetailButton">
          <i class="material-icons">home</i>
        </div>
        <div class="sub-overlay-tab-button" id="disruptionDetailButton">
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
  import glayout from './golden-layout/glayout.vue';

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

  const c = {
    settings: {
      showPopoutIcon: false,
      showMaximiseIcon: false,
      showCloseIcon: false
    }
  };
</script>

<script>
import { defineComponent } from "vue";
import { getSimulationComponent } from "./util/goldenLayoutHelper.js";
import { showDisruptionDetail } from "./components/disruption/disruption.js";
import { TimetableManager } from "./model/TimetableManager.js";

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
            componentState: {
              getCurrentInfrastructure: () => {
                return undefined
              }
            }
          },
          {
            title: 'Simulation',
            type: 'component',
            componentType: 'SimulationComponent',
          },
          {
            title: 'Timetable',
            type: 'component',
            componentType: 'TimetableComponent',
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
  mounted() {
    this.initManagers();
    this.initGLayout();
    this.initListeners();
  },

  methods: {
    initManagers() {
      window.timetableManager = new TimetableManager();
    },

    initGLayout() {
      this.$refs.GLayoutRoot.loadGLLayout(initLayout);
    },

    initListeners() {
      // Define global functions
      let overlayContainer = document.getElementById('overlayContainer');
      let overlayToggleButton = document.getElementById('overlayToggleButton');

      var showOverlay = () => overlayContainer.classList.remove('hidden');
      var hideOverlay = () => overlayContainer.classList.add('hidden');
      var toggleOverlay = () => overlayContainer.classList.toggle('hidden');

      let subOverlay = document.getElementById('subOverlay');
      var showSubOverlay = () => {
        showOverlay(), subOverlay.classList.remove('hidden');
      }
      var hideSubOverlay = () => subOverlay.classList.add('hidden');
      var toggleSubOverlay = () => subOverlay.classList.toggle('hidden');

      overlayToggleButton.addEventListener('click', toggleOverlay);

      var disruptionMap = new Map();
      disruptionMap.set('1', 80);
      disruptionMap.set('2', 120);
      var disruptionDists = false;

      let subOverlayClose = document.getElementById('subOverlayClose');
      subOverlayClose.addEventListener('click', _ => {
        subOverlay.classList.add('hidden');

        for (let overlayTab of document.getElementById('subOverlayTabs').children) {
          overlayTab.classList.remove('enabled');
        }
      });

      const addInfrastructureComponent = document.getElementById('addInfrastructureComponentButton');
      addInfrastructureComponent.addEventListener('click', () => {
        window.infrastructureManager.addInfrastructureComponent();
      });

      const addSimulationComponent = document.getElementById('addSimulationComponentButton');
      addSimulationComponent.addEventListener('click', () => {
        window.goldenLayout.addComponent('SimulationComponent', undefined, 'Simulation');
      });

      const addTimetableComponent = document.getElementById('addTimetableComponentButton');
      addTimetableComponent.addEventListener('click', () => {
        window.timetableManager.addTimetableComponent('TimetableComponent', undefined, 'Timetable');
      });

      const simulateButton = document.getElementById('simulateButton');
      simulateButton.addEventListener('click', () => {
        getSimulationComponent().forEach(simComp => simComp.simulate());
      });

      function fillOptions(selectElement, optionNames) {
        selectElement.length = 1;

        for (const name of optionNames) {
          if (name === '.' || name === '..') continue;

          let option = document.createElement("option");
          option.label = name;
          option.value = name;
          selectElement.add(option);
        }
      }

      const getSelectedOption = select => {
        if (select.selectedIndex === -1) {
          return '';
        }

        return select.options[select.selectedIndex].value;
      }

      const infrastructureSelect = document.getElementById('infrastructureSelect');
      fetch(window.origin + '/infrastructure/')
          .then(response => response.json())
          .then(dir => fillOptions(infrastructureSelect, dir['dirs']));

      infrastructureSelect.addEventListener('change', () => {
        const currentInfrastructureName = getSelectedOption(infrastructureSelect);
        window.infrastructureManager.load(currentInfrastructureName);
      });

      const timetableSelect = document.getElementById('timetableSelect');
      fetch(window.origin + '/timetable/')
          .then(response => response.json())
          .then(dir => fillOptions(timetableSelect, dir['dirs']));

      timetableSelect.addEventListener('change', () => {
        const currenTimetableName = getSelectedOption(timetableSelect);
        window.timetableManager.load(currenTimetableName, window.infrastructureManager.get());
      });

      const clearCacheButton = document.getElementById('clearCacheButton');
      clearCacheButton.addEventListener('click', _ => {
        deleteAllFiles();
      });

      let stationDetailButton = document.getElementById('stationDetailButton');
      stationDetailButton.addEventListener('click', e => {
        stationDetailButton.classList.toggle('enabled');
        showSubOverlay();
      });

      let disruptionDetailButton = document.getElementById('disruptionDetailButton');
      disruptionDetailButton.addEventListener('click', e => {
        disruptionDetailButton.classList.add('enabled');
        showDisruptionDetail(window.timetableManager.get());
        showSubOverlay();
      });
    }
  },
}
</script>
