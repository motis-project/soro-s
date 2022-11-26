<template>
  <div>
    <div id="simWrapper">
      <div id="simDivider">
        <div id="simGraph"></div>
      </div>
      <div id="distGraph"></div>
    </div>

    <div id="toolTip" class="toolTip">
      <ul id="tooltipList">
        <li id="tooltipID"></li>
        <li id="tooltipSSRID"></li>
      </ul>
    </div>
  </div>
</template>

<script>
import { d3Graph } from './simulation/d3Graph.js';
import { mapState } from 'vuex';
import {InfrastructureNameSpace} from "../../stores/infrastructure-store.js";
import {TimetableNamespace} from "../../stores/timetable-store.js";

const nodeWidth = 100;
const nodeHeight = 80;
const nodeDiagonal = Math.sqrt(Math.pow(nodeWidth, 2) + Math.pow(nodeHeight, 2));
const nodeSpacingHorizontal = nodeWidth * 0.5;
const nodeSpacingVertical = nodeHeight * 0.5;

const outerWidthPadding = nodeWidth * 0.1;
const outerHeightPadding = nodeHeight * 0.1;

const innerNodeCount = 3;

const innerNodeWidth = (nodeWidth - outerWidthPadding * 2);
const innerNodeHeight = (nodeHeight - outerHeightPadding * 2) / innerNodeCount;

const innerHeightPadding = innerNodeHeight * 0.1;

export default {
  name: "SimulationComponent",

  data() {
    return {
      container: undefined,
      componentState: undefined,
      _d3Graph: undefined,
    }
  },

  computed: {
    ...mapState(InfrastructureNameSpace, ['currentInfrastructure']),
    ...mapState(TimetableNamespace, ['currentTimetable']),
  },

  watch: {
    currentInfrastructure: { handler() { this.reloadGraph() }  },
    currentTimetable: { handler() { this.reloadGraph() } },
  },

  methods: {
    setContainer(container) {
      this.container = container;
      this._d3Graph = new d3Graph(container.element, container.width, container.height);
    },

    setComponentState(componentState) {
      this.componentState = componentState;
    },

    reloadGraph() {
      if (this.currentTimetable && this.currentInfrastructure) {
        this._d3Graph.createGraph(this.currentInfrastructure, this.currentTimetable);
      }
    },

    simulate() {
      this._d3Graph.simulate();
    },
  }
}
</script>

<style scoped>
#simWrapper {
  display: grid;
  grid-template-columns: 1fr 1fr;
}

#simDivider {
  border-right: 1px solid #DEDEDE;
}

#toolTip {
  display: none;
  left: 0;
  top: 0;
  background: lightsteelblue;
  border: 2px;
  border-radius: 5px;
}
</style>

<style rel="stylesheet" href="..ss" />
