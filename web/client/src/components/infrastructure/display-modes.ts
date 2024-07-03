export enum DisplayMode {
  Node = 'node',
  Element = 'element',
  Map = 'map'
}

export const DisplayModes: DisplayMode[] = Object.values(DisplayMode);

export const DisplayModeValues: string[] = Object.keys(DisplayMode);

export const DisplayModeLabel = {
  [DisplayMode.Node]: 'Node',
  [DisplayMode.Element]: 'Element',
  [DisplayMode.Map]: 'Map'
};
