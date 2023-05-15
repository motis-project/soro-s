//@ts-nocheck
import { DirectedGraph } from 'graphology';
import Sigma from 'sigma';
import { sendRequest, sendPostData } from '../api/api_graph';
import {
  rendererSettings,
  edgeSettings,
  nodeColor,
  edgeColor,
  edgeOnHoverColor,
  edgeOnReverseColor
} from './settings';
import { subgraph } from 'graphology-operators';
import $ from 'jquery';
import { NodeDisplayData } from 'sigma/types';

interface State {
  hoveredNode?: string;
  hoveredNeighbors?: Set<string>;
  selectedNodes?: Set<string>;
}

class deleteGraph {
  /**
   * requests the updated graph after reversing an edge
   * @param edgeInformation source and target of the edge which should be reversed
   * @param graph
   */

  private getUpdatedGraph(
    edgeInformation: Array<string>,
    graph: DirectedGraph
  ) {
    // Creates the Data Graph without displaying it (neighboorhood + 1)
    this.createSubGraph(false);
    const data: string = this.exportGraphAsJson(this.dataGraph);
    sendPostData({
      url: '/api/ordering_graph/invert',
      data: data,
      values: edgeInformation
    })
      .then((response) => response.json())
      .then((json) => {
        this.processGraphDelta(json);
      });
  }

  /**
   * This method changes the direction of an edge aswell as changing the color.
   * @param graph
   * @param edge
   */
  private changeEdgeDirection(graph: DirectedGraph, edge) {
    const oldEdgeSource = graph.source(edge);
    const oldEdgeTarget = graph.target(edge);

    graph.dropEdge(oldEdgeSource, oldEdgeTarget);
    const newEdge = graph.addDirectedEdgeWithKey(
      edge,
      oldEdgeTarget,
      oldEdgeSource
    );
    this.edgeAttributes(graph, newEdge);
    graph.setEdgeAttribute(newEdge, 'color', edgeOnReverseColor);

    this.recentlyChangedEdges.add(newEdge);
  }

  /**
   * This method replaces an edge with another one aswell as changing the color.
   * @param graph
   * @param oldSource
   * @param oldTarget
   * @param newSource
   * @param newTarget
   */
  private replaceEdge(
    graph: DirectedGraph,
    oldSource,
    oldTarget,
    newSource,
    newTarget
  ) {
    const edgeID = graph.edge(oldSource, oldTarget);
    graph.dropDirectedEdge(oldSource, oldTarget);

    const newEdge = graph.addDirectedEdgeWithKey(edgeID, newSource, newTarget);
    this.edgeAttributes(graph, newEdge);
    graph.setEdgeAttribute(newEdge, 'color', edgeOnReverseColor);
    this.recentlyChangedEdges.add(newEdge);
  }

  /**
   * This method processes the changes and adjusts the edges in the graph.
   * @param data The delta recieved from the server when inverting an edge
   */
  private processGraphDelta(data) {
    if (data.at) {
      this.subGraph.replaceAttributes(data.at);
    }
    let i, l;

    if (data.c) {
      for (i = 0, l = data.c.length; i < l; i++) {
        const source = data.c[i][0];
        const target = data.c[i][1];
        const edge = this.subGraph.edge(source, target);
        this.changeEdgeDirection(this.subGraph, edge);
        this.changeEdgeDirection(this.graph, edge);
      }
    }
    if (data.d && data.a) {
      for (i = 0, l = data.d.length; i < l; i++) {
        const oldSource = data.d[i][0];
        const oldTarget = data.d[i][1];
        const newSource = data.a[i][0];
        const newTarget = data.a[i][1];
        if (this.subGraph.hasEdge(oldSource, oldTarget)) {
          this.replaceEdge(
            this.subGraph,
            oldSource,
            oldTarget,
            newSource,
            newTarget
          );
        }
        this.replaceEdge(
          this.graph,
          oldSource,
          oldTarget,
          newSource,
          newTarget
        );
      }
    }
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
    });

    return {
      a: {},
      n: nodes,
      e: edges
    };
  }

  /**
   * Requests a randomly generated graph from the server.
   */
  private async getInitGraph(graph: DirectedGraph) {
    sendRequest({ url: '/api/ordering_graph/' })
      .then((response) => response.json())
      .then((json) => {
        this.unsafeImport(json, graph);
      });
  }
}
