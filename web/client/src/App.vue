<template>
	<div class="full-height">
		<div
			ref="overlayContainer"
			:class="overlayContainerClasses"
		>
			<div class="overlay">
				<div class="overlay-content">
					<div class="window-controls">
						<soro-button
							label="New Infrastructure Tab"
							@click="addInfrastructureTab"
						/>
						<soro-button
							disabled
							label="New Simulation Tab"
						/>
						<soro-button
							disabled
							label="New Timetable Tab"
						/>
					</div>
					<div class="data-selects">
						<soro-select
							label="Select Infrastructure"
							:value="currentInfrastructure"
							:options="infrastructures"
							@select="loadInfrastructure"
						/>
						<soro-select
							label="Select Timetable"
							:value="currentTimetable"
							:options="timetables"
							@select="loadTimetable"
						/>
					</div>
					<div class="dev-tools">
						<soro-button
							disabled
							label="Clear Cache"
						/>
						<soro-button
							disabled
							label="Simulate"
						/>
					</div>
				</div>
				<div
					ref="subOverlay"
					:class="subOverlayClasses"
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
			<div
				ref="subOverlayTabs"
				class="overlay-tabs"
			>
				<div class="overlay-toggle">
					<button
						class="matter-button-contained overlay-toggle-button"
						@click="toggleOverlay"
					>
						<i
							class="material-icons"
							style="display: flex; justify-content:center;"
						>menu</i>
					</button>
				</div>
				<div
					ref="stationDetailButton"
					class="sub-overlay-tab-button"
				>
					<i class="material-icons">home</i>
				</div>
				<div
					ref="disruptionDetailButton"
					class="sub-overlay-tab-button"
				>
					<i class="material-icons">train</i>
				</div>
			</div>
		</div>
		<golden-layout-adapter
			ref="GLayoutRoot"
			class="golden-layout-root"
		/>
	</div>
</template>

<script setup lang="ts">
import GoldenLayoutAdapter from './golden-layout/golden-layout-adapter.vue';
import DisruptionDetail from './components/disruption-detail.vue';
import SoroSelect from './components/soro-select.vue';
import SoroButton from './components/soro-button.vue';
</script>

<script lang="ts">
import { mapActions, mapState } from 'vuex';
import { InfrastructureNamespace } from './stores/infrastructure-store';
import { TimetableNamespace } from './stores/timetable-store';
import { defineComponent, ref } from 'vue';
import { LayoutConfig } from 'golden-layout';
import { Components } from './golden-layout/golden-layout-constants';

const initLayout: LayoutConfig = {
	root: {
		type: 'row',
		content: [
			{
				type: 'column',
				content: [
					{
						title: 'Infrastructure',
						type: 'component',
						componentType: 'InfrastructureComponent',
					},
				]
			}
		]
	}
};

const GLayoutRoot = ref();

export default defineComponent({
	data() {
		return {
			overlay: false,
		};
	},

	computed: {
		overlayContainerClasses() {
			return `overlay-container ${this.overlay ? '' : 'hidden'}`;
		},

		subOverlayClasses() {
			return 'sub-overlay hidden';
		},

		...mapState(InfrastructureNamespace, [
			'currentInfrastructure',
			'infrastructures',
		]),
		...mapState(TimetableNamespace, [
			'currentTimetable',
			'timetables',
		]),
	},

	mounted() {
		this.loadInfrastructures();
		this.loadTimetables();
		GLayoutRoot.value.loadGLLayout(initLayout);
	},

	methods: {
		toggleOverlay() {
			this.overlay = !this.overlay;
		},

		addInfrastructureTab() {
			GLayoutRoot.value.addGLComponent(Components.Infrastructure, 'Infrastructure');
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

<style>
.golden-layout-root {
  width: 100%;
  height: calc(100% - 90px);
}

.full-height {
  height: 100%;
}

.overlay-container {
  display: flex;
  align-items: flex-start;
  justify-content: center;
  height: 100%;
  padding-left: var(--overlay-padding-left);
  padding-top: var(--overlay-padding-top);
  padding-bottom: 20px;
  position: absolute;
  top: 0;
  left: 0;
  transition: all .4s ease;
  pointer-events: none;
}

.overlay-container.hidden {
  left: calc(0px - calc(var(--overlay-width) + var(--overlay-padding-left)));
}

.overlay {
  z-index: 10;
  background-color: var(--overlay-color);
  box-shadow: 0 10px 20px rgba(0, 0, 0, 0.19), 0 6px 6px rgba(0, 0, 0, 0.23);
  border-radius: 3px;
  border-top-right-radius: 0;
  transition: all .4s ease;
  width: var(--overlay-width);
  flex: 0 0 auto;
  order: 1;
  height: 95%;
  position: relative;
}

.overlay-container.hidden .overlay {
  box-shadow: unset;
}

.overlay-content {
  display: flex;
  flex-direction: column;
  height: 100%;
  pointer-events: auto;
  position: relative;
}

.overlay-tabs {
  z-index: 10;
  order: 2;
  pointer-events: auto;
  display: flex;
  flex-direction: column;
}

.overlay-toggle-button {
  box-shadow: 0 10px 20px rgba(0, 0, 0, 0.19), 0 6px 6px rgba(0, 0, 0, 0.23);
  border-bottom-left-radius: 0;
  border-top-left-radius: 0;
}

.sub-overlay {
  position: absolute;
  top: 0;
  left: 0;
  padding: 0.8em;
  width: calc(100% - 1.6em);
  height: calc(100% - 1.6em);
  transition: all .2s ease;
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
  box-shadow: 0 6px 6px rgba(0, 0, 0, 0.23), 0 -2px 6px rgba(0, 0, 0, 0.23);
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

.window-controls, .dev-tools {
  display: flex;
  flex-direction: column;
  flex-wrap: wrap;
  width: 94%;
  justify-content: space-around;
  padding: 3%;
  margin-top: 0.5em;
  margin-bottom: 0.5em;
}

.data-selects {
  display: flex;
  flex-direction: column;
  flex-wrap: wrap;
  width: 94%;
  justify-content: space-around;
  padding: 3%;
  margin-top: 0.5em;
  margin-bottom: 0.5em;
}

.data-select {
  margin-top: 2.2em;
  margin-bottom: 2.2em;
}

/*  ============= Station Detail ============= */

.sub-overlay-tab-button {
  width: 30px;
  height: 40px;
  margin-top: 10px;
  background: var(--dialog-color);
  color: var(--secondary-text-color);
  box-shadow: 3px 3px 2px rgba(0, 0, 0, 0.2);
  cursor: pointer;
  display: flex;
  align-items: center;
  justify-content: center;
  opacity: 1;
  transition: all 0.2s ease;
}

.sub-overlay-tab-button.enabled {
  color: white;
  background: var(--highlight-color);
}
</style>
