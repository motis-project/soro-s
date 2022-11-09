import { getInfrastructureComponents } from "../utl/goldenLayoutHelper.js";

function loadInfrastructureFromIDBFS(filePath) {
  console.log("This is to be removed");
}

export class InfrastructureManager {
  _current = undefined;

  constructor() {
  }

  updateInfrastructureComponents() {
    for (const infrastructureComponent of getInfrastructureComponents()) {
      infrastructureComponent.changeInfrastructure(this._current);
    }
  }

  get() {
    return this._current;
  }

  load(infrastructureFilename) {
    console.log("Switching to infrastructure to", infrastructureFilename);
    this._current = infrastructureFilename;
    this.updateInfrastructureComponents();
  }

  unload() {
    this._current = undefined;
    this.updateInfrastructureComponents();
  }

  addInfrastructureComponent() {
    window.goldenLayout.addComponent('InfrastructureComponent', {
      getCurrentInfrastructure: () => {
        return this._current
      }
    }, 'Infrastructure')
  }

  highlightSignalStationRoute(signalStationRouteID) {
    for (const infrastructureComponent of getInfrastructureComponents()) {
      infrastructureComponent.highlightSignalStationRoute(signalStationRouteID);
    }
  }

  deHighlightSignalStationRoute(signalStationRouteID) {
    for (const infrastructureComponent of getInfrastructureComponents()) {
      infrastructureComponent.deHighlightSignalStationRoute(signalStationRouteID);
    }
  }

  highlightStationRoute(stationRouteID) {
    for (const infrastructureComponent of getInfrastructureComponents()) {
      infrastructureComponent.highlightStationRoute(stationRouteID);
    }
  }

  deHighlightStationRoute(stationRouteID) {
    for (const infrastructureComponent of getInfrastructureComponents()) {
      infrastructureComponent.deHighlightStationRoute(stationRouteID);
    }
  }
}