//@ts-nocheck
import { DirectedGraph } from "graphology";
import Sigma from "sigma";
import { sendRequest, sendPostData } from "../api/api_graph";
import { rendererSettings, edgeSettings, nodeColor, edgeColor, edgeOnHoverColor } from "./settings";
import { subgraph } from "graphology-operators";
import $ from "jquery";
import { NodeDisplayData } from "sigma/types";

interface State {
    hoveredNode?: string;
    hoveredNeighbors?: Set<string>;
    selectedNodes?: Set<string>;
}


export class SigmaGraphCreator {
    //helpers for filtering
    state: State;
    maxTrainLength: number;
    subGraphRouteIds: Set<number>;
    nodesForGivenTrainId: Map<number, Array<string>>;
    nodesForGivenRouteId: Map<number, Array<string>>;
    enableNodeHover: Boolean;

    //graph
    sigmaContainer: HTMLElement;
    graph: DirectedGraph;
    subGraph: DirectedGraph;
    renderer: Sigma<DirectedGraph>;

    //input variables
    inputTrainID;
    inputRouteID;
    inputNeighbor;
    inputScaling;

    constructor(rootElement: HTMLElement) {
        this.sigmaContainer = rootElement.querySelector('#sigma-container') as HTMLElement;
        this.maxTrainLength = 0;
        this.enableNodeHover = false;
        this.state = {};
        this.nodesForGivenTrainId = new Map<number, Array<string>>();
        this.subGraph = new DirectedGraph();
        this.graph = new DirectedGraph();
        // This function populates the data for this.graph, but does not yet display it.
        this.getInitGraph(this.graph);

        var self = this;
        // These renderer events always operate on the pre-filtered graph.
        $("#node-enable").on("change", function () {
            self.enableNodeHover = $("#node-enable").is(":checked");
        });
        $("#route-input").on("change", function () {
            self.processRouteID();
        });
        $("#Scaling-input").on("change", function () {
            self.destroySigmaGraph();
            self.subGraphRouteIds = new Set<number>();
            self.inputScaling = parseInt($("#Scaling-input").val());
            self.createSubGraph();
            self.processRouteID();
        });
        // These graph events operate on the entire data set, rather than just a pre-filtered subset.
        $("#search").on("input", "#search-input, #neighbor-input", function () {
            self.processInput();
            if (self.validateInputTrainId()) {
                self.createSubGraph();
                self.createRouteSuggestions();
            }
        });
    }

    /**
     * // This function begins the process of highlighting nodes when a Route ID is entered into the UI.
     */
    private processRouteID() {
        this.inputRouteID = $("#route-input").val();
        // If the selected value is not the default, highlight the corresponding nodes.
        if (this.inputRouteID != "null") {
            this.highlightRoute(this.subGraph, parseInt(this.inputRouteID));
        }
        // If there is no selected value, reset the renderer to display all nodes.
        else {
            this.state.selectedNodes = undefined;
            this.renderer.refresh();
        }
    }

    /**
     * This function handles user interactions with the UI by setting important class variables, 
     * and enables/disables the Route ID window or displays an error message on the Train ID window if the input is incorrect.
     */
    private processInput() {
        // Since the available Route IDs depend on Train ID and Neighborhood, 
        // we need to clear the Route ID field every time any  of the other fields are changed.
        $("#route-input").empty();
        $("#route-input").append('<option value="null">--- Select RouteID ---</option>')

        // If the user enters a non-empty Train ID, we need to update the class attributes.
        if ($("#search-input").val() !== "") {
            this.destroySigmaGraph();
            this.state.selectedNodes = undefined;
            this.subGraphRouteIds = new Set<number>();
            this.maxTrainLength = 0;
            this.inputTrainID = parseInt($("#search-input").val());
            this.inputNeighbor = $("#neighbor-input").val();
            this.inputScaling = parseInt($("#Scaling-input").val());
            $("#output").html(this.inputNeighbor);

            // If the given Train ID is valid, we display the Route ID window.
            if (this.validateInputTrainId()) {
                $("#route-row").show();
                $("#search-input").removeClass("has-error");
                $("#warning").hide();
            }
            // Otherwise, we hide the Route ID window and display an error on the Train ID window.
            else {
                $("#route-row").hide();
                $("#search-input").addClass("has-error");
                $("#warning").show();
            }
        }
        // If the Train ID window is empty, we hide RouteID and remove the error Message.
        else {
            $("#route-row").hide();
            $("#search-input").removeClass("has-error");
            $("#warning").hide();
            this.inputTrainID = undefined;
        }
    }

    /**
     * 
     * @returns true if the user input exists in the original Graph data, and false otherwise.
     */
    private validateInputTrainId(): boolean {
        return this.nodesForGivenTrainId.has(this.inputTrainID);
    }

    /**
     * 
     * @returns a set with all relevant Train IDs for the subGraph.
     */
    private getRelevantTrainIds(): Set<number> {
        let result = new Set<number>();
        result.add(this.inputTrainID);
        for (let i = 0; i < this.inputNeighbor; i++) {
            result = new Set([...result, ...this.getNeighborTrainIds(result)]);
        }
        return result;
    }

    /**
     * For each value in Train IDs, we retrieve the array from the hashmap, iterate over each node, and add the Train ID of each neighbor to the result.
     * @param trainIDs
     * @returns a set of trainIds, which are a neighbor of a train for every trainId in trainIds
     */
    private getNeighborTrainIds(trainIDs: Set<number>): Set<number> {
        let result = new Set<number>();
        trainIDs.forEach(trainId => {
            this.nodesForGivenTrainId.get(trainId)!.forEach(node => {
                this.graph.forEachNeighbor(node, neighbor => {
                    if (this.graph.getNodeAttribute(node, "t") != this.graph.getNodeAttribute(neighbor, "t")) {
                        result.add(this.graph.getNodeAttribute(neighbor, "t"));
                    }
                });
            })
        });
        return result;
    }


    /**
     * We iterate over each Train ID, retrieve the nodes from the hashmap, 
     * set the maximum train length, subgraph Route IDs, and the hashmap for the Route IDs for the subgraph.
     * @param relevantTrainIds The set of all Train IDs that need to be displayed.
     * @returns The set of all relevant nodes for the given input of Train ID and neighborhood degree.
     */
    private getRelevantNodes(relevantTrainIds: Set<number>): Set<string> {
        let allNodes = new Set<string>();
        relevantTrainIds.forEach(trainId => {
            const nodesOfCurrentTrainId = this.nodesForGivenTrainId.get(trainId);
            this.maxTrainLength = this.maxTrainLength < nodesOfCurrentTrainId!.length ? nodesOfCurrentTrainId!.length : this.maxTrainLength;

            nodesOfCurrentTrainId!.forEach((node) => {
                const route_id = this.graph.getNodeAttribute(node, "r");
                this.intializeRouteInfo(route_id, node);
                allNodes.add(node);
            });
        });
        return allNodes;
    }

    /**
     * This function adds the specified RouteID to the subGraphRouteIds class attribute and also includes the given node in the nodesForGivenRouteId hashmap.
     * @param route_id 
     * @param node 
     */
    private intializeRouteInfo(route_id, node) {
        this.subGraphRouteIds.add(route_id);
        this.nodesForGivenRouteId.get(route_id) === undefined ? this.nodesForGivenRouteId.set(route_id, [node]) : this.nodesForGivenRouteId.get(route_id)!.push(node);
    }

    /**
     * Creates the Dropdown Menu for the Route IDs.
     */
    private createRouteSuggestions() {
        this.subGraphRouteIds.forEach((route) => {
            $("#route-input").append('<option value="' + route + '">' + route + '</option>');
        });
    }

