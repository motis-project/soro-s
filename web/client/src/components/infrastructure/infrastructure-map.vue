<template>
    <div ref="container">
        <div
            ref="map"
            class="map infrastructure-map"
        />
        <v-sheet
            ref="mapLegend"
            class="map-overlay infrastructure-map-legend"
        >
            <template
                v-for="(elementType, index) in legendControlTypes"
                :key="index"
            >
                <v-checkbox
                    :ref="elementType"
                    v-model="checkedControls"
                    :name="elementType"
                    :value="elementType"
                    class="legend-key"
                    color="primary"
                    density="compact"
                    min-height="0px"
                    hide-details
                >
                    <template #label>
                        <img
                            v-if="hasImage(elementType)"
                            class="legend-key-icon"
                            :src="iconUrl + elementType + iconExtension"
                            alt=""
                        >
                        {{ elementTypeLabels[elementType] ?? elementType }}
                    </template>
                </v-checkbox>
            </template>
        </v-sheet>
        <div
            ref="infrastructureTooltip"
            class="infrastructureTooltip infrastructure-tooltip"
        >
            <ul id="infrastructureTooltipList">
                <li id="kilometerPoint" />
                <li id="risingOrFalling" />
            </ul>
        </div>
    </div>
</template>

<script lang="ts">
import { mapState } from 'vuex';
import { InfrastructureNamespace } from '@/stores/infrastructure-store';
import {
    deHighlightSignalStationRoute,
    deHighlightStationRoute,
    highlightSignalStationRoute,
    highlightStationRoute
} from './infrastructureMap';
import { FilterSpecification, Map } from 'maplibre-gl';
import { infrastructureMapStyle } from './mapStyle';
import { addIcons, iconExtension, iconUrl } from './addIcons';
import { elementTypes, elementTypeLabels } from './elementTypes';
import { defineComponent } from 'vue';
import { transformUrl } from '@/api/api-client';

const specialLayoutControls = ['Rising', 'Falling'];
const initiallyCheckedControls = ['station', 'ms', 'as', 'eotd', ...specialLayoutControls];
const legendControlTypes = [
    ...elementTypes,
    ...specialLayoutControls
];

const mapDefaults = {
    style: infrastructureMapStyle,
    attributionControl: false,
    zoom: 18,
    hash: 'location',
    center: [8, 47],
    maxBounds: [[6, 45], [17, 55]], // [SW Point] [NE Point] in LonLat
    bearing: 0,
};

export default defineComponent({
    name: 'InfrastructureMap',

    data() {
        return {
            libreGLMap: null as (Map | null),
            legendControlTypes,
            checkedControls: Array.from(initiallyCheckedControls),
            iconUrl,
            iconExtension,
            elementTypeLabels: elementTypeLabels as { [elementType: string]: string },
        };
    },

    computed: {
        ...mapState(InfrastructureNamespace, [
            'currentInfrastructure',
            'highlightedSignalStationRouteID',
            'highlightedStationRouteID',
        ]),
    },

    watch: {
        currentInfrastructure(newInfrastructure: string | null) {
            this.libreGLMap?.remove();
            // Re-instantiating the map on infrastructure change currently leads to duplicated icon fetching on change.
            // @ts-ignore type instantiation for some reason is too deep
            this.libreGLMap = newInfrastructure ? this.createMap(newInfrastructure) : null;
        },

        checkedControls(newCheckedControls: string[], oldCheckedControls: string[]) {
            if (!this.libreGLMap) {
                return;
            }

            const controlsToDeactivate = oldCheckedControls.filter((control) => !newCheckedControls.includes(control));
            const controlsToActivate = newCheckedControls.filter((control) => !oldCheckedControls.includes(control));

            controlsToDeactivate.forEach((control) => {
                if (specialLayoutControls.includes(control)) {
                    this.evaluateSpecialLegendControls();

                    return;
                }

                this.setElementTypeVisibility(control, false);
            });

            controlsToActivate.forEach((control) => {
                if (specialLayoutControls.includes(control)) {
                    this.evaluateSpecialLegendControls();

                    return;
                }

                this.setElementTypeVisibility(control, true);
            });
        },

        highlightedSignalStationRouteID(newID, oldID) {
            if (!this.libreGLMap) {
                return;
            }

            if (newID) {
                // @ts-ignore
                highlightSignalStationRoute(this.libreGLMap, this.currentInfrastructure, newID);
            } else {
                // @ts-ignore
                deHighlightSignalStationRoute(this.libreGLMap, oldID);
            }
        },

        highlightedStationRouteID(newID, oldID) {
            if (newID) {
                // @ts-ignore
                highlightStationRoute(this.libreGLMap, this.currentInfrastructure, newID);
            } else {
                // @ts-ignore
                deHighlightStationRoute(this.libreGLMap, oldID);
            }
        },
    },

    methods: {
        resize() {
            if (!this.libreGLMap) {
                return;
            }

            this.libreGLMap.resize();
        },

        hasImage(elementType: string) {
            return !specialLayoutControls.includes(elementType);
        },

        setElementTypeVisibility(elementType: string, visible: boolean) {
            if (elementType !== 'station') {
                this.libreGLMap?.setLayoutProperty(
                    `circle-${elementType}-layer`,
                    'visibility',
                    visible ? 'visible': 'none',
                );
            }

            this.libreGLMap?.setLayoutProperty(
                `${elementType}-layer`,
                'visibility',
                visible ? 'visible': 'none',
            );
        },

        evaluateSpecialLegendControls() {
            if (!this.libreGLMap)  {
                return;
            }

            const rising_checked = (this.$refs.Rising as HTMLInputElement).checked;
            const falling_checked = (this.$refs.Falling as HTMLInputElement).checked;

            let filter: FilterSpecification;
            if (!rising_checked && falling_checked) {
                filter = ['!', ['get', 'rising']];
            } else if (rising_checked && !falling_checked) {
                filter = ['get', 'rising'];
            } else if (!rising_checked && !falling_checked) {
                filter = ['boolean', false];
            }

            elementTypes.forEach((elementType) => {
                if (elementType === 'station') {
                    return;
                }

                // @ts-ignore
                this.libreGLMap.setFilter(elementType + '-layer', filter);
                // @ts-ignore
                this.libreGLMap.setFilter('circle-' + elementType + '-layer', filter);
            });
        },

        createMap(infrastructure: string) {
            const map = new Map({
                ...mapDefaults,
                container: this.$refs.map as HTMLElement,
                // @ts-ignore
                transformRequest: (relative_url) => {
                    if (relative_url.startsWith('/')) {
                        return { url: transformUrl(`/${infrastructure}${relative_url}`) };
                    }
                }
            });

            map.on('load', async () => {
                await addIcons(map);
                elementTypes.forEach((type) => this.setElementTypeVisibility(type, this.checkedControls.includes(type)));
            });

            map.dragPan.enable({
                linearity: 0.01,
                easing: t => t,
                maxSpeed: 1400,
                deceleration: 2500
            });

            return map;
        },
    }
});
</script>

<style>
.infrastructure-map {
    padding: 0;
    margin: 0;
    position: absolute;
    height: 100%;
    width: 100%;
}

.infrastructure-tooltip {
    display: none;
    left: 0;
    top: 0;
    background: white;
    border: 2px;
    border-radius: 5px;
}

.map-overlay {
    position: absolute;
    bottom: 0;
    right: 0;
    margin-right: 20px;
}

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

<style href="..e-gl.css" rel="stylesheet" />
<style href="..re.css" rel="stylesheet" />
