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

            <soro-collapsible
                label="Settings"
                class="settings"
            >
                <v-btn-toggle
                    :model-value="darkLightModePreference"
                    tile
                    color="secondary"
                    group
                    mandatory
                    @update:model-value="setDarkLightModePreference"
                >
                    <v-btn :value="DarkLightModes.OS">
                        OS
                    </v-btn>
                    <v-btn :value="DarkLightModes.LIGHT">
                        Light
                    </v-btn>
                    <v-btn :value="DarkLightModes.DARK">
                        Dark
                    </v-btn>
                </v-btn-toggle>

                <v-sheet
                    title="some"
                    class="d-flex accent-color-picker"
                >
                    <div class="flex-grow-0 accent-color-display" />
                    <v-btn class="flex-grow-1 ms-2">
                        Select color
                        <v-menu
                            activator="parent"
                            :close-on-content-click="false"
                        >
                            <v-color-picker
                                style="overflow: unset;"
                                :model-value="colorSelection"
                                min-width="300"
                                hide-inputs
                                show-swatches
                                @update:model-value="onUpdateColorSelection"
                            />
                        </v-menu>
                    </v-btn>
                </v-sheet>
            </soro-collapsible>

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
</script>

<script lang="ts">
import { defineComponent } from 'vue';
import { mapActions, mapState } from 'vuex';
import { InfrastructureNamespace } from '@/stores/infrastructure-store';
import { TimetableNamespace } from '@/stores/timetable-store';
import { GLComponentTitles, ComponentTechnicalName } from '@/golden-layout/golden-layout-constants';
import { DarkLightModes, SettingsNamespace } from '@/stores/settings-store';
import { GoldenLayoutNamespace } from '@/stores/golden-layout-store';

export default defineComponent({
    name: 'SoroNavigationMenuContent',
    emits: ['change-overlay'],

    data() {
        return {
            colorSelection: null as (null | string),
            showColorSelector: false,
            showOverlay: false,
            ComponentTechnicalNames: ComponentTechnicalName,
            DarkLightModes,
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
        ...mapState(SettingsNamespace, [
            'darkLightModePreference',
            'primaryColor',
        ]),
    },

    mounted() {
        this.colorSelection = this.primaryColor;
    },

    methods: {
        addTab(componentTechnicalName: ComponentTechnicalName) {
            this.addGoldenLayoutTab({
                componentTechnicalName,
                title: GLComponentTitles[componentTechnicalName],
            });
        },

        onUpdateColorSelection(newColor: string) {
            this.setPrimaryColor(newColor);
            this.colorSelection = newColor;
        },

        clearLocalStorage() {
            window.localStorage.clear();
        },

        changeToSearchOverlay() {
            this.$emit('change-overlay', 'search');
        },

        ...mapActions(SettingsNamespace, [
            'setDarkLightModePreference',
            'setPrimaryColor',
        ]),
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

.accent-color-picker {
    margin-top: 20px;
    max-width: 100%;
    justify-self: center;
}

.accent-color-display {
    height: auto;
    width: 50px;
    background: rgb(var(--v-theme-primary));
    border-radius: 5px;
}
</style>