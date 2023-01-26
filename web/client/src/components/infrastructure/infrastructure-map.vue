<template>
    <div ref="container">
        <div
            ref="map"
            class="map infrastructure-map"
        />
        <div
            ref="mapLegend"
            class="map-overlay infrastructure-map-legend"
        >
            <template
                v-for="(elementType, index) in legendControlTypes"
                :key="index"
            >
                <input
                    :id="elementType"
                    :ref="elementType"
                    :value="elementType"
                    :checked="initiallyCheckedControls.includes(elementType)"
                    type="checkbox"
                    @input="onLegendControlClicked"
                >
                <label
                    class="legend-key"
                    :for="elementType"
                >
                    <img
                        v-if="hasImage(elementType)"
                        class="legend-key-icon"
                        :src="iconUrl + elementType + iconExtension"
                        alt=""
                    >
                    {{ elementTypeLabels[elementType] ?? elementType }}
                </label>
                <br>
            </template>
        </div>

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
            initiallyCheckedControls,
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
            // Re-instantiating the map on infrastructure change currently leads to duplicated icon fetching on change.
            // @ts-ignore type instantiation for some reason is too deep
            this.libreGLMap = newInfrastructure ? this.createMap(newInfrastructure) : null;
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

        onLegendControlClicked(event: Event) {
            if (specialLayoutControls.includes((event.target as HTMLInputElement).id)) {
                this.evaluateSpecialLegendControls();

                return;
            }

            this.evaluateLegendControlForControlType((event.target as HTMLInputElement).value);
        },

        evaluateLegendControlForControlType(type: string) {
            if (!this.libreGLMap) {
                return;
            }

            // @ts-ignore
            this.libreGLMap.setLayoutProperty(type + '-layer', 'visibility', this.$refs[type][0].checked ? 'visible' : 'none');

            if (type !== 'station') {
                // @ts-ignore
                this.libreGLMap.setLayoutProperty('circle-' + type + '-layer', 'visibility', this.$refs[type][0].checked ? 'visible' : 'none');
            }
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
                elementTypes.forEach((type) => this.evaluateLegendControlForControlType(type));
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
    background: var(--overlay-color);
    margin-right: 20px;
    font-family: var(--main-font-family);
    overflow: auto;
    border-radius: var(--border-radius);
}

.infrastructure-map-legend {
    padding: 10px;
    box-shadow: 0 1px 2px rgb(0 0 0 / 10%);
    line-height: 18px;
    height: fit-content;
    margin-bottom: 40px;
    width: fit-content;
    background: var(--overlay-color);
}

.legend-key {
    height: 1em;
    margin-right: 5px;
    margin-left: 5px;
}

.legend-key-icon {
    margin-right: 7px;
    display: inline-block;
    height: 1em;
}
</style>

<style href="..e-gl.css" rel="stylesheet" />
<style href="..re.css" rel="stylesheet" />
