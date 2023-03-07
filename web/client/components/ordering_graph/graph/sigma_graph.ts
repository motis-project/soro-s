import { DirectedGraph } from "graphology";
import Sigma from "sigma";
import { sendRequest, sendPostData } from "../api/api_graph";
import { rendererSettings, edgeSettings, nodeColor, edgeColor, edgeOnHoverColor } from "./settings";
import { subgraph } from "graphology-operators";




export class SigmaGraphCreator {
    rootElement: HTMLElement;
    sigmaContainer: HTMLElement;
    renderer: Sigma<DirectedGraph>;

    constructor(rootElement: HTMLElement) {
        this.rootElement = rootElement;
        this.sigmaContainer = rootElement.querySelector('#sigma-container') as HTMLElement;
        this.getInitGraph();
    };



    //---- graph methods ----

    private createSigmaGraph(json) {
        const graph = new DirectedGraph();
        this.unsafeImport(json, graph);
        const train = 3;
        const smallGraph = subgraph(graph, function(key, attr) {
            return attr.t === 3;
        });
        this.setGraphAttributes(smallGraph);
        this.renderer = new Sigma(smallGraph, this.sigmaContainer, rendererSettings);
        this.setEventHandler(smallGraph);
    }



    private setGraphAttributes(graph: DirectedGraph) {
        this.setNodeAttributes(graph);
        this.setEdgeAttributes(graph);
    }

    private setNodeAttributes(graph: DirectedGraph) {
        var trainIdDummy: number;
        var nodeXValue = 0;

        graph.forEachNode((node, i) => {
            const currentTrainId = graph.getNodeAttribute(node, "t");
            const labelName = "Train:" + currentTrainId + " Route:" + graph.getNodeAttribute(node, "r");
            if (trainIdDummy === currentTrainId) {
                nodeXValue++
            }
            else {
                trainIdDummy = currentTrainId
                nodeXValue = 0
            }
            graph.setNodeAttribute(node, "x", nodeXValue / 60)
                .setNodeAttribute(node, "y", currentTrainId / 30)
                .setNodeAttribute(node, "label", labelName)
                .setNodeAttribute(node, "color", nodeColor)
                .setNodeAttribute(node, "size", 2)
                .setNodeAttribute(node, "type", "base");
        });
    }

    private setEdgeAttributes(graph: DirectedGraph) {
        graph.forEachEdge((edge) => {
            this.edgeAttributes(graph, edge);
        });
    }

    private isRouteEdge(graph: DirectedGraph, edge): boolean {
        const source = graph.source(edge);
        const target = graph.target(edge);
        const sourceTrainId = graph.getNodeAttribute(source, "t");
        const targetTrainId = graph.getNodeAttribute(target, "t");
        return sourceTrainId !== targetTrainId;
    }

    private edgeAttributes(graph: DirectedGraph, edge) {
        graph.mergeEdgeAttributes(edge, edgeSettings);
    }


    //---- renderer methods ----

    private setEventHandler(graph: DirectedGraph) {
        this.invertEdge(graph);
        this.highlightHoveredEdge(graph);
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
            this.rootElement.removeChild(this.sigmaContainer);
        }
    }


    private highlightHoveredEdge(graph: DirectedGraph) {
        this.renderer.on("enterEdge", ({ edge }) => {
            graph.setEdgeAttribute(edge, "color", edgeOnHoverColor);
        });
        this.renderer.on("leaveEdge", ({ edge }) => {
            graph.setEdgeAttribute(edge, "color", edgeColor);
        })
    }

    private invertEdge(graph: DirectedGraph) {
        this.renderer.on("clickEdge", (event) => {
            if (this.isRouteEdge(graph, event.edge)) {
                const edgeInformation: Array<string> = [graph.source(event.edge), graph.target(event.edge)];
                this.getUpdatedGraph(edgeInformation, graph);
            }
        });
        this.renderer.refresh();
    }

    //---- data handling ----

    /**
     * requests the random generated graph from the server
     */
    private getInitGraph() {
        sendRequest({ url: '/api/ordering_graph/' })
            .then(response => response.json())
            .then(json =>
                this.createSigmaGraph(json));
    }

    /**
     * requests the updated graph after reversing an edge
     * @param edgeInformation source and target of the edge which should be reversed
     * @param graph 
     */
    private getUpdatedGraph(edgeInformation: Array<string>, graph: DirectedGraph) {
        sendPostData({ url: '/api/ordering_graph/invert', data: graph, values: edgeInformation })
            .then(response => response.json())
            .then(json =>
                this.createSigmaGraph(json));
    }


    /**
     * Unsafe Version of the Graphology import Function, removed checks to make it faster
     * nodes : k = key of the node, a = attributes of each node where t = train_id and r = route_id
     * edges : s = source of the edge, t = target of the edge, a = attributes of the edge, takes the loop variable i as a key for faster lookup
     * @param data json format:
     * {
        "attributes": {},
        "nodes": [
            {
                "k": string,
                "a": {
                    "r": number,
                    "t": number
                }
            }
        ],
        "edges": [
            {
                "s": string,
                "t": string
            }
        ]
       }
     * 
     * @param graph empty graph to fill with data
     * @returns the graph with the imported data
     */
    private unsafeImport(data, graph: DirectedGraph) {
        // Importing a serialized graph    
        if (data.attributes) {
            graph.replaceAttributes(data.attributes);
        }

        let i, l, list, node, edge;

        if (data.nodes) {
            list = data.nodes;

            for (i = 0, l = list.length; i < l; i++) {
                node = list[i];

                // Adding the node
                const { k, a } = node;
                graph.addNode(k, a);
            }
        }

        if (data.edges) {
            let undirectedByDefault = false;
            list = data.edges;

            for (i = 0, l = list.length; i < l; i++) {
                edge = list[i];
                // Adding the edge
                const {
                    s,
                    t,
                    a,
                    undirected = undirectedByDefault
                } = edge;

                graph.addDirectedEdgeWithKey(i, s, t, a);
            }
        }
        return graph;
    }


}
