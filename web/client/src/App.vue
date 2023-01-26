<template>
    <div class="full-height">
        <v-theme-provider :theme="darkLightModePreference">
            <v-layout>
                <soro-navigation @add-golden-layout-tab="addGoldenLayoutTab" />
            </v-layout>
            <golden-layout-adapter
                ref="GLayoutRoot"
                class="golden-layout-root"
            />
        </v-theme-provider>
    </div>
</template>

<script setup lang="ts">
import GoldenLayoutAdapter from '@/golden-layout/golden-layout-adapter.vue';
import SoroNavigation from '@/components/navigation/soro-navigation.vue';
</script>

<script lang="ts">
import { mapActions, mapState } from 'vuex';
import { InfrastructureNamespace } from '@/stores/infrastructure-store';
import { TimetableNamespace } from '@/stores/timetable-store';
import { defineComponent, ref } from 'vue';
import { LayoutConfig } from 'golden-layout';
import { ComponentTechnicalName, GLComponentNames, GLComponentTitles } from '@/golden-layout/golden-layout-constants';
import { SettingsNamespace } from '@/stores/settings-store';

const initLayout: LayoutConfig = {
    dimensions: { headerHeight: 36 },
    root: {
        type: 'row',
        content: [
            {
                type: 'column',
                content: [
                    {
                        title: GLComponentTitles[ComponentTechnicalName.INFRASTRUCTURE],
                        type: 'component',
                        componentType: GLComponentNames[ComponentTechnicalName.INFRASTRUCTURE],
                    },
                ]
            }
        ]
    }
};

const GLayoutRoot = ref();

export default defineComponent({
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

    mounted() {
        this.loadInfrastructures();
        this.loadTimetables();
        GLayoutRoot.value.loadGLLayout(initLayout);
    },

    methods: {
        addGoldenLayoutTab({ componentTechnicalName, title }: { componentTechnicalName: ComponentTechnicalName, title: string}) {
            GLayoutRoot.value.addGLComponent(GLComponentNames[componentTechnicalName], title);
        },

        ...mapActions(InfrastructureNamespace, {
            loadInfrastructures: 'initialLoad',
            loadInfrastructure: 'load',
        }),

        ...mapActions(TimetableNamespace, {
            loadTimetables: 'initialLoad',
            loadTimetable: 'load',
        }),
    },
});
</script>

<style scoped>
.golden-layout-root {
    width: 100%;
    height: calc(100% - 90px);
}

.full-height {
    height: 100%;
}
</style>