    /**
     * Creates and displays the subGraph.
     */
    private createSubGraph() {
        let allTrainIds = this.getRelevantTrainIds();
        const numberOfTrains = allTrainIds.size;
        allTrainIds = [...allTrainIds].sort((a, b) => a - b);

        this.nodesForGivenRouteId = new Map<number, Array<string>>();


        this.subGraph = subgraph(this.graph, this.getRelevantNodes(allTrainIds));
        this.subGraphRouteIds = [...this.subGraphRouteIds].sort((a, b) => a - b);

        this.setGraphAttributes(this.subGraph, numberOfTrains, this.inputScaling);
        this.renderer = new Sigma(this.subGraph, this.sigmaContainer, rendererSettings);
        this.bindRendererEvents(this.subGraph);
    }

    private setGraphAttributes(graph: DirectedGraph, numberOfTrains: number, Scaling: number) {
        this.setNodeAttributes(graph, numberOfTrains, Scaling);
        this.setEdgeAttributes(graph);
    }

    /**
     * The x and y values are set to create a grid-like representation.
     * @param graph 
     * @param numberOfTrains 
     */
    private setNodeAttributes(graph: DirectedGraph, numberOfTrains: number, Scaling: number) {
        let lastTrainId = null;
        let ratio = this.maxTrainLength / numberOfTrains;
        let xPosition = 0;
        let yPosition = -1;

        let xSpacing = 500;
        let ySpacing = 500;

        graph.forEachNode((node) => {
            const currentTrainId = graph.getNodeAttribute(node, "t");
            const labelName = "Train:" + currentTrainId + " Route:" + graph.getNodeAttribute(node, "r");


            if (lastTrainId != currentTrainId) {
                xPosition = 0;
                yPosition++;
            }
            else {
                xPosition++;
            }
            lastTrainId = currentTrainId;
            graph.setNodeAttribute(node, "x", xPosition * xSpacing * (Scaling / 10))
                .setNodeAttribute(node, "y", yPosition * ySpacing * ratio)
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

    /**
     * 
     * @returns  true if the source and target nodes have different route IDs, and false otherwise.
     */
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

    private bindRendererEvents(graph: DirectedGraph) {
        this.invertEdge(graph);
        this.highlightHoveredEdge(graph);
        this.highlightHoveredNode(graph);
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
        }
    }

    private setHoveredNode(graph: DirectedGraph, node?: string) {
        if (node && this.enableNodeHover) {
            this.state.hoveredNode = node;
            this.state.hoveredNeighbors = new Set(graph.neighbors(node));
        } else {
            this.state.hoveredNode = undefined;
            this.state.hoveredNeighbors = undefined;
        }
    }

    private rendererReducer(graph: DirectedGraph) {
        this.rendererNodeReducer(graph);
        this.rendererEdgeReducer(graph);
    }

    /**
     * renders the nodes which are not of interest as white and doesnt display the label thus "reducing" rendered nodes
     * as well as resetting it, if needed
     * @param graph 
     */
    private rendererNodeReducer(graph: DirectedGraph, routeId?) {
        this.renderer.setSetting("nodeReducer", (node, attributes) => {
            const nodeData: Partial<NodeDisplayData> = { ...attributes };
            if (this.state.hoveredNeighbors && !this.state.hoveredNeighbors.has(node) && this.state.hoveredNode !== node) {
                nodeData.label = "";
                nodeData.color = "#f6f6f6";
            }
            // every node which is not in selected not or has no neighbor in selected is rendered white
            if (this.state.selectedNodes && !(this.state.selectedNodes.has(node) && this.stateEdgeValidation(graph, node))) {
                nodeData.label = null;
                nodeData.color = "#f6f6f6";
            } else if(this.state.selectedNodes && this.state.selectedNodes.has(node)) {
                nodeData.highlighted = true;
            }
            return nodeData;
        });
    }


    /**
     * checks has a neighbor in this.selectedNodes
     * @param graph 
     * @param node 
     */
    private stateEdgeValidation(graph: DirectedGraph, node: string): boolean {
        let result = false;
        this.state.selectedNodes?.forEach(selectedNode => {
            if (graph.areNeighbors(node, selectedNode)) {
                //if we found a neighbor, we dont need to continue
                result = true;
                return;
            }
        });
        return result;
    }

    /** 
     *  
    */
    private rendererEdgeReducer(graph: DirectedGraph) {
        this.renderer.setSetting("edgeReducer", (edge, data) => {
            const edgeData: Partial<EdgeDisplayData> = { ...data };

            if (this.state.hoveredNode && !graph.hasExtremity(edge, this.state.hoveredNode)) {
                edgeData.hidden = true;
            }

            if (this.state.selectedNodes && (!this.state.selectedNodes.has(graph.source(edge)) || !this.state.selectedNodes.has(graph.target(edge)))) {
                edgeData.hidden = true;
            }

            return edgeData;
        });
    }


    /**
     * Highlights the nodes with the route ID entered by the user.
     */
    private highlightRoute(graph: DirectedGraph, routeId: number) {
        this.state.selectedNodes = new Set(this.nodesForGivenRouteId.get(routeId));
        this.rendererNodeReducer(graph, routeId); 
        this.rendererEdgeReducer(graph);
        this.renderer.refresh();

    }

    private highlightHoveredNode(graph: DirectedGraph) {
        this.renderer.on("enterNode", ({ node }) => {
            this.hoveredNodeActions(graph, node);
        });
        this.renderer.on("leaveNode", () => {
            this.hoveredNodeActions(graph, undefined);
        })
    }

    private hoveredNodeActions(graph: DirectedGraph, node?: string) {
        this.setHoveredNode(graph, node);
        this.rendererReducer(graph);
        this.renderer.refresh();
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
     * Requests a randomly generated graph from the server.
     */
    private async getInitGraph(graph: DirectedGraph) {
        sendRequest({ url: '/api/ordering_graph/' })
            .then(response => response.json())
            .then(json => {
                this.unsafeImport(json, graph);
            });
    }


    /**
     * requests the updated graph after reversing an edge
     * @param edgeInformation source and target of the edge which should be reversed
     * @param graph 
     */
    private getUpdatedGraph(edgeInformation: Array<string>, graph: DirectedGraph) {
        const data = this.exportGraphAsJson(graph);
        sendPostData({ url: '/api/ordering_graph/invert', data: data, values: edgeInformation })
            .then(response => response.json())
            .then(json =>
                this.unsafeImport(json, graph));
    }

    /**
     * returns the same format which the unsafeImport method uses as an input
     * @param graph 
     */
    private exportGraphAsJson(graph: DirectedGraph) {
        const nodes = new Array(graph._nodes.size);

        let i = 0;

        graph.forEachNode((node, attr) => {
            nodes[i++] = [node, attr.r, attr.t];
        });

        const edges = new Array(graph._edges.size);

        i = 0;

        graph.forEachEdge((edge) => {
            edges[i++] = [graph.source(edge), graph.target(edge)];
        })

        return {
            "a": {},
            "n": nodes,
            "e": edges
        };
    }


    /**
     * Unsafe Version of the Graphology import Function, removed checks and personalized json processing to make it faster
     * @param data json format:
     * {
        //general graph attributes
        "a": {},
        // nodes 
        "n": [
            // string: key of the node  first number: route_id of the node  second number: train_id of the node
            [string, number, number]
        ],
        // edges
        "e": [
            // first string: source, second: target
            [string, string]
        ]
       }
     * 
     * @param graph empty graph to fill with data
     * @returns the graph with the imported data
     */
    private unsafeImport(data, graph: DirectedGraph) {
        // Importing a serialized graph    
        if (data.a) {
            graph.replaceAttributes(data.a);
        }

        let i, l;

        if (data.n) {
            for (i = 0, l = data.n.length; i < l; i++) {
                const r = data.n[i][1];
                const t = data.n[i][2];

                graph.addNode(data.n[i][0], { r, t });
                // map for faster filtering later on
                this.nodesForGivenTrainId.get(t) === undefined ? this.nodesForGivenTrainId.set(t, [data.n[i][0]]) : this.nodesForGivenTrainId.get(t)!.push(data.n[i][0]);
            }
        }

        if (data.e) {
            for (i = 0, l = data.e.length; i < l; i++) {
                graph.addDirectedEdgeWithKey(i, data.e[i][0], data.e[i][1]);
            }
        }
        return graph;
    }


}
