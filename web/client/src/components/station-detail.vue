<template>
	<div>
		<div
			v-if="station"
			id="stationDetail"
			class="station-detail hidden"
		>
			<div class="station-detail-name">
				{{ station.name }}
			</div>
			<div class="station-detail-routes">
				<div class="wrap-collapsible">
					<input
						id="collapsible"
						class="collapsible-toggle hidden"
						type="checkbox"
					>
					<label
						for="collapsible"
						class="collapsible-toggle"
					>Station Routes</label>
					<div class="collapsible-content">
						<div
							id="stationRouteCollapsibleContent"
							class="content-inner"
						>
							<template
								v-for="(stationRoute, index) in stationRoute"
								:key="index"
							>
								<label class="matter-switch station-detail-signal-route">
									<input
										type="checkbox"
										:value="stationRoute.sr.id"
										:checked="highlightedStationRoutes.find(f => f.properties.id === stationRoute.sr.id)"
										@change="onStationRouteInput"
									>
									<span>{{ stationRoute.key }}</span>
								</label>
							</template>
						</div>
					</div>
				</div>
			</div>
			<div class="station-detail-signal-routes">
				<!-- TODO refactor collapsible to own component -->
				<div class="wrap-collapsible">
					<input
						id="ssrCollapsible"
						class="collapsible-toggle hidden"
						type="checkbox"
					>
					<label
						for="ssrCollapsible"
						class="collapsible-toggle"
					>Signal Station Routes</label>
					<div class="collapsible-content">
						<div
							id="signalStationRouteCollapsibleContent"
							class="content-inner"
						>
							<template
								v-for="(signalStationRoute, index) in signalStationRoutes"
								:key="index"
							>
								<label class="matter-switch station-detail-signal-route">
									<input
										type="checkbox"
										:value="signalStationRoute.id"
										:checked="highlightedSignalStationRoutes.find(f => f.properties.id === signalStationRoute.id)"
										@change="onSignalStationRouteInput"
									>
									<span>{{ `ID: ${signalStationRoute.id}` }}</span>
								</label>
							</template>
						</div>
					</div>
				</div>
			</div>
		</div>
	</div>
</template>

<script>
import { mapMutations } from 'vuex';
import { InfrastructureNamespace } from '../stores/infrastructure-store.js';
import { iterate } from '../util/iterate.js';

document.getElementById('subOverlayContent').innerHTML = ''; // TODO do in App.vue

export default {
	name: 'StationDetail',

	data() {
		return {
			station: undefined,
			stationRoutes: [],
			highlightedStationRoutes: [],
			signalStationRoutes: [],
			highlightedSignalStationRoutes: [],
		};
	},

	methods: {
		onStationRouteInput(event) {
			if (event.target.checked) {
				this.setHighlightedStationRouteID(Number(event.target.value)); // TODO own soro-checkbox with abstraction events
			} else {
				this.setHighlightedStationRouteID(undefined);
			}
		},

		onSignalStationRouteInput(event) {
			if (event.target.checked) {
				this.setHighlightedSignalStationRouteID(Number(event.target.value));
			} else {
				this.setHighlightedSignalStationRouteID(undefined);
			}
		},

		fillStation(station, infrastructure, highlightedStationRoutes, highlightedSignalStationRoutes) {
			this.stationRoutes = [];
			this.highlightedStationRoutes = highlightedStationRoutes;
			for (let i = 0; i < station.station_routes.size(); i++) {
				const key = station.station_routes.keys().get(i);
				const sr = station.station_routes.get(key);

				this.stationRoutes.push({ key, sr });
			}

			this.signalStationRoutes = [];
			this.highlightedSignalStationRoutes = highlightedSignalStationRoutes;
			for (const ssr of iterate(infrastructure.station_to_ssrs.get(station.id))) {
				this.signalStationRoutes.push(ssr);
			}

			// showSubOverlay(); // TODO
		},

		...mapMutations(InfrastructureNamespace, [
			'setHighlightedStationRouteID',
			'setHighlightedSignalStationRouteID',
		]),
	}
};
</script>

<style>
.station-detail {
  margin-top: 10px;

  background: transparent;
  color: #777;
  width: 100%;
  height: 100%;
  box-shadow: 3px 3px 2px rgba(0, 0, 0, 0.2);
  cursor: pointer;
  display: flex;
  align-items: center;
  justify-content: center;
  opacity: 1;
  transition: all 0.2s ease;
  flex-direction: column;
  visibility: visible;
}

.station-detail.hidden {
  left: calc(0px - calc(var(--overlay-width) + var(--overlay-padding-left)));
}

.station-detail-name {
  font-family: var(--main-font-family);
  font-weight: bold;
  font-size: 20px;
  justify-content: center;
  align-items: center;
  color: var(--text-color);
  flex-direction: column;
}

.station-detail-routes {
  display: flex;
  flex-direction: column;
  width: 100%;
  height: 90%;
}

.station-detail-route {
  width: calc(100% - 1em);
  background: var(--dialog-color);
  border: 0.4em solid var(--dialog-color);
}

.station-detail-route:nth-child(even) {
  background: var(--overlay-color);
  border: 0.4em solid var(--overlay-color);
  border-right: 0;
}

.station-detail-signal-routes {
  display: flex;
  flex-direction: column;
  width: 100%;
  height: 90%;
}

.station-detail-signal-route {
  width: calc(100% - 1em);
  background: var(--dialog-color);
  border: 0.4em solid var(--dialog-color);
}

.station-detail-signal-route:nth-child(even) {
  background: var(--overlay-color);
  border: 0.4em solid var(--overlay-color);
  border-right: 0;
}
</style>