import * as d3 from "../../deps/d3/d3.js";
import { getFileContents } from "../../utl/getFileContents.js";
import { Module } from "../../soro-client.js";
import { iterate } from "../../utl/iterate.js";
import { Tooltip } from "./Tooltip.js";
import { iterateDist } from "../../utl/iterate.js";
import { timeFormat } from "../../deps/d3/d3.js";

const nodeWidth = 100;
const nodeHeight = 80;
const nodeDiagonal = Math.sqrt(Math.pow(nodeWidth, 2) + Math.pow(nodeHeight, 2));
const nodeSpacingHorizontal = nodeWidth * 0.5;
const nodeSpacingVertical = nodeHeight * 0.5;

const outerWidthPadding = nodeWidth * 0.1;
const outerHeightPadding = nodeHeight * 0.1;

const innerNodeCount = 3;

const innerNodeWidth = (nodeWidth - outerWidthPadding * 2);
const innerNodeHeight = (nodeHeight - outerHeightPadding * 2) / innerNodeCount;

const innerHeightPadding = innerNodeHeight * 0.1;

class d3Graph {
  constructor(rootElement, width, height) {
    this._width = width;
    this._height = height;

    let selected = d3.select(rootElement.querySelector('#simGraph'));

    this._svg = selected
      .append("svg")
      .attr("width", "100%")
      .attr("height", "100%")
      .attr("viewBox", [0, 0, 1000, 1000]);

    this._svg.call(d3.zoom().scaleExtent([0, 1000]).on("zoom", ({transform}) => {
      this._d3_graph.attr("transform", transform);
    }));

    this._d3_graph = this._svg
      .append("g");

    let selectedDistGraph = d3.select(rootElement.querySelector('#distGraph'));

    this._dist_svg = selectedDistGraph.append("svg")
      .attr("width", "100%")
      .attr("height", "100%")
      .attr("viewBox", "0 0 1000 1000");

    this._tooltip = new Tooltip(rootElement);
  }

  resizeSVG(width, height) {
    this._svg.attr("width", width / 2)
      .attr("height", height)
      .attr("viewBox", [0, 0, width / 2, height]);

    this._dist_svg.attr("width", width / 2)
      .attr("height", height)
      .attr("viewBox", [0, 0, width / 2, height]);
  }

  // upper left X
  getX(nodeID) {
    const node = this._sim_graph.nodes.get(nodeID);
    const idRange = this._sim_graph.train_to_sim_nodes.get(node.TrainID);
    return (nodeID - idRange.first) * (nodeWidth + nodeSpacingHorizontal);
  }

  // upper left Y
  getY(nodeID) {
    const node = this._sim_graph.nodes.get(nodeID);
    return node.TrainID * (nodeHeight + nodeSpacingVertical);
  }

  // upper left coords
  getCoords(nodeID) {
    return [this.getX(nodeID), this.getY(nodeID)];
  }

  getXCenter(nodeID) {
    return this.getX(nodeID) + (nodeHeight / 2);
  }

  getYCenter(nodeID) {
    return this.getY(nodeID) + (nodeHeight / 2);
  }

  getCoordsCenter(nodeID) {
    return [this.getXCenter(nodeID), this.getYCenter(nodeID)];
  }

  getTrainEdgeSourceCoords(sourceID) {
    let sourceCoords = this.getCoords(sourceID);
    sourceCoords[0] += nodeWidth;
    sourceCoords[1] += nodeHeight / 2;
    return sourceCoords;
  }

  getTrainEdgeTargetCoords(targetID) {
    let targetCoords = this.getCoords(targetID);
    targetCoords[1] += nodeHeight / 2;
    return targetCoords;
  }

  getOrderEdgeCoords(sourceID, targetID) {
    const sourceCoords = this.getCoords(sourceID);
    const targetCoords = this.getCoords(targetID);

    let [X, Y] = targetCoords;

    // source is right of target
    if (sourceCoords[0] > targetCoords[0]) {
      X += nodeWidth;
    }

    // source is lower than target
    if (sourceCoords[1] > targetCoords[1]) {
      Y += nodeHeight;
    }

    return [X, Y];
  }

