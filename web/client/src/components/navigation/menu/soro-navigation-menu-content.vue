<template>
    <div>
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
                    disabled
                    @select="loadTimetable"
                />
            </div>

            <station-search
                :show-extended-link="true"
                class="station-search-field"
                @change-to-extended="changeToSearchOverlay"
            />

            <menu-settings class="settings" />

            <soro-collapsible
                label="Dev Tools"
                class="dev-tools"
            >
                <soro-button
                    label="Clear local storage"
                    @click="clearLocalStorage"
                />
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
</template>

<script setup lang="ts">
import DisruptionDetail from '@/components/disruption-detail.vue';
import SoroSelect from '@/components/soro-select.vue';
import SoroButton from '@/components/soro-button.vue';
import SoroCollapsible from '@/components/soro-collapsible.vue';
import StationSearch from '@/components/navigation/station-search/station-search.vue';
import MenuSettings from './menu-settings.vue';
</script>

<script lang="ts">
import { defineComponent } from 'vue';
import { mapActions, mapState } from 'vuex';
import { InfrastructureNamespace } from '@/stores/infrastructure-store';
import { TimetableNamespace } from '@/stores/timetable-store';
import { ComponentTechnicalName } from '@/golden-layout/golden-layout-constants';
import { GoldenLayoutNamespace } from '@/stores/golden-layout-store';

export default defineComponent({
    name: 'SoroNavigationMenuContent',
    emits: ['change-overlay'],

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
        addTab(componentTechnicalName: ComponentTechnicalName) {
            this.addGoldenLayoutTab({ componentTechnicalName });
        },

        clearLocalStorage() {
            window.localStorage.clear();
        },

        changeToSearchOverlay() {
            this.$emit('change-overlay', 'search');
        },

        ...mapActions(InfrastructureNamespace, { loadInfrastructure: 'load' }),
        ...mapActions(TimetableNamespace, { loadTimetable: 'load' }),
        ...mapActions(GoldenLayoutNamespace, ['addGoldenLayoutTab']),
    },
});
</script>

<style scoped>
.overlay-content {
    display: flex;
    flex-direction: column;
    pointer-events: auto;
    position: relative;
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

.station-search-field {
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