import { getStationCoordPath } from "../utl/IDBFSHelper.js";
import { getInfrastructureComponents } from "../utl/goldenLayoutHelper.js";
import { Module } from "../soro-client.js";

function loadInfrastructureFromIDBFS(filePath) {
  /* Loading from cache is very slow atm, so disabled
   const cachePath = new Module.FilesystemPath('/idbfs/cache/' + infrastructureFilename);
   if (exists(cachePath.string())) {
     this._current = new Module.Infrastructure(cachePath, true);
   } else {
   */

  let infraOpts = new Module.InfrastructureOptions();
  infraOpts.determine_conflicts = true;
  infraOpts.infrastructure_path = new Module.FilesystemPath(filePath);
  infraOpts.gps_coord_path = new Module.FilesystemPath(getStationCoordPath());

  return new Module.Infrastructure(infraOpts);

  /* try {
     this._current.save(cachePath);
     saveToPersistent();
   } catch (e) {
     console.error("Could not write serialized cache, maybe compiled without SERIALIZE?");
     console.error(e);
   }
}
   */
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
    if (!Module.validSSRID(signalStationRouteID)) {
      return;
    }

    for (const infrastructureComponent of getInfrastructureComponents()) {
      infrastructureComponent.highlightSignalStationRoute(signalStationRouteID);
    }
  }

  deHighlightSignalStationRoute(signalStationRouteID) {
    if (!Module.validSSRID(signalStationRouteID)) {
      return;
    }

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