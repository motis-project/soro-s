import { Settings } from "sigma/settings";
import NodeProgram from "sigma/rendering/webgl/programs/node"

export const nodeColor = "#000000";
export const edgeColor = "#808080";
export const edgeOnHoverColor = "blue";
export const edgeOnReverseColor = "red";

export const rendererSettings: Partial<Settings> = {
    allowInvalidContainer: true,
    enableEdgeClickEvents: true,
    renderLabels: false,
    enableEdgeHoverEvents: true,
    nodeProgramClasses: {
        base: NodeProgram
    }
}

export const edgeSettings = {
    type: "arrow",
    size: 2,
    color: edgeColor,
}
