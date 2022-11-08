import { select } from "../deps/d3/d3-selection.js";

function makeIntoPermanent(mouseTooltip) {
  // on rightclick dismiss permanent tooltip
  const rightclick = event => {
    event.preventDefault();
    mouseTooltip.remove();
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

      mouseTooltip
        .style('left', (mouseTooltip.node().offsetLeft - deltaX) + 'px')
        .style('top', (mouseTooltip.node().offsetTop - deltaY) + 'px');
    }
  }

  mouseTooltip
    .on('mouseout', undefined) // make tooltip permanent
    .on('contextmenu', rightclick)
    .on('mousedown', mousedown);
}

class Tooltip {
  constructor(rootElement, tooltipDiv) {
    this._mouse_tooltip = select(rootElement.querySelector('#' + tooltipDiv))
      .style('position', 'absolute');
  }

  setHTMLContent(content) {
    this._mouse_tooltip.html(content);
  }

  show(x, y) {
    this._mouse_tooltip.style('display', 'inline')
      .style('left', x + 'px')
      .style('top', y + 'px')
  }

  hide() {
    this._mouse_tooltip.style('display', 'none');
  }

  select(selectString) {
    return this._mouse_tooltip.select(selectString);
  }

  _mouse_tooltip = undefined;
}

export class ClickTooltip extends Tooltip {
  constructor(rootElement, tooltipDiv) {
    super(rootElement, tooltipDiv);
  }

  click(event, getX, getY) {
    super.show(getX(event), getY(event));

    let permanentTooltip = this._mouse_tooltip.clone(true);
    makeIntoPermanent(permanentTooltip);
    super.hide();
  }
}

export class HoverTooltip extends Tooltip {
  constructor(rootElement, tooltipDiv) {
    super(rootElement, tooltipDiv);
  }

  mouseover(X, Y) {
    // super.show(event.layerX + 10, event.layerY + 10);
    super.show(X, Y);

    // const simNode = event.target.__data__;
    //
    // this._mouse_tooltip.select('#tooltipID').text('ID: ' + simNode.ID);
    // this._mouse_tooltip.select('#tooltipSSRID').text('SSR ID: ' + simNode.SSRID);

    d3.select(e.target).style('stroke', 'black');
  }

  mousemove(event, getX, getY) {
    super.show(getX(event), getY(event));
    // super.show(event.layerX + 10, event.layerY + 10);
  }

  mouseout(event) {
    this.hide();

    // event.target.style.stroke = 'none';
  }

  click() {
    let permanentTooltip = this._mouse_tooltip.clone(true);
    makeIntoPermanent(permanentTooltip);
  }
}