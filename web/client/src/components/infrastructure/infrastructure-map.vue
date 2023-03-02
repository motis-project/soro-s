<template>
    <div ref="container">
        <div
            ref="map"
            class="map infrastructure-map"
        />
        <infrastructure-legend
            class="map-overlay"
            :checked-controls="checkedControls"
            @checked="(name) => onLegendControlChanged(name, true)"
            @unchecked="(name) => onLegendControlChanged(name, false)"
            @reset="resetLegend"
        />
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

<script setup lang="ts">
import InfrastructureLegend from '@/components/infrastructure/infrastructure-legend.vue';
</script>

<script lang="ts">
import { mapState } from 'vuex';
import { InfrastructureNamespace } from '@/stores/infrastructure-store';
import {
    deHighlightSignalStationRoute,
    deHighlightStationRoute,
    highlightSignalStationRoute,
    highlightStationRoute,
} from './infrastructureMap';
import { FilterSpecification, Map } from 'maplibre-gl';
import { createInfrastructureMapStyle } from './mapStyle';
import { addIcons } from './addIcons';
import { ElementTypes, ElementType } from './elementTypes';
import { defineComponent } from 'vue';
import { transformUrl } from '@/api/api-client';
import { ThemeInstance, useTheme } from 'vuetify';
import { SpecialLegendControls, SpecialLegendControl } from '@/components/infrastructure/infrastructure-legend.vue';

const initiallyCheckedControls = [
    ElementType.STATION,
    ElementType.HALT,
    ElementType.MAIN_SIGNAL,
    ElementType.APPROACH_SIGNAL,
    ElementType.END_OF_TRAIN_DETECTOR,
    ...SpecialLegendControls,
];

const mapDefaults = {
    attributionControl: false,
    zoom: 18,
    hash: 'location',
    center: [8, 47],
    maxBounds: [[6, 45], [17, 55]], // [SW Point] [NE Point] in LonLat
    bearing: 0,
};

export type MapPosition = {
    lat: number,
    lon: number,
};

export default defineComponent({
    name: 'InfrastructureMap',
    inject: {
        goldenLayoutKeyInjection: {
            default: '',
        },
    },

    setup() {
        return { currentTheme: useTheme().global };
    },

    data() {
        return {
            libreGLMap: null as (Map | null),
            checkedControls: Array.from(initiallyCheckedControls),
        };
    },

    computed: {
        checkedLegendControlLocalStorageKey() {
            return `infrastructure[${this.goldenLayoutKeyInjection}].checkedControls`;
        },

        ...mapState(InfrastructureNamespace, [
            'currentInfrastructure',
            'currentSearchedMapPosition',
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

        currentSearchedMapPosition(mapPosition: MapPosition) {
            if (!this.libreGLMap) {
                return;
            }

            this.libreGLMap.jumpTo({
                center: mapPosition,
                zoom: 14,
            });
        },

        currentTheme: {
            handler(newTheme: ThemeInstance) {
                if (!this.libreGLMap) {
                    return;
                }

                this.libreGLMap.setStyle(createInfrastructureMapStyle({
                    currentTheme: newTheme.current.value,
                    activatedElements: this.checkedControls,
                }));
            },
            deep: true,
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

    created() {
        const checkedControlsString = window.localStorage.getItem(this.checkedLegendControlLocalStorageKey);
        if (checkedControlsString) {
            this.checkedControls = JSON.parse(checkedControlsString);
        }
    },

    methods: {
        resize() {
            if (!this.libreGLMap) {
                return;
            }

            this.libreGLMap.resize();
        },

        onLegendControlChanged(legendControl: string, checked: boolean) {
            if (checked) {
                this.checkedControls.push(legendControl);
            } else {
                this.checkedControls = this.checkedControls.filter((control) => control !== legendControl);
            }

            this.saveControls();

            if (!this.libreGLMap) {
                return;
            }

            if (SpecialLegendControls.includes(legendControl)) {
                this.evaluateSpecialLegendControls();

                return;
            }

            this.setElementTypeVisibility(legendControl, checked);
        },

        resetLegend() {
            this.checkedControls = initiallyCheckedControls;
            this.saveControls();
            this.setVisibilityOfAllControls();
        },

        saveControls() {
            window.localStorage.setItem(this.checkedLegendControlLocalStorageKey, JSON.stringify(this.checkedControls));
        },

        setVisibilityOfAllControls() {
            ElementTypes.forEach((type) => this.setElementTypeVisibility(type, this.checkedControls.includes(type)));
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

            const rising_checked = this.checkedControls.includes(SpecialLegendControl.RISING);
            const falling_checked = this.checkedControls.includes(SpecialLegendControl.FALLING);

            let filter: FilterSpecification;
            if (!rising_checked && falling_checked) {
                filter = ['!', ['get', 'rising']];
            } else if (rising_checked && !falling_checked) {
                filter = ['get', 'rising'];
            } else if (!rising_checked && !falling_checked) {
                filter = ['boolean', false];
            }

            ElementTypes.forEach((elementType) => {
                if (elementType === ElementType.STATION) {
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
                },
                style: createInfrastructureMapStyle({
                    currentTheme: this.$vuetify.theme.current,
                    activatedElements: this.checkedControls,
                }),
            });

            map.on('load', async () => {
                await addIcons(map);
                this.setVisibilityOfAllControls();
            });

            map.dragPan.enable({
                linearity: 0.01,
                easing: t => t,
                maxSpeed: 1400,
                deceleration: 2500,
            });

            return map;
        },
    },
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
</style>

<style href="..e-gl.css" rel="stylesheet" />
<style href="..re.css" rel="stylesheet" />
