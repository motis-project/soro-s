export const ElementType = {
    BUMPER: 'bumper',
    BORDER: 'border',
    TRACK_END: 'track_end',
    SWITCH: 'simple_switch',
    APPROACH_SIGNAL: 'as',
    MAIN_SIGNAL: 'ms',
    PROTECTION_SIGNAL: 'ps',
    END_OF_TRAIN_DETECTOR: 'eotd',
    SPEED_LIMIT: 'spl',
    TUNNEL: 'tunnel',
    HALT: 'hlt',
    RTCP: 'rtcp',
    KM_JUMP: 'km_jump',
    LINE_SWITCH: 'line_switch',
    SLOPE: 'slope',
    CROSS: 'cross',
    CTC: 'ctc',
    STATION: 'station',
};

export const ElementTypes = Object.values(ElementType);

export const ElementTypeLabels = {
    [ElementType.BUMPER]: 'Bumper',
    [ElementType.BORDER]: 'Border',
    [ElementType.TRACK_END]: 'Track End',
    [ElementType.SWITCH]: 'Switch',
    [ElementType.APPROACH_SIGNAL]: 'Approach Signal',
    [ElementType.MAIN_SIGNAL]: 'Main Signal',
    [ElementType.PROTECTION_SIGNAL]: 'Protection Signal',
    [ElementType.END_OF_TRAIN_DETECTOR]: 'End of Train Detector',
    [ElementType.SPEED_LIMIT]: 'Speed Limit',
    [ElementType.TUNNEL]: 'Tunnel',
    [ElementType.HALT]: 'Halt',
    [ElementType.RTCP]: 'RTCP',
    [ElementType.KM_JUMP]: 'KM Jump',
    [ElementType.LINE_SWITCH]: 'Line Switch',
    [ElementType.SLOPE]: 'Slope',
    [ElementType.CROSS]: 'Cross',
    [ElementType.CTC]: 'CTC',
    [ElementType.STATION]: 'Station'
};
