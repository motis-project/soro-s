function getComponentsFilter(filter_fun) {
  return window.goldenLayout.getAllContentItems().filter(filter_fun).map(i => {
    return i.component;
  });
}

export function getComponents() {
  return window.goldenLayout.getAllContentItems()
    .filter(c => {
      return c.isComponent && c.componentType !== 'ControlComponent';
    })
    .map(i => {
      return i.component;
    });
}

export function getInfrastructureComponents() {
  return getComponentsFilter(c => {
    return c.isComponent && c.componentType === 'InfrastructureComponent';
  });
}

export function getTimetableComponents() {
  return getComponentsFilter(c => {
    return c.isComponent && c.componentType === 'TimetableComponent';
  });
}

export function getSimulationComponent() {
  return getComponentsFilter(c => {
    return c.isComponent && c.componentType === 'SimulationComponent';
  });
}

export function getOrderingGraphComponent() {
    return getComponentsFilter(c => {
        return c.isComponent && c.componentType === 'OrderingGraphComponent';
    });
}

export function getControlComponent() {
  return getComponentsFilter(c => {
    return c.isComponent && c.componentType === 'ControlComponent';
  })[0];
}