  createNodes() {
    let nodes = this._d3_graph
      .selectAll('node')
      .data(iterate(this._sim_graph.nodes));

    // this creates the node rectangles
    nodes.enter().append('rect')
      .attr('x', d => this.getX(d.ID))
      .attr('y', d => this.getY(d.ID))
      .attr('width', nodeWidth)
      .attr('height', nodeHeight)
      .style("fill", 'white')
      .on('mouseover', e => this._tooltip.mouseover(e))
      .on('mouseout', e => this._tooltip.mouseout(e))
      .on('mousemove', this._tooltip.mousemove.bind(this._tooltip))
      .on('click', this._tooltip.click.bind(this._tooltip));

    nodes.enter().append('rect')
      .attr('x', d => this.getX(d.ID) + outerWidthPadding)
      .attr('y', d => this.getY(d.ID) + outerHeightPadding)
      .attr('width', innerNodeWidth)
      .attr('height', innerNodeHeight)
      .style('fill', 'lightgrey')
      .on('click', e => this.showDist(e.target.__data__.EntryDPD));

    nodes.enter().append('rect')
      .attr('x', d => this.getX(d.ID) + outerWidthPadding)
      .attr('y', d => this.getY(d.ID) + outerHeightPadding + innerNodeHeight + innerHeightPadding)
      .attr('width', innerNodeWidth)
      .attr('height', innerNodeHeight)
      .style('fill', 'lightgrey')
      .on('click', e => this.showDist(e.target.__data__.EotdDPD));

    nodes.enter().append('rect')
      .attr('x', d => this.getX(d.ID) + outerWidthPadding)
      .attr('y', d => this.getY(d.ID) + outerHeightPadding + (2 * (innerNodeHeight + innerHeightPadding)))
      .attr('width', innerNodeWidth)
      .attr('height', innerNodeHeight)
      .style('fill', 'lightgrey')
      .on('click', e => this.showDist(e.target.__data__.ExitDPD));

    // this creates the node text labels
    nodes.enter().append('text')
      .attr('x', d => this.getXCenter(d.ID))
      .attr('y', d => this.getYCenter(d.ID))
      .attr('text-anchor', 'middle')
      .attr('dominant-baseline', 'middle')
      .attr('pointer-events', 'none')
      .text(d => d.ID);
  }

