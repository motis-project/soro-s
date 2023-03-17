export enum ComponentTechnicalName {
    INFRASTRUCTURE = 'INFRASTRUCTURE',
    SIMULATION = 'SIMULATION',
    TIMETABLE = 'TIMETABLE',
    ORDERING_GRAPH = 'ORDERING_GRAPH',
}

export const GLComponentNames: { [key in ComponentTechnicalName]: string } = {
    [ComponentTechnicalName.INFRASTRUCTURE]: 'InfrastructureComponent',
    [ComponentTechnicalName.SIMULATION]: 'SimulationComponent',
    [ComponentTechnicalName.TIMETABLE]: 'TimetableComponent',
    [ComponentTechnicalName.ORDERING_GRAPH]: 'OrderingGraphComponent',
};

export const GLComponentTitles: { [key in ComponentTechnicalName]: string } = {
    [ComponentTechnicalName.INFRASTRUCTURE]: 'Infrastructure',
    [ComponentTechnicalName.SIMULATION]: 'Simulation',
    [ComponentTechnicalName.TIMETABLE]: 'Timetable',
    [ComponentTechnicalName.ORDERING_GRAPH]: 'Ordering Graph',
};