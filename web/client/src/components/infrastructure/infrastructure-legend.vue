<template>
    <div class="infrastructure-legend">
        <v-btn
            class="infrastructure-legend-collapse-button"
            color="primary"
            @click="() => isExpanded = !isExpanded"
        >
            <i class="material-icons">{{ collapseIcon }}</i>
        </v-btn>
        <v-sheet
            ref="mapLegend"
            class="infrastructure-map-legend"
            :class="isExpanded ? '' : 'infrastructure-map-legend-collapsed'"
            :elevation="5"
        >
            <div class="infrastructure-map-reset">
                <a
                    href="/"
                    @click.prevent="$emit('reset')"
                >
                    Reset
                </a>
            </div>

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
    </div>
</template>

<script lang="ts">
import { ElementTypeLabels, ElementTypes } from '@/components/infrastructure/element-types.js';
import { iconExtension, iconUrl } from './add-icons';
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
        'change',
        'reset',
    ],

    data() {
        return {
            legendControlTypes,
            iconUrl,
            iconExtension,
            isExpanded: true,
        };
    },

    computed: {
        collapseIcon() {
            return this.isExpanded ? 'chevron_right' : 'legend_toggle';
        },
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
                'change',
                eventTarget.value,
                eventTarget.checked,
            );
        },
    },
});
</script>

<style scoped>
.infrastructure-legend {
    display: flex;
    margin-bottom: 40px;
}

.infrastructure-legend-collapse-button {
    align-self: end;
}

.infrastructure-map-legend {
    padding: 10px 10px 20px;
    height: fit-content;
    width: fit-content;
}

.infrastructure-map-legend-collapsed {
    display: none;
}

.infrastructure-map-reset {
    display: flex;
    flex-direction: row;
    justify-content: end;
    width: 100%;
    color: rgb(var(--v-theme-primary));
}

a,
a:visited,
a:hover,
a:active {
    color: inherit;
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