/**
 * This is a minimal example of sigma. You can use it as a base to write new
 * examples, or reproducible test cases for new issues, for instance.
 */
import Graph from "graphology";
import Sigma from "sigma";

import data from "./data2.json";

export class SigmaGraphCreator {
    rootelement;
    sigmaContainer;
    renderer;
    constructor(rootelement: HTMLElement){
        this.rootelement = rootelement;
        this.sigmaContainer = document.createElement('div');
        this.sigmaContainer.setAttribute("style", "height:100%;");
        this.rootelement.appendChild(this.sigmaContainer);
    };

   /**
    c foo
    */
   public createSigmaGraph() {       
        const graph = new Graph();
        graph.import(data);
        var train_id_dummy;
        var x_value = 0;

        graph.nodes().forEach((node, i) => {
            let currrent_train_id = graph.getNodeAttribute(node, "train_id");
            if (train_id_dummy === undefined || train_id_dummy !== currrent_train_id) {
                train_id_dummy = currrent_train_id;
                x_value = 0;
            }
            else {
                x_value++;
            }
            graph.setNodeAttribute(node, "x", x_value);
            //graph.setNodeAttribute(node, "x", graph.getNodeAttribute(node, "route_id"));
            graph.setNodeAttribute(node, "y", currrent_train_id);
            let labelName = "Train:" + currrent_train_id +" Route:" + graph.getNodeAttribute(node, "route_id");
            graph.mergeNodeAttributes(node, {"label": labelName});
          });
       
        this.renderer = new Sigma(graph, this.sigmaContainer, {allowInvalidContainer : true});
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
        if(this.renderer !== undefined) {
            this.renderer.clear();
            this.renderer.kill();
            this.rootelement.removeChild(this.sigmaContainer);
        }
    }
}