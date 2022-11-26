import { getFileContents } from "../../util/getFileContents.js";
import { iterate } from "../../util/iterate.js";

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

export function showDisruptionDetail(currentTimetable) {
  getFileContents("./components/disruption/disruption.html")
    .then(html => {
      document.getElementById('subOverlayContent').innerHTML = html;

      let maxSpeeds = document.getElementById('trainMaxSpeeds');
      fillDisruptionDetail(currentTimetable, maxSpeeds);
    });
}