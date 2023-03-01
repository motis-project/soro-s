import {getFileContents} from "../../../utl/getFileContents.js";
import {iterate} from "../../../utl/iterate.js";

const stationRouteSwitchCallback = e => {
    if (e.target.checked) {
        window.infrastructureManager.highlightStationRoute(Number(e.target.value));
    } else {
        window.infrastructureManager.deHighlightStationRoute(Number(e.target.value));
    }
}

const interlockingRouteSwitchCallback = e => {
    if (e.target.checked) {
        window.infrastructureManager.highlightInterlockingRoute(Number(e.target.value));
    } else {
        window.infrastructureManager.deHighlightInterlockingRoute(Number(e.target.value));
    }
}

function fillStation(station, highlightedStationRoutes, highlightedInterlockingRoutes) {
    let stationDetailName = document.getElementById('stationDetailName');
    stationDetailName.innerText = station.ds100;

    let routeContent = document.getElementById('stationRouteCollapsibleContent');
    let newRoutes = [];
    for (const stationRoute of station.station_routes) {
        let route = document.createElement('label');
        route.classList.add('matter-switch', 'station-detail-route');

        let input = document.createElement('input');
        input.type = 'checkbox';
        input.value = stationRoute.id;
        input.checked = highlightedStationRoutes.find(f => f.properties.id === stationRoute.id);
        input.addEventListener('input', stationRouteSwitchCallback);

        let span = document.createElement('span');
        span.innerHTML = stationRoute.name;

        route.appendChild(input);
        route.appendChild(span);
        newRoutes.push(route);
    }

    routeContent.replaceChildren(...newRoutes);

    let newSignalStationRoutes = [];
    for (const interlockingRouteId of station.interlocking_routes) {
        let route = document.createElement('label');
        route.classList.add('matter-switch', 'station-detail-signal-route');

        let input = document.createElement('input');
        input.type = 'checkbox';
        input.value = interlockingRouteId;
        input.checked = highlightedInterlockingRoutes.find(f => f.properties.id === interlockingRouteId);
        input.addEventListener('input', interlockingRouteSwitchCallback);

        let span = document.createElement('span');
        span.innerHTML = 'ID: ' + interlockingRouteId;

        route.appendChild(input);
        route.appendChild(span);
        newSignalStationRoutes.push(route);
    }

    let signalStationRouteContent = document.getElementById('signalStationRouteCollapsibleContent');
    signalStationRouteContent.replaceChildren(...newSignalStationRoutes);

    showSubOverlay();
}

export function showStation(station, highlightedStationRoutes, highlightedSignalStationRoutes) {
    getFileContents("./components/infrastructure/station_detail/station_detail.html")
        .then(html => {
            document.getElementById('subOverlayContent').innerHTML = html;
            fillStation(station, highlightedStationRoutes, highlightedSignalStationRoutes);
        });
}
