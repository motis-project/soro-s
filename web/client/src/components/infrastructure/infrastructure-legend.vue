<template>
    <v-sheet
        ref="mapLegend"
        class="infrastructure-map-legend"
        :elevation="5"
    >
        <template
            v-for="(elementType, index) in legendControlTypes"
            :key="index"
        >
            <v-checkbox
                :ref="elementType"
                :model-value="checkedControls"
                :name="elementType"
                :value="elementType"
                class="legend-key"
                color="primary"
                density="compact"
                min-height="0px"
                hide-details
                @input="emitChange"
            >
                <template #label>
                    <img
                        v-if="hasImage(elementType)"
                        class="legend-key-icon"
                        :src="iconUrl + elementType + iconExtension"
                        alt=""
                    >
                    {{ getElementLabel(elementType) }}
                </template>
            </v-checkbox>
        </template>
    </v-sheet>
</template>

<script lang="ts">
import { ElementTypeLabels, ElementTypes } from '@/components/infrastructure/elementTypes.js';
import { iconExtension, iconUrl } from '@/components/infrastructure/addIcons';
import { defineComponent } from 'vue';

export const SpecialLegendControl = {
    RISING: 'Rising',
    FALLING: 'Falling',
};
export const SpecialLegendControls = Object.values(SpecialLegendControl);
const legendControlTypes = [
    ...ElementTypes,
    ...SpecialLegendControls,
];

export default defineComponent({
    name: 'InfrastructureLegend',

    props: {
        checkedControls: {
            type: Array,
            required: true,
        },
    },

    emits: [
        'checked',
        'unchecked',
    ],

    data() {
        return {
            legendControlTypes,
            iconUrl,
            iconExtension,
        };
    },

    methods: {
        hasImage(elementType: string) {
            return !SpecialLegendControls.includes(elementType);
        },

        getElementLabel(elementType: string) {
            return ElementTypeLabels[elementType] ?? elementType;
        },

        emitChange(event: Event) {
            const eventTarget = event.target as HTMLInputElement;

            this.$emit(
                eventTarget.checked ? 'checked' : 'unchecked',
                eventTarget.value,
            );
        },
    },
});
</script>

<style scoped>
.infrastructure-map-legend {
    padding: 10px 10px 20px;
    height: fit-content;
    margin-bottom: 40px;
    width: fit-content;
}

.legend-key {
    height: 1.5em;
    margin-right: 5px;
    margin-left: 5px;
}

.legend-key-icon {
    margin-right: 7px;
    margin-left: 7px;
    display: inline-block;
    height: 1em;
}
</style>