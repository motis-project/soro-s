<template>
  <div id="disruptionDetail" class="disruption-detail hidden">
    Disruption:
    <div id="trainMaxSpeeds" ref="trainMaxSpeeds">
      Maximum Speeds [km/h]
      <br>
    </div>
  </div>
</template>

<script>
import { iterate } from '../util/iterate.js';
import { mapState } from 'vuex';
import { TimetableNamespace } from '../stores/timetable-store';

const disruptionMap = new Map();
disruptionMap.set('1', 80);
disruptionMap.set('2', 120);
let disruptionDists = false;

function createInputField(name, maxSpeedsDiv) {
	let input = document.createElement('input');
	input.type = 'text';
	input.id = name + 'Input';
	input.name = name;
	input.value = disruptionMap.get(name);

	input.addEventListener('input',
		e => disruptionMap.set(e.target.name, e.target.value));

	let label = document.createElement('label');
	label.htmlFor = input.id;
	label.innerText = name + ' ';

	let br = document.createElement('br');

	maxSpeedsDiv.appendChild(label);
	maxSpeedsDiv.appendChild(input);
	maxSpeedsDiv.appendChild(br);
	maxSpeedsDiv.appendChild(br);

}

function fillDisruptionDetail(currentTimetable, maxSpeedsDiv) {
	for (const train_run of iterate(currentTimetable.trainRuns)) {
		createInputField(train_run.name, maxSpeedsDiv);
	}

	let br = document.createElement('br');

	let distsLabel = document.createElement('label');
	distsLabel.classList.add('matter-switch');

	let distsInput = document.createElement('input');
	distsInput.type = 'checkbox';
	distsInput.value = 'UseDists';
	distsInput.checked = disruptionDists;
	distsInput.addEventListener('input', e => disruptionDists = e.target.checked);

	let span = document.createElement('span');
	span.innerHTML = 'Use Distributions';

	distsLabel.append(distsInput);
	distsLabel.append(span);
	maxSpeedsDiv.appendChild(distsLabel);
}

export default {
	name: 'disruption-detail',

	computed: mapState(TimetableNamespace, ['currentTimetable']),

	methods: {
		showDetail() {
			fillDisruptionDetail(this.currentTimetable, this.$refs.trainMaxSpeeds);
		},
	},
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