import { getFileContents } from "../../utl/getFileContents.js";
import * as d3 from "../../deps/d3/d3.js";

const nodeRadius = 40;


class d3Graph {
    constructor(rootElement, width, height) {
        this._width = width;
        this._height = height;

        var selected = d3.select(rootElement.querySelector('#graph'));

        this._svg = selected
            .append("svg")
            .attr("width", "100%")
            .attr("height", "100%")
            .attr("viewBox", [0, 0, 1000, 1000]);

        this._d3_graph = this._svg
            .append("g");


        this._svg.call(d3.zoom().scaleExtent([0, 1000]).on("zoom", ({ transform }) => {
            this._d3_graph.attr("transform", transform);
        }));

    }

    resizeSVG(width, height) {
        this._svg.attr("width", width)
            .attr("height", height)
            .attr("viewBox", [0, 0, width, height]);
    }

    getNodeXValue(id) {
        return 200 * id;
    }

    getNodeYValue(id) {
        return 200 * id;
    }

    createGraph() {
        let self = this;
        d3.json("../../components/ordering_graph/test2.json").then(function (data) {
            for (var i = 0; i < data.length; i++) {
                for (var j = 0; j < data[i].trains.length; j++) {
                    d3.select("g")
                        .append("circle")
                        .attr("cx", self.getNodeXValue(i))
                        .attr("cy", self.getNodeYValue(j))
                        .attr("r", nodeRadius)
                        .attr("id", "ID" + data[i].trains[j].id)
                        .style("fill", "green");
                }

            }
            for (var i = 0; i < data.length; i++) {
                for (var j = 0; j < data[i].trains.length; j++) {
                    for (var k = 0; k < data[i].trains[j].out.length; k++) {
                        var x = d3.select("#ID" + data[i].trains[j].out[k])
                            .attr("cx");
                        var y = d3.select("#ID" + data[i].trains[j].out[k])
                            .attr("cy");

                        d3.select("g")
                            .append("line")
                            .style("stroke", "lightgreen")
                            .style("stroke-width", 10)
                            .attr("x1", self.getNodeXValue(i))
                            .attr("y1", self.getNodeYValue(j))
                            .attr("x2", x)
                            .attr("y2", y);
                    }

                }

            }

        });



    }

    createNodes() {

    }
}

export class OrderingGraphComponent {
    constructor(container) {
        this.container = container;
        this.rootElement = container.element;

        getFileContents("./components/ordering_graph/ordering_graph_component.html")
            .then((html) => {
                this.rootElement.innerHTML = html;
            }).then(() => {
                this._d3Graph = new d3Graph(this.rootElement, container.width, container.height);
                this._d3Graph.createGraph();

                container.on('resize', () => {
                    this._d3Graph.resizeSVG(container.width, container.height);
                });
            });
    }
}