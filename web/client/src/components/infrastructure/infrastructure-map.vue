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
					{{ elementTypesReadable[elementType] ?? elementType }}
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

<script>
import { mapState } from 'vuex';
import { InfrastructureNamespace } from '../../stores/infrastructure-store.ts';
import {
	deHighlightSignalStationRoute,
	deHighlightStationRoute,
	highlightSignalStationRoute,
	highlightStationRoute
} from './infrastructureMap.js';
import { Map } from 'maplibre-gl';
import { infrastructureMapStyle } from './mapStyle.js';
import { addIcons, iconExtension, iconUrl } from './addIcons.js';
import { elementTypes, elementTypesReadable } from './elementTypes.js';
import { ClickTooltip } from '../../util/Tooltip.js';

const specialLayoutControls = ['Rising', 'Falling']; // question figure out what these do
const initiallyCheckedControls = ['station', 'ms', 'as', 'eotd', ...specialLayoutControls];
const legendControlTypes = [
	...elementTypes,
	...specialLayoutControls
];

export default {
	name: 'InfrastructureMap',

	data() {
		return {
			libreGLMap: undefined,
			tooltip: undefined,
			legendControlTypes,
			initiallyCheckedControls,
			iconUrl,
			iconExtension,
			elementTypesReadable,
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
		currentInfrastructure(newInfrastructure) {
			this.libreGLMap = newInfrastructure ? this.createMap(newInfrastructure) : undefined;
		},

		highlightedSignalStationRouteID(newID, oldID) {
			if (newID) {
				highlightSignalStationRoute(this.libreGLMap, this.currentInfrastructure, newID);
			} else {
				deHighlightSignalStationRoute(this.libreGLMap, this.currentInfrastructure, oldID);
			}
		},

		highlightedStationRouteID(newID, oldID) {
			if (newID) {
				highlightStationRoute(this.libreGLMap, this.currentInfrastructure, newID);
			} else {
				deHighlightStationRoute(this.libreGLMap, oldID);
			}
		},
	},

	created() { this.componentCreated(); },

	methods: {
		componentCreated() {
			this.tooltip = new ClickTooltip(this.$refs.infrastructureTooltip); // question what does he do
		},

		hasImage(elementType) {
			return !specialLayoutControls.includes(elementType);
		},

		onLegendControlClicked(event) {
			if (specialLayoutControls.includes(event.target.id)) {
				this.evaluateSpecialLegendControls();

				return;
			}

			this.evaluateLegendControlForControlType(event.target.value);
		},

		evaluateLegendControlForControlType(type) {
			this.libreGLMap.setLayoutProperty(type + '-layer', 'visibility', this.$refs[type][0].checked ? 'visible' : 'none');

			if (type !== 'station') {
				this.libreGLMap.setLayoutProperty('circle-' + type + '-layer', 'visibility', this.$refs[type][0].checked ? 'visible' : 'none');
			}
		},

		evaluateSpecialLegendControls() {
			const rising_checked = this.$refs.Rising.checked;
			const falling_checked = this.$refs.Falling.checked;

			let filter;
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

				this.libreGLMap.setFilter(elementType + '-layer', filter);
				this.libreGLMap.setFilter('circle-' + elementType + '-layer', filter);
			});
		},

		createMap(infrastructure) {
			let map = new Map({
				container: this.$refs.map,
				style: infrastructureMapStyle,
				attributionControl: false,
				zoom: 14,
				hash: 'location',
				center: [14, 49],
				maxBounds: [[6, 45], [17, 55]], // [SW Point] [NE Point] in LonLat
				bearing: 0,
				transformRequest: (relative_url) => {
					if (relative_url.startsWith('/')) {
						const url = window.origin + '/' + infrastructure + relative_url;
						return { url };
					}
				}
			});

			map.on('load', () => {
				addIcons(map);
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
};
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
  border-radius: var(--border-radius)
}

.infrastructure-map-legend {
  padding: 10px;
  box-shadow: 0 1px 2px rgba(0, 0, 0, 0.1);
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
