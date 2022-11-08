import { getFileContents } from "../../../utl/getFileContents.js";
import { iterate } from "../../../utl/iterate.js";

const routeSwitchCallback = e => {
  if (e.target.checked) {
    window.infrastructureManager.highlightStationRoute(Number(e.target.value));
  } else {
    window.infrastructureManager.deHighlightStationRoute(Number(e.target.value));
  }
}

const signalRouteSwitchCallback = e => {
  if (e.target.checked) {
    window.infrastructureManager.highlightSignalStationRoute(Number(e.target.value));
  } else {
    window.infrastructureManager.deHighlightSignalStationRoute(Number(e.target.value));
  }
}

function fillStation(station, infrastructure, highlightedStationRoutes, highlightedSignalStationRoutes) {
  let stationDetailName = document.getElementById('stationDetailName');
  stationDetailName.innerText = station.name;

  let routeContent = document.getElementById('stationRouteCollapsibleContent');
  let newRoutes = [];
  for (let i = 0; i < station.station_routes.size(); i++) {
    const key = station.station_routes.keys().get(i);
    const sr = station.station_routes.get(key);

    let route = document.createElement('label');
    route.classList.add('matter-switch', 'station-detail-route');

    let input = document.createElement('input');
    input.type = 'checkbox';
    input.value = sr.id;
    input.checked = highlightedStationRoutes.find(f => f.properties.id === sr.id);
    input.addEventListener('input', routeSwitchCallback);

    let span = document.createElement('span');
    span.innerHTML = key;

    route.appendChild(input);
    route.appendChild(span);
    newRoutes.push(route);
  }

  routeContent.replaceChildren(...newRoutes);

  let newSignalStationRoutes = [];
  for (const ssr of iterate(infrastructure.station_to_ssrs.get(station.id))) {
    let route = document.createElement('label');
    route.classList.add('matter-switch', 'station-detail-signal-route');

    let input = document.createElement('input');
    input.type = 'checkbox';
    input.value = ssr.id;
    input.checked = highlightedSignalStationRoutes.find(f => f.properties.id === ssr.id);
    input.addEventListener('input', signalRouteSwitchCallback);

    let span = document.createElement('span');
    span.innerHTML = 'ID: ' + ssr.id;

    route.appendChild(input);
    route.appendChild(span);
    newSignalStationRoutes.push(route);
  }

  let signalStationRouteContent = document.getElementById('signalStationRouteCollapsibleContent');
  signalStationRouteContent.replaceChildren(...newSignalStationRoutes);

  showSubOverlay();
}

export function showStation(station, infrastructure, highlightedStationRoutes, highlightedSignalStationRoutes) {
  getFileContents("./components/infrastructure/station_detail/station_detail.html")
    .then(html => {
      document.getElementById('subOverlayContent').innerHTML = html;
      fillStation(station, infrastructure, highlightedStationRoutes, highlightedSignalStationRoutes);
    });
}
