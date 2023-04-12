import { VueWrapper } from '@vue/test-utils';
import { VExpansionPanels } from 'vuetify/components';

import SidebarMenu from '@/components/sidebar/menu/menu.vue';
import { GoldenLayoutNamespace } from '@/stores/golden-layout-store';
import { ComponentTechnicalName } from '@/golden-layout/golden-layout-constants';
import {
  InfrastructureNamespace,
  InfrastructureState
} from '@/stores/infrastructure-store';
import { TimetableNamespace, TimetableState } from '@/stores/timetable-store';

import { mountWithDefaults } from '@/test/helpers/mount-with-defaults';

describe('sidebar-menu', async () => {
  let soroNavigationMenuContent: VueWrapper<any>;
  const goldenLayoutActions = { addGoldenLayoutTab: vi.fn() };
  const infrastructureState: InfrastructureState = {
    infrastructures: [],
    currentInfrastructure: '',
    currentSearchResults: [],
    highlightedStationRoutes: [],
    highlightedInterlockingRoutes: []
  };
  const timetableState: TimetableState = {
    timetables: [],
    currentTimetable: ''
  };
  const infrastructureActions = { load: vi.fn() };
  const timetableActions = { load: vi.fn() };

  const defaults = {
    store: {
      [GoldenLayoutNamespace]: { actions: goldenLayoutActions },
      [InfrastructureNamespace]: {
        state: infrastructureState,
        actions: infrastructureActions
      },
      [TimetableNamespace]: {
        state: timetableState,
        actions: timetableActions
      }
    }
  };

  beforeEach(async () => {
    goldenLayoutActions.addGoldenLayoutTab.mockImplementation(() => ({}));
    soroNavigationMenuContent = await mountWithDefaults(SidebarMenu, defaults);
    vi.clearAllMocks();
    window.localStorage.clear();
  });

  it('displays several buttons to allow adding new golden layout tabs', async () => {
    const windowControls = soroNavigationMenuContent.find('.window-controls');

    const tabButtons = windowControls.findAllComponents({
      name: 'soro-button'
    });
    tabButtons[0].vm.$emit('click');
    tabButtons[3].vm.$emit('click');

    expect(tabButtons).toHaveLength(4);
    // Buttons 2 and 3 should be disabled
    expect(tabButtons[1].attributes('disabled')).toBeDefined();
    expect(tabButtons[2].attributes('disabled')).toBeDefined();
    // Buttons 1 and 4 should have added golden layout tabs as in their click order above
    expect(goldenLayoutActions.addGoldenLayoutTab).toHaveBeenCalledTimes(2);
    expect(goldenLayoutActions.addGoldenLayoutTab).toHaveBeenNthCalledWith(
      1,
      expect.any(Object),
      { componentTechnicalName: ComponentTechnicalName.INFRASTRUCTURE }
    );
    expect(goldenLayoutActions.addGoldenLayoutTab).toHaveBeenNthCalledWith(
      2,
      expect.any(Object),
      { componentTechnicalName: ComponentTechnicalName.ORDERING_GRAPH }
    );
  });

  it('displays a select component for infrastructures', async () => {
    infrastructureState.infrastructures.push('foo-bar', 'kung-foo');
    const dataSelects = soroNavigationMenuContent.find('.data-selects');

    const selects = dataSelects.findAllComponents({ name: 'soro-select' });
    selects[0].vm.$emit('select', 'some-infrastructure');

    expect(selects[0].vm.$props.options).toStrictEqual(['foo-bar', 'kung-foo']);
    expect(infrastructureActions.load).toHaveBeenCalledWith(
      expect.any(Object),
      'some-infrastructure'
    );
  });

  it('displays a select component for timetables', async () => {
    timetableState.timetables.push('foo-bar-part-two', 'kung-foo-part-two');
    const dataSelects = soroNavigationMenuContent.find('.data-selects');

    const selects = dataSelects.findAllComponents({ name: 'soro-select' });
    selects[1].vm.$emit('select', 'some-timetable');

    expect(selects[1].vm.$props.options).toStrictEqual([
      'foo-bar-part-two',
      'kung-foo-part-two'
    ]);
    expect(timetableActions.load).toHaveBeenCalledWith(
      expect.any(Object),
      'some-timetable'
    );
  });

  it('contains menu settings', async () => {
    const menuSettings = soroNavigationMenuContent.findComponent({
      name: 'menu-settings'
    });

    expect(menuSettings.exists()).toBe(true);
  });

  it('displays a dev tool to clear local storage', async () => {
    const devTools =
      soroNavigationMenuContent.findComponent<VExpansionPanels>('.dev-tools');
    window.localStorage.setItem('some-key', 'some-value');

    const devToolButtons = devTools.findAllComponents({ name: 'soro-button' });
    devToolButtons[0].vm.$emit('click');
    expect(window.localStorage).toHaveLength(0);
  });
});
