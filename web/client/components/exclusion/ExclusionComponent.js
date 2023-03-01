import {getFileContents} from "../../utl/getFileContents.js";

const interlockingRouteSwitchCallback = e => {
    if (e.target.checked) {
        window.infrastructureManager.highlightInterlockingRoute(Number(e.target.value));
    } else {
        window.infrastructureManager.deHighlightInterlockingRoute(Number(e.target.value));
    }
}

function expandExclusionSet(contentDiv, exclusionSetId) {

    const fillExclusionSet = interlockingRouteIds => {
        let newRoutes = [];
        for (const interlockingRouteId of interlockingRouteIds) {
            let route = document.createElement('label');
            route.classList.add('matter-switch', 'station-detail-route');

            let input = document.createElement('input');
            input.type = 'checkbox';
            input.value = interlockingRouteId;
            // input.checked = highlightedStationRoutes.find(f => f.properties.id === stationRoute.id);
            input.addEventListener('input', interlockingRouteSwitchCallback);

            let span = document.createElement('span');
            span.innerHTML = interlockingRouteId;

            route.appendChild(input);
            route.appendChild(span);
            newRoutes.push(route);
        }

        contentDiv.replaceChildren(...newRoutes);
    };


    const currentInfrastructureName = window.infrastructureManager.get();
    const url = window.origin + '/infrastructure/' + currentInfrastructureName + '/exclusion_sets/' + exclusionSetId;

    fetch(url).then(response => response.json())
        .then(exclusionSet => fillExclusionSet(exclusionSet.interlocking_routes));
}

function toggleExclusionSet(event) {
    console.log('toggle exclusion set', event);
}

function fillExclusionSets(exclusionSetsDiv, exclusionSets) {

    let newExclusionSets = [];
    for (const exclusionSet of exclusionSets) {
        let exclusionSetDiv = document.createElement('div');
        exclusionSetDiv.id = 'es' + exclusionSet.id;
        exclusionSetDiv.classList.add('exclusion-set');

        let wrapCollapsible = document.createElement('div');
        wrapCollapsible.classList.add('wrap-collapsible');

        let exclusionSetInput = document.createElement('input');
        exclusionSetInput.classList.add('collapsible-toggle', 'hidden');
        exclusionSetInput.id = exclusionSetDiv.id + "-collapsible";
        exclusionSetInput.type = 'checkbox';

        let exclusionSetLabel = document.createElement('label');
        exclusionSetLabel.htmlFor = exclusionSetInput.id;
        exclusionSetLabel.innerHTML = 'Exclusion Set ' + exclusionSet.id + ' | Size: ' + exclusionSet.size;
        exclusionSetLabel.classList.add('collapsible-toggle');

        let exclusionSetContent = document.createElement('div');
        exclusionSetContent.classList.add('content-inner');
        exclusionSetContent.id = exclusionSetDiv.id + '-content';

        let exclusionSetSwitch = document.createElement('label');
        exclusionSetSwitch.classList.add('matter-switch');

        let exclusionSetSwitchInput = document.createElement('input');
        exclusionSetSwitchInput.type = 'checkbox';

        let exclusionSetSwitchSpan = document.createElement('span');

        exclusionSetInput.addEventListener('input', () => expandExclusionSet(exclusionSetContent, exclusionSet.id));
        exclusionSetSwitchInput.addEventListener('input', toggleExclusionSet);

        exclusionSetSwitch.appendChild(exclusionSetSwitchInput);
        exclusionSetSwitch.appendChild(exclusionSetSwitchSpan);

        exclusionSetDiv.appendChild(wrapCollapsible);
        wrapCollapsible.appendChild(exclusionSetInput);
        wrapCollapsible.appendChild(exclusionSetLabel);
        wrapCollapsible.appendChild(exclusionSetContent)

        exclusionSetDiv.appendChild(exclusionSetSwitch);

        newExclusionSets.push(exclusionSetDiv);
    }

    exclusionSetsDiv.replaceChildren(...newExclusionSets);
}

function initializeExclusionComponent() {
    const currentInfrastructureName = window.infrastructureManager.get();

    if (!currentInfrastructureName) {
        return;
    }

    let exclusionSetsDiv = document.getElementById("exclusion-sets");

    const url = window.origin + '/infrastructure/' + currentInfrastructureName + '/exclusion_sets/';
    fetch(url).then(response => response.json())
        .then(exclusionSets => fillExclusionSets(exclusionSetsDiv, exclusionSets.exclusion_sets));
}

export class ExclusionComponent {
    constructor(container) {
        this.container = container;
        this.rootElement = container.element;

        getFileContents("./components/exclusion/exclusion_component.html")
            .then((html) => {
                this.rootElement.innerHTML = html;
            }).then(() => initializeExclusionComponent());

    }
}
