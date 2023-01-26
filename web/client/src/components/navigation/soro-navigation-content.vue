<template>
    <div
        ref="overlayContainer"
        class="overlay-container"
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
                        disabled
                        @select="loadTimetable"
                    />
                </div>

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
                        <v-btn value="light">
                            Light
                        </v-btn>
                        <v-btn value="dark">
                            Dark
                        </v-btn>
                    </v-btn-toggle>
                </soro-collapsible>

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
    </div>
</template>

<script setup lang="ts">
import DisruptionDetail from '../disruption-detail.vue';
import SoroSelect from '../soro-select.vue';
import SoroButton from '../soro-button.vue';
import SoroCollapsible from '@/components/soro-collapsible.vue';
</script>

<script lang="ts">
import { defineComponent } from 'vue';
import { mapActions, mapMutations, mapState } from 'vuex';
import { InfrastructureNamespace } from '@/stores/infrastructure-store';
import { TimetableNamespace } from '@/stores/timetable-store';
import { GLComponentTitles, ComponentTechnicalName } from '@/golden-layout/golden-layout-constants';
import { SettingsNamespace } from '@/stores/settings-store';

export default defineComponent({
    name: 'SoroNavigationContent',

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
        ...mapState(SettingsNamespace, ['darkLightModePreference']),
    },

    methods: {
        addTab(componentTechnicalName: ComponentTechnicalName) {
            this.$emit(
                'add-golden-layout-tab',
                {
                    componentTechnicalName,
                    title: GLComponentTitles[componentTechnicalName],
                },
            );
        },

        ...mapMutations(SettingsNamespace, ['setDarkLightModePreference']),
        ...mapActions(InfrastructureNamespace, { loadInfrastructure: 'load' }),
        ...mapActions(TimetableNamespace, { loadTimetable: 'load' }),
    }
});
</script>

<style scoped>
.overlay {
    width: var(--overlay-width);
    height: 95%;
}

.overlay-content {
    display: flex;
    flex-direction: column;
    pointer-events: auto;
    position: relative;
}

.overlay-container {
    display: flex;
    align-items: flex-start;
    justify-content: center;
    transition: all 0.4s ease;
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
    justify-content: space-around;
    padding: 3%;
    margin-top: 0.5em;
    margin-bottom: 0.5em;
}

.data-selects {
    display: flex;
    flex-flow: column wrap;
    justify-content: space-around;
    padding: 3%;
    margin-top: 0.5em;
    margin-bottom: 0.5em;
}

.settings {
    padding: 3%;
}
</style>