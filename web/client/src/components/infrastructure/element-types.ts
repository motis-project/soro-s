export enum ElementType {
  Bumper = 'bumper',
  Border = 'border',
  TrackEnd = 'track_end',
  Switch = 'simple_switch',
  ApproachSignal = 'as',
  MainSignal = 'ms',
  ProtectionSignal = 'ps',
  EndOfTrainDetector = 'eotd',
  SpeedLimit = 'spl',
  Tunnel = 'tunnel',
  Halt = 'hlt',
  RTCP = 'rtcp',
  RTCP_U = 'rtcp_u',
  KmJump = 'km_jump',
  LineSwitch = 'line_switch',
  Slope = 'slope',
  Cross = 'cross',
  CTC = 'ctc',
  Meta = 'meta',
  PicturePoint = 'pp'
}

export const ElementTypes: ElementType[] = Object.values(ElementType);

export function valueToType(value: string): ElementType {
  return ElementType[value as keyof typeof ElementType];
}

export const ElementTypeLabel = {
  [ElementType.Bumper]: 'Bumper',
  [ElementType.Border]: 'Border',
  [ElementType.TrackEnd]: 'Track End',
  [ElementType.Switch]: 'Switch',
  [ElementType.ApproachSignal]: 'Approach Signal',
  [ElementType.MainSignal]: 'Main Signal',
  [ElementType.ProtectionSignal]: 'Protection Signal',
  [ElementType.EndOfTrainDetector]: 'End of Train Detector',
  [ElementType.SpeedLimit]: 'Speed Limit',
  [ElementType.Tunnel]: 'Tunnel',
  [ElementType.Halt]: 'Halt',
  [ElementType.RTCP]: 'RTCP',
  [ElementType.RTCP_U]: 'RTCP_U',
  [ElementType.KmJump]: 'KM Jump',
  [ElementType.LineSwitch]: 'Line Switch',
  [ElementType.Slope]: 'Slope',
  [ElementType.Cross]: 'Cross',
  [ElementType.CTC]: 'CTC',
  [ElementType.Meta]: 'Meta',
  [ElementType.PicturePoint]: 'Picture Pont'
};
