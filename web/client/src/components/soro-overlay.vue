<template>
    <div
        ref="overlayContainer"
        :class="`overlay-container ${showOverlay ? '' : 'hidden'}`"
    >
        <div class="overlay">
            <div class="overlay-content">
                <div class="window-controls">
                    <soro-button
                        label="New Infrastructure Tab"
                        @click="addTab(ComponentTechnicalNames.INFRASTRUCTURE)"
                    />
                    <soro-button
                        disabled
                        label="New Simulation Tab"
                    />
                    <soro-button
                        disabled
                        label="New Timetable Tab"
                    />
                    <soro-button
                        label="New OrderingGraph Tab"
                        @click="addTab(ComponentTechnicalNames.ORDERING_GRAPH)"
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
                        @select="loadTimetable"
                    />
                </div>

                <soro-collapsible
                    label="Dev Tools"
                    class="dev-tools"
                >
                    <soro-button
                        disabled
                        label="Clear Cache"
                    />
                    <soro-button
                        disabled
                        label="Simulate"
                    />
                </soro-collapsible>
            </div>
            <div
                ref="subOverlay"
                class="sub-overlay hidden"
            >
                <div
                    id="subOverlayContent"
                    class="sub-overlay-content"
                >
                    <disruption-detail
                        v-if="false"
                        ref="disruption"
                    />
                </div>
                <div
                    ref="subOverlayClose"
                    class="sub-overlay-close"
                >
                    <i class="material-icons">close</i>
                </div>
            </div>
        </div>
        <div
            ref="subOverlayTabs"
            class="overlay-tabs"
        >
            <div class="overlay-toggle">
                <button
                    class="matter-button-contained overlay-toggle-button"
                    @click="toggleOverlay"
                >
                    <i
                        class="material-icons"
                        style="display: flex; justify-content: center;"
                    >menu</i>
                </button>
            </div>
            <div
                ref="stationDetailButton"
                class="sub-overlay-tab-button"
            >
                <i class="material-icons">home</i>
            </div>
            <div
                ref="disruptionDetailButton"
                class="sub-overlay-tab-button"
            >
                <i class="material-icons">train</i>
            </div>
        </div>
    </div>
</template>

<script setup lang="ts">
import DisruptionDetail from './disruption-detail.vue';
import SoroSelect from './soro-select.vue';
import SoroButton from './soro-button.vue';
import SoroCollapsible from '@/components/soro-collapsible.vue';
</script>

<script lang="ts">
import { defineComponent } from 'vue';
import { mapActions, mapState } from 'vuex';
import { InfrastructureNamespace } from '@/stores/infrastructure-store';
import { TimetableNamespace } from '@/stores/timetable-store';
import { GLComponentTitles, ComponentTechnicalName } from '@/golden-layout/golden-layout-constants';

export default defineComponent({
    name: 'SoroOverlay',

    emits: ['add-golden-layout-tab'],

    data() {
        return {
            showOverlay: false,
            ComponentTechnicalNames: ComponentTechnicalName,
        };
    },

    computed: {
        ...mapState(InfrastructureNamespace, [
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
            this.showOverlay = !this.showOverlay;
        },

        addTab(componentTechnicalName: ComponentTechnicalName) {
            this.$emit(
                'add-golden-layout-tab',
                {
                    componentTechnicalName,
                    title: GLComponentTitles[componentTechnicalName],
                },
            );
        },

        ...mapActions(InfrastructureNamespace, { loadInfrastructure: 'load' }),

        ...mapActions(TimetableNamespace, { loadTimetable: 'load' }),
    }
});
</script>

<style scoped>
.overlay {
    z-index: 10;
    background-color: var(--overlay-color);
    box-shadow: 0 10px 20px rgb(0 0 0 / 19%), 0 6px 6px rgb(0 0 0 / 23%);
    border-radius: 3px 0 3px 3px;
    transition: all 0.4s ease;
    width: var(--overlay-width);
    flex: 0 0 auto;
    order: 1;
    height: 95%;
    position: relative;
}

.overlay-container.hidden .overlay {
    box-shadow: unset;
}

.overlay-content {
    display: flex;
    flex-direction: column;
    height: 100%;
    pointer-events: auto;
    position: relative;
}

.overlay-tabs {
    z-index: 10;
    order: 2;
    pointer-events: auto;
    display: flex;
    flex-direction: column;
}

.overlay-toggle-button {
    box-shadow: 0 10px 20px rgb(0 0 0 / 19%), 0 6px 6px rgb(0 0 0 / 23%);
    border-bottom-left-radius: 0;
    border-top-left-radius: 0;
}

.overlay-container {
    display: flex;
    align-items: flex-start;
    justify-content: center;
    height: 100%;
    padding-left: var(--overlay-padding-left);
    padding-top: var(--overlay-padding-top);
    padding-bottom: 20px;
    position: absolute;
    top: 0;
    left: 0;
    transition: all 0.4s ease;
    pointer-events: none;
}

.overlay-container.hidden {
    left: calc(0px - calc(var(--overlay-width) + var(--overlay-padding-left)));
}

.sub-overlay {
    position: absolute;
    top: 0;
    left: 0;
    padding: 0.8em;
    width: calc(100% - 1.6em);
    height: calc(100% - 1.6em);
    transition: all 0.2s ease;
    pointer-events: auto;
    z-index: 20;
    border-radius: var(--border-radius);
}

.sub-overlay.hidden {
    left: calc(0px - calc(var(--overlay-width) + var(--overlay-padding-left)));
}

.sub-overlay-content {
    width: 100%;
    height: 100%;
    display: flex;
    flex-direction: column;
    border-radius: inherit;
    background: var(--overlay-color);
    box-shadow: 0 6px 6px rgb(0 0 0 / 23%), 0 -2px 6px rgb(0 0 0 / 23%);
}

.sub-overlay-close {
    position: absolute;
    top: 18px;
    right: 18px;
    cursor: pointer;
    color: var(--icon-color);
}

.sub-overlay-close i {
    cursor: pointer;
}

.window-controls,
.dev-tools {
    display: flex;
    flex-flow: column wrap;
    width: 94%;
    justify-content: space-around;
    padding: 3%;
    margin-top: 0.5em;
    margin-bottom: 0.5em;
}

.data-selects {
    display: flex;
    flex-flow: column wrap;
    width: 94%;
    justify-content: space-around;
    padding: 3%;
    margin-top: 0.5em;
    margin-bottom: 0.5em;
}

.data-select {
    margin-top: 2.2em;
    margin-bottom: 2.2em;
}

/*  ============= Station Detail ============= */

.sub-overlay-tab-button {
    width: 30px;
    height: 40px;
    margin-top: 10px;
    background: var(--dialog-color);
    color: var(--secondary-text-color);
    box-shadow: 3px 3px 2px rgb(0 0 0 / 20%);
    cursor: pointer;
    display: flex;
    align-items: center;
    justify-content: center;
    opacity: 1;
    transition: all 0.2s ease;
}

.sub-overlay-tab-button.enabled {
    color: white;
    background: var(--highlight-color);
}
</style>