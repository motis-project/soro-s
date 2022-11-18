import { getFileContents } from "../../utl/getFileContents.js";

export class OrderingGraphComponent {
    constructor(container) {
        this.container = container;
        this.rootElement = container.element;

        getFileContents("./components/ordering_graph/ordering_graph_component.html")
            .then((html) => {
                this.rootElement.innerHTML = html;
            });
    }
}
