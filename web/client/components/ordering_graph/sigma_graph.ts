import Graph from "graphology";
import Sigma from "sigma";
import data from "./data2.json";

export class SigmaGraphCreator {
    //the ordering graph html
    rootelement: HTMLElement;
    //the div element, where the graph will be
    sigmaContainer:HTMLElement;
    //the canvas elements which make up the graph
    renderer: Sigma;

    constructor(rootelement: HTMLElement) {
        this.rootelement = rootelement;
        this.sigmaContainer = rootelement.querySelector('#sigma-container') as HTMLElement;
    };

    public createSigmaGraph() {
        const graph = new Graph();
        graph.import(data);
        var trainIdDummy;
        var nodeXValue = 0;

        //nodes
        graph.forEachNode((node, i) => {
            let currrentTrainId = graph.getNodeAttribute(node, "train_id")
            if (trainIdDummy === currrentTrainId) {
                nodeXValue++
            }
            else {
                trainIdDummy = currrentTrainId
                nodeXValue = 0
            }
            //coordinates of each node
            graph.setNodeAttribute(node, "x", nodeXValue / 2);
            graph.setNodeAttribute(node, "y", currrentTrainId / 2);

            //style elements
            let labelName = "Train:" + currrentTrainId + " Route:" + graph.getNodeAttribute(node, "route_id")
            graph.mergeNodeAttributes(node, { "label": labelName })
            graph.setNodeAttribute(node, "color", "#000000")
        });


        //edges
        graph.forEachEdge((edge) => {
            //style elements
            graph.setEdgeAttribute(edge, "type", "arrow")
        });
        this.renderer = new Sigma(graph, this.sigmaContainer, { allowInvalidContainer: true });
    }

    public resizeSigmaGraph() {
        if (this.renderer !== undefined) {
            this.renderer.refresh();
        }
        else {
            console.log("graph doesn't exist!");
        }
    }

    //used to close the canvas to not run into errors when opening to many windows
    public destroySigmaGraph() {
        if (this.renderer !== undefined) {
            this.renderer.clear();
            this.renderer.kill();
            this.rootelement.removeChild(this.sigmaContainer);
        }
    }
}
