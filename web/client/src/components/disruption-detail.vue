<template>
	<div class="disruption-detail hidden">
		Disruption:
		<div>
			Maximum Speeds [km/h]
			<br>
			<template
				v-for="trainRun in iterate(currentTimetable.trainRuns)"
				:key="trainRun.name"
			>
				<label :for="`${trainRun.name}Input`">{{ trainRun.name }}</label>
				<input
					:id="`${trainRun.name}Input`"
					:name="trainRun.name"
					:value="disruptionMap.get(trainRun.name)"
					type="text"
					@input="(e) => disruptionMap.set(e.target.name, e.target.value)"
				>
				<br>
				<br>
			</template>
			<label class="matter-switch">
				<input
					v-model="useDistributions"
					type="checkbox"
					value="UseDists"
				>
				<span>Use Distributions</span>
			</label>
		</div>
	</div>
</template>

<script>
import { iterate } from '../util/iterate.js';
import { mapState } from 'vuex';
import { TimetableNamespace } from '../stores/timetable-store';

const disruptionMapDefaults = {
	1: 80,
	2: 120,
};
const useDistributions = false;

export default {
	name: 'DisruptionDetail',

	data() {
		return {
			useDistributions,
			disruptionMap: new Map(),
		};
	},

	computed: mapState(TimetableNamespace, ['currentTimetable']),

	created() {
		Object.keys(disruptionMapDefaults).forEach((key) => this.disruptionMap.set(key, disruptionMapDefaults[key]));
	},

	methods: { iterate },
};
</script>

<style scoped>
.disruption-detail {
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

.disruption-detail.hidden {
  left: calc(0px - calc(var(--overlay-width) + var(--overlay-padding-left)));
}
</style>