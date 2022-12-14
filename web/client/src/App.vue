<template>
	<div class="full-height">
		<div
			ref="overlayContainer"
			:class="overlayContainerClasses"
		>
			<div class="overlay">
				<div class="overlay-content">
					<div class="window-controls">
						<button
							class="matter-button-contained window-button"
							@click="addInfrastructureTab"
						>
							Infrastructure
						</button>
						<button
							class="matter-button-contained window-button"
							disabled
						>
							Simulation
						</button>
						<button
							class="matter-button-contained window-button"
							disabled
						>
							Timetable
						</button>
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
						<button
							class="matter-button-contained window-button"
							disabled
						>
							Clear Cache
						</button>
						<button
							class="matter-button-contained window-button"
							disabled
						>
							Simulate
						</button>
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
			style="width: 100%; height: calc(100% - 90px)"
		/>
	</div>
</template>

<script setup lang="ts">
import GoldenLayoutAdapter from './golden-layout/golden-layout-adapter.vue';
import DisruptionDetail from './components/disruption-detail.vue';
import SoroSelect from './components/soro-select.vue';
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
html {
  height: 100%;
}
body {
  height: 100%;
  margin: 0;
  overflow: hidden;
}
.full-height, #app {
  height: 100%;
}
#app {
  font-family: Avenir, Helvetica, Arial, sans-serif;
  -webkit-font-smoothing: antialiased;
  -moz-osx-font-smoothing: grayscale;
}
</style>
