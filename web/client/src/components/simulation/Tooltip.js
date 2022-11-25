import * as d3 from "../../../deps/d3/d3.js";

function cloneIntoPermanent(mouseTooltip, highlightedSSR) {
  let permanentTooltip = mouseTooltip.clone(true);

  // on rightclick dismiss permanent tooltip
  const rightclick = event => {
    event.preventDefault();
    permanentTooltip.remove();
    window.infrastructureManager.deHighlightSignalStationRoute(highlightedSSR);
  }

  // make the permanent tooltip draggable
  const mousedown = event => {
    event.preventDefault();

    // when the mouse is released stop dragging the element
    document.onmouseup = () => {
      document.onmouseup = undefined;
      document.onmousemove = undefined;
    }

    // update the position on mouse movement
    let deltaX = 0, deltaY = 0, tmpX = event.clientX, tmpY = event.clientY;
    document.onmousemove = event => {
      event.preventDefault();

      deltaX = tmpX - event.clientX;
      deltaY = tmpY - event.clientY;
      tmpX = event.clientX;
      tmpY = event.clientY;

      permanentTooltip
        .style('left', (permanentTooltip.node().offsetLeft - deltaX) + 'px')
        .style('top', (permanentTooltip.node().offsetTop - deltaY) + 'px');
    }
  }

  permanentTooltip
    .on('mouseout', undefined) // make tooltip permanent
    .on('contextmenu', rightclick)
    .on('mousedown', mousedown);
}

export class Tooltip {
  constructor(rootElement) {
    this._mouse_tooltip = d3.select(rootElement.querySelector('#toolTip'))
      .style('position', 'absolute');
  }

  mouseover(event) {
    this._mouse_tooltip.style('display', 'inline')
      .style('left', (event.layerX + 10) + 'px')
      .style('top', (event.layerY + 10) + 'px')

    const simNode = event.target.__data__;

    this._mouse_tooltip.select('#tooltipID').text('ID: ' + simNode.ID);
    this._mouse_tooltip.select('#tooltipSSRID').text('SSR ID: ' + simNode.SSRID);

    window.infrastructureManager.highlightSignalStationRoute(simNode.SSRID);

    d3.select(event.target).style('stroke', 'black');
  }

  mousemove(event) {
    this._mouse_tooltip
      .style('left', (event.layerX + 10) + 'px')
      .style('top', (event.layerY + 10) + 'px');
  }

  mouseout(event) {
    this._mouse_tooltip.style('display', 'none');
    event.target.style.stroke = 'none';

    const simNode = event.target.__data__;
    window.infrastructureManager.deHighlightSignalStationRoute(simNode.SSRID);
  }

  click(e) {
    const ssrID = e.target.__data__.SSRID;
    cloneIntoPermanent(this._mouse_tooltip, ssrID);
    window.infrastructureManager.highlightSignalStationRoute(ssrID);
  }

  _mouse_tooltip = undefined;
}