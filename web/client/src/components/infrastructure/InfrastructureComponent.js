import { getFileContents } from "../../../utl/getFileContents.js";
import { ClickTooltip } from "../../../utl/Tooltip.js";
import {
  createMap,
  deHighlightSignalStationRoute,
  deHighlightStationRoute,
  highlightSignalStationRoute,
  highlightStationRoute
} from "./map/infrastructureMap.js";

export class InfrastructureComponent {
  _libreGLMap = undefined;
  _tooltip = undefined;

  constructor(container, componentState) {
    this.container = container;
    this.rootElement = container.element;

    getFileContents("./components/infrastructure/infrastructure_component.html")
      .then((html) => {
        this.rootElement.innerHTML = html;
      }).then(() => {

      this._tooltip = new ClickTooltip(this.rootElement, 'infrastructureTooltip');

      container.on('resize', () => this._libreGLMap?.resize());
    }).then(() => {
      this.changeInfrastructure(componentState.getCurrentInfrastructure());
    });
  }

  changeInfrastructure(newInfrastructureName) {
    this._libreGLMap = newInfrastructureName
      ? createMap(this.rootElement, newInfrastructureName, this._tooltip)
      : undefined;
  }

  highlightSignalStationRoute(signalStationRouteID) {
    highlightSignalStationRoute(this._libreGLMap, window.infrastructureManager.get(), signalStationRouteID);
  }

  deHighlightSignalStationRoute(signalStationRouteID) {
    deHighlightSignalStationRoute(this._libreGLMap, window.infrastructureManager.get(), signalStationRouteID);
  }

  highlightStationRoute(stationRouteID) {
    highlightStationRoute(this._libreGLMap, window.infrastructureManager.get(), stationRouteID);
  }

  deHighlightStationRoute(stationRouteID) {
    deHighlightStationRoute(this._libreGLMap, stationRouteID);
  }
}
