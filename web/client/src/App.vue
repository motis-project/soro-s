<template>
	<div class="full-height">
		<soro-overlay @add-golden-layout-tab="addGoldenLayoutTab" />
		<golden-layout-adapter
			ref="GLayoutRoot"
			class="golden-layout-root"
		/>
	</div>
</template>

<script setup lang="ts">
import GoldenLayoutAdapter from './golden-layout/golden-layout-adapter.vue';
import SoroOverlay from './components/soro-overlay.vue';
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
		addGoldenLayoutTab({ component, title }: { component: typeof Components[keyof typeof Components], title: string}) {
			GLayoutRoot.value.addGLComponent(component, title);
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

<style scoped>
.golden-layout-root {
  width: 100%;
  height: calc(100% - 90px);
}

.full-height {
  height: 100%;
}
</style>
