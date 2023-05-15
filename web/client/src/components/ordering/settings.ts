import { Settings } from 'sigma/settings';
import NodeProgram from 'sigma/rendering/webgl/programs/node';
import NodeFastProgram from 'sigma/rendering/webgl/programs/node.fast';
// import EdgesFastProgram from 'sigma/rendering/webgl/programs/edge.fast';

export const nodeColor = '#000000';
export const edgeColor = '#808080';
export const edgeOnHoverColor = '#2196f3';
export const edgeOnReverseColor = 'red';

export const rendererSettings: Partial<Settings> = {
  allowInvalidContainer: true,
  enableEdgeClickEvents: false,
  renderLabels: true,
  enableEdgeHoverEvents: false,
  defaultEdgeType: 'edges-fast',
  nodeProgramClasses: {
    fast: NodeFastProgram,
    base: NodeProgram
  }
};

export const edgeSettings = {
  type: 'arrow',
  size: 2
};
