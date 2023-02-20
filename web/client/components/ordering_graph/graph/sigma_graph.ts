import Graph from "graphology";
import Sigma from "sigma";
import { getGeneratedGraph } from "../api/api_graph";

export class SigmaGraphCreator {
    rootelement: HTMLElement;
    //the div element, where the graph will be
    sigmaContainer: HTMLElement;
    //the canvas elements which make up the graph
    renderer: Sigma;
    url: string;
    serverRequest: Promise<any>;

    constructor(rootelement: HTMLElement) {
        this.rootelement = rootelement;
        this.sigmaContainer = rootelement.querySelector('#sigma-container') as HTMLElement;
        this.url = window.origin + '/api/ordering_graph/';
        this.serverRequest = getGeneratedGraph({ url: this.url });
    };

    public createSigmaGraph() {
        this.serverRequest
            .then(json => {
                const graph = new Graph();
                graph.import(json);
                this.setGraphAttributes(graph);
                this.renderer = new Sigma(graph, this.sigmaContainer, { allowInvalidContainer: true });
            })

    }

    private setGraphAttributes(graph) {
        var trainIdDummy: number;
        var nodeXValue = 0;

        graph.forEachNode((node, i) => {
            const currrentTrainId = graph.getNodeAttribute(node, "train_id");
            const labelName = "Train:" + currrentTrainId + " Route:" + graph.getNodeAttribute(node, "route_id");
            if (trainIdDummy === currrentTrainId) {
                nodeXValue++
            }
            else {
                trainIdDummy = currrentTrainId
                nodeXValue = 0
            }
            graph.setNodeAttribute(node, "x", nodeXValue / 2)
                .setNodeAttribute(node, "y", currrentTrainId / 2)
                .setNodeAttribute(node, "label", labelName)
                .setNodeAttribute(node, "color", "#0000FF");
        });

        graph.forEachEdge((edge) => {
            graph.setEdgeAttribute(edge, "type", "arrow")
        });
    }

    public resizeSigmaGraph() {
        if (this.renderer !== undefined) {
            this.renderer.refresh();
        }
        else {
            console.log("graph doesn't exist!");
        }
    }

    public destroySigmaGraph() {
        if (this.renderer !== undefined) {
            this.renderer.clear();
            this.renderer.kill();
            this.rootelement.removeChild(this.sigmaContainer);
        }
    }
}
