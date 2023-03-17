<template>
    <soro-collapsible label="Settings">
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
</template>

<script setup lang="ts">
import SoroCollapsible from '@/components/soro-collapsible.vue';
</script>

<script lang="ts">
import { DarkLightModes, SettingsNamespace } from '@/stores/settings-store';
import { mapActions, mapState } from 'vuex';
import { defineComponent } from 'vue';

export default defineComponent({
    name: 'MenuSettings',

    data() {
        return {
            colorSelection: null as (null | string),
            showColorSelector: false,
            DarkLightModes,
        };
    },

    computed: {
        ...mapState(SettingsNamespace, [
            'darkLightModePreference',
            'primaryColor',
        ]),
    },

    mounted() {
        this.colorSelection = this.primaryColor;
    },

    methods: {
        onUpdateColorSelection(newColor: string) {
            this.setPrimaryColor(newColor);
            this.colorSelection = newColor;
        },

        ...mapActions(SettingsNamespace, [
            'setDarkLightModePreference',
            'setPrimaryColor',
        ]),
    },
});
</script>

<style scoped>
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