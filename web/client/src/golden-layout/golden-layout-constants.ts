export enum ComponentTechnicalName {
    INFRASTRUCTURE,
    SIMULATION,
    TIMETABLE,
    ORDERING_GRAPH,
}

export const GLComponentNames = {
    [ComponentTechnicalName.INFRASTRUCTURE]: 'InfrastructureComponent',
    [ComponentTechnicalName.SIMULATION]: 'SimulationComponent',
    [ComponentTechnicalName.TIMETABLE]: 'TimetableComponent',
    [ComponentTechnicalName.ORDERING_GRAPH]: 'OrderingGraphComponent',
};

export const GLComponentTitles = {
    [ComponentTechnicalName.INFRASTRUCTURE]: 'Infrastructure',
    [ComponentTechnicalName.SIMULATION]: 'Simulation',
    [ComponentTechnicalName.TIMETABLE]: 'Timetable',
    [ComponentTechnicalName.ORDERING_GRAPH]: 'Ordering Graph',
};