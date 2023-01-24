import { getFileContents } from "../../utl/getFileContents.js";
// bundle basicly works as a library now for all typescript files
import "./bundle.js";

export class OrderingGraphComponent {
    constructor(container) {
        this.container = container;
        this.rootElement = container.element;
        
        getFileContents("./components/ordering_graph/ordering_graph_component.html")
            .then((html) => {
                this.rootElement.innerHTML = html;
            }).then(() => {
                var graph = new webpackSigmaGraph.SigmaGraphCreator(this.rootElement);
                graph.createSigmaGraph();

                container.on('resize', () => {
                    graph.resizeSigmaGraph();
                });
                container.on('destroy', () => {
                    graph.destroySigmaGraph();
                });
            });
    }
}
