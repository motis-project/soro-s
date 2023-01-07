<template>
    <div>
        <div id="simWrapper">
            <div id="simDivider">
                <div id="simGraph" />
            </div>
            <div id="distGraph" />
        </div>

        <div
            id="toolTip"
            class="toolTip"
        >
            <ul id="tooltipList">
                <li id="tooltipID" />
                <li id="tooltipSSRID" />
            </ul>
        </div>
    </div>
</template>

<script>
/* eslint-disable */
import { d3Graph } from '@/components/simulation/d3Graph.js';
import { mapState } from 'vuex';
import { InfrastructureNamespace } from '@/stores/infrastructure-store';
import { TimetableNamespace } from '@/stores/timetable-store';
import { ComponentContainer } from 'golden-layout';
import { defineComponent } from 'vue';

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

export default defineComponent({
    name: 'SimulationComponent',

    props: {
        container: {
            type: ComponentContainer,
            required: false,
            default: undefined,
        },
    },

    data() {
        return { d3Graph: undefined };
    },

    computed: {
        ...mapState(InfrastructureNamespace, ['currentInfrastructure']),
        ...mapState(TimetableNamespace, ['currentTimetable']),
    },

    watch: {
        currentInfrastructure: { handler() {
            this.reloadGraph(); 
        }  },
        currentTimetable: { handler() {
            this.reloadGraph(); 
        } },
    },

    created() {
        this.configureContainer(); 
    },

    methods: {
        configureContainer() {
            this.d3Graph = new d3Graph(this.container.element, this.container.width, this.container.height);
        },

        reloadGraph() {
            if (this.currentTimetable && this.currentInfrastructure) {
                this.d3Graph.createGraph(this.currentInfrastructure, this.currentTimetable);
            }
        },

        simulate() {
            this.d3Graph.simulate();
        },
    },
});
</script>

<style scoped>
/* stylelint-disable */
#simWrapper {
    display: grid;
    grid-template-columns: 1fr 1fr;
}

#simDivider {
    border-right: 1px solid #dedede;
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