  createEdges() {
    for (const node of iterate(this._sim_graph.nodes)) {
      if (node.hasSucc()) {
        this._links.push({
          source: node.ID,
          target: node.trainSucc,
          type: 'trainEdge'
        });
      }

      for (const out of iterate(node.OrderOut())) {
        this._links.push({
          source: node.ID,
          target: out,
          type: 'orderEdge'
        });
      }
    }

    console.log('inks', this._links);

    let linkGen = d3.linkHorizontal()
      .source(d => d.type === 'trainEdge' ?
        this.getTrainEdgeSourceCoords(d.source) : this.getOrderEdgeCoords(d.target, d.source))
      .target(d => d.type === 'trainEdge' ?
        this.getTrainEdgeTargetCoords(d.target) : this.getOrderEdgeCoords(d.source, d.target))

    const color = type => {
      if (type === 'trainEdge') {
        return 'black';
      }
      if (type === 'orderEdge') {
        return 'red';
      }
    }

    var types = ['trainEdge', 'orderEdge'];
    this._svg.append("defs").selectAll("marker")
      .data(types)
      .join("marker")
      .attr("id", d => `${d}Arrow`)
      .attr("viewBox", "0 -5 10 10")
      .attr("refX", 0)
      .attr("refY", 0)
      .attr("markerWidth", 6)
      .attr("markerHeight", 6)
      .attr("orient", "auto")
      .append("path")
      .attr("fill", color)
      .attr("d", "M0,-5L10,0L0,5");

    this._d3_graph
      .selectAll('path')
      .data(this._links)
      .join('path')
      .attr('stroke', d => color(d.type))
      .attr('fill', 'none')
      .attr("marker-end", d => `url(${new URL(`#${d.type}Arrow`, location)})`)
      .attr('d', linkGen);
  }

  showDist(dist) {
    this._dist_svg.selectAll("*").remove();

    let minTime = Number.MAX_VALUE;
    let maxTime = 0;

    let minSpeed = Number.MAX_VALUE;
    let maxSpeed = 0;

    for (const [time, speed,] of iterateDist(dist)) {
      minTime = Math.min(minTime, time);
      maxTime = Math.max(maxTime, time);

      minSpeed = Math.min(minSpeed, speed);
      maxSpeed = Math.max(maxSpeed, speed);
    }

    const color = d3.scaleSequential(d3.interpolateWarm)

    const xScaleStep = 6; // We want every 'time-bucket' to be 6 seconds in size
    const yScaleStep = 1; // We want every 'speed-bucket' to be 1 second in size

    const xRangeFrom = 0; // SVG coordinate
    const xRangeTo = this._width / 2; // SVG coordinate

    const yRangeFrom = 0; // SVG coordinate
    const yRangeTo = this._height; // SVG coordinate

    const fontSize = 30;

    const x = d3.scaleBand()
      .domain(d3.range(minTime, maxTime + xScaleStep, xScaleStep))
      .range([xRangeFrom, xRangeTo])
      .padding(0.1);

    const y = d3.scaleBand()
      .domain(d3.range(minSpeed, maxSpeed + yScaleStep, yScaleStep))
      .range([yRangeFrom, yRangeTo])
      .padding(0.1);

    const boxWidth = x.bandwidth();
    const boxHeight = y.bandwidth();

    const timeFormat = t => d3.timeFormat('%X')(t * 1000);

    const xAxis = d3.axisTop(x)
      .ticks(10)
      .tickFormat(timeFormat);

    const yAxis = d3.axisLeft(y).ticks(10);

    const x_g = this._dist_svg.append("g")
      .call(xAxis);

    const y_g = this._dist_svg.append("g")
      .call(yAxis);

    const g_g = this._dist_svg.selectAll('.groups')
      .data(iterateDist(dist))
      .enter()
      .append('g');

    const filterEmpty = ([, , prob]) => prob !== 0;

    g_g.append('rect')
      .filter(filterEmpty)
      .attr('x', ([time, ,]) => x(time))
      .attr('y', ([, speed,]) => y(speed))
      .attr('height', boxHeight)
      .attr('width', boxWidth)
      .attr('fill', ([, , prob]) => color(prob));

    g_g.append('text')
      .filter(filterEmpty)
      .attr('x', ([time, speed, prob]) => x(time) + boxWidth / 2)
      .attr('y', ([time, speed, prob]) => y(speed) + boxHeight / 2)
      .attr('font-size', fontSize)
      .attr('text-anchor', 'middle')
      .attr('dominant-baseline', 'middle')
      .attr('pointer-events', 'none')
      .text(([time, ,]) => timeFormat(time));

    g_g.append('text')
      .filter(filterEmpty)
      .attr('x', ([time, speed, prob]) => x(time) + boxWidth / 2)
      .attr('y', ([time, speed, prob]) => y(speed) + fontSize + boxHeight / 2)
      .attr('font-size', fontSize)
      .attr('text-anchor', 'middle')
      .attr('dominant-baseline', 'middle')
      .attr('pointer-events', 'none')
      .text(([, speed,]) => "Speed: " + speed + " km/h");

    g_g.append('text')
      .filter(filterEmpty)
      .attr('x', ([time, speed, prob]) => x(time) + boxWidth / 2)
      .attr('y', ([time, speed, prob]) => y(speed) + fontSize * 2 + boxHeight / 2)
      .attr('font-size', fontSize)
      .attr('text-anchor', 'middle')
      .attr('dominant-baseline', 'middle')
      .attr('pointer-events', 'none')
      .text(([, , prob]) => "Prob: " + prob.toFixed(2) * 100 + "%");

    this._dist_svg.call(d3.zoom().scaleExtent([0, 1000]).on('zoom', ({transform}) => {
      x_g.attr('transform', transform);
      y_g.attr('transform', transform);
      g_g.attr('transform', transform);
    }));
  }

  createGraph(infrastructure, timetable) {
    this._infrastructure = infrastructure;
    this._timetable = timetable;
    this._sim_graph = new Module.SimGraph(infrastructure, timetable);
    this.createNodes();
    this.createEdges();
  }

  simulate() {
    this._sim_graph.simulate(this._infrastructure, this._timetable, disruptionDists,
      disruptionMap.get('1'), disruptionMap.get('2'));
    this.createNodes();
    this.createEdges();
  }

  resize(width, height) {
    this._width = width;
    this._height = height;
    this.createNodes();
    this.createEdges();
  }

  _svg = undefined;
  _d3_graph = undefined;

  _infrastructure = undefined;
  _timetable = undefined;
  _sim_graph = undefined;

  _links = [];
  _tooltip = undefined;

  _width = undefined;
  _height = undefined;
}

export class SimulationComponent {
  constructor(container) {
    this.container = container;
    this.rootElement = container.element;

    getFileContents("./components/simulation/simulation_component.html")
      .then((html) => {
        this.rootElement.innerHTML = html;
      }).then(() => {
      this._d3Graph = new d3Graph(this.rootElement, container.width, container.height);

      // resize on golden layout component resize event
      container.on('resize', () => {
        this._d3Graph.resizeSVG(container.width, container.height);
      });
    });
  }

  changeInfrastructure(newInfrastructure) {
    const currentTimetable = window.timetableManager.get();
    if (currentTimetable) {
      this._d3Graph.createGraph(newInfrastructure, currentTimetable);
    }
  }

  changeTimetable(newTimetable) {
    const currentInfrastructure = window.infrastructureManager.get();
    if (currentInfrastructure) {
      this._d3Graph.createGraph(currentInfrastructure, newTimetable);
    }
  }

  simulate() {
    this._d3Graph.simulate();
  }

  _d3Graph = undefined;
}
