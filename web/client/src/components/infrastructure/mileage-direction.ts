export enum MileageDirection {
  Falling = 'falling',
  Rising = 'rising',
  Undirected = 'undirected'
}

export const MileageDirections: MileageDirection[] =
  Object.values(MileageDirection);

export const MileageDirectionLabel = {
  [MileageDirection.Falling]: 'Falling',
  [MileageDirection.Rising]: 'Rising',
  [MileageDirection.Undirected]: 'Undirected'
};
