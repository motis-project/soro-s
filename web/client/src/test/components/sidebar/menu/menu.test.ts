import { VueWrapper } from '@vue/test-utils';
import { VExpansionPanels } from 'vuetify/components';

import SidebarMenu from '@/components/sidebar/menu/menu.vue';
import { GoldenLayoutNamespace } from '@/stores/golden-layout-store';
import { ComponentTechnicalName } from '@/golden-layout/golden-layout-constants';
import { SidebarNamespace, SidebarState } from '@/stores/sidebar-store';

import { mountWithDefaults } from '@/test/helpers/mount-with-defaults';

describe('sidebar-menu', async () => {
  let soroNavigationMenuContent: VueWrapper<any>;
  const goldenLayoutActions = { addGoldenLayoutTab: vi.fn() };
  const sidebarState: SidebarState = {
    infrastructures: [],
    currentInfrastructure: '',
    timetables: [],
    currentTimetable: '',
    currentSearchResults: [],
    highlightedStationRoutes: [],
    highlightedInterlockingRoutes: [],
    trainIdsFilter: []
  };

  const defaults = {
    store: {
      [GoldenLayoutNamespace]: { actions: goldenLayoutActions },
      [SidebarNamespace]: {
        state: sidebarState,
        actions: {
          loadInfrastructure: vi.fn(),
          loadTimetable: vi.fn()
        }
      }
    }
  };

  beforeEach(async () => {
    goldenLayoutActions.addGoldenLayoutTab.mockImplementation(() => ({}));
    defaults.store[SidebarNamespace].state.infrastructures = [];
    defaults.store[SidebarNamespace].state.timetables = [];
    soroNavigationMenuContent = await mountWithDefaults(SidebarMenu, defaults);
    vi.clearAllMocks();
    window.localStorage.clear();
  });

  it('displays several buttons to allow adding new golden layout tabs', async () => {
    const windowControls = soroNavigationMenuContent.find('.window-controls');
    const tabButtons = windowControls.findAllComponents({
      name: 'soro-button'
    });

    const dataSelects = soroNavigationMenuContent.find('.data-selects');

    const selects = dataSelects.findAllComponents({ name: 'soro-select' });

    // all buttons should be disabled at the beginning
    expect(tabButtons).toHaveLength(4);
    expect(tabButtons[0].attributes('disabled')).toBeDefined();
    expect(tabButtons[1].attributes('disabled')).toBeDefined();
    expect(tabButtons[2].attributes('disabled')).toBeDefined();
    expect(tabButtons[3].attributes('disabled')).toBeDefined();

    expect(selects).toHaveLength(2);

    // add infrastructures
    sidebarState.infrastructures.push('some-infrastructure', 'some-other');

    expect(tabButtons[0].attributes('disabled')).toBe('');

    tabButtons[0].vm.$emit('click');

    selects[0].vm.$emit('select', 'some-infrastructure');
    sidebarState.timetables.push('some-timetable', 'some-other');

    expect(tabButtons[1].attributes('disabled')).toBe('');
    expect(selects[1].attributes('disabled')).toBeUndefined();

    selects[1].vm.$emit('select', 'some-timetable');

    tabButtons[1].vm.$emit('click');

    expect(tabButtons[2].attributes('disabled')).toBe('');

    tabButtons[2].vm.$emit('click');

    // Buttons 1 and 4 should have added golden layout tabs as in their click order above
    expect(goldenLayoutActions.addGoldenLayoutTab).toHaveBeenCalledTimes(3);
    expect(goldenLayoutActions.addGoldenLayoutTab).toHaveBeenNthCalledWith(
      1,
      expect.any(Object),
      { componentTechnicalName: ComponentTechnicalName.INFRASTRUCTURE }
    );
    expect(goldenLayoutActions.addGoldenLayoutTab).toHaveBeenNthCalledWith(
      2,
      expect.any(Object),
      { componentTechnicalName: ComponentTechnicalName.TIMETABLE }
    );
    expect(goldenLayoutActions.addGoldenLayoutTab).toHaveBeenNthCalledWith(
      3,
      expect.any(Object),
      { componentTechnicalName: ComponentTechnicalName.ORDERING_GRAPH }
    );
  });

  it('displays a select component for infrastructures', async () => {
    sidebarState.infrastructures.push('foo-bar', 'kung-foo');
    const dataSelects = soroNavigationMenuContent.find('.data-selects');

    const selects = dataSelects.findAllComponents({ name: 'soro-select' });
    selects[0].vm.$emit('select', 'some-infrastructure');

    expect(selects[0].vm.$props.options).toStrictEqual(['foo-bar', 'kung-foo']);
    expect(
      defaults.store[SidebarNamespace].actions.loadInfrastructure
    ).toHaveBeenCalledWith(expect.any(Object), 'some-infrastructure');
  });

  it('displays a select component for timetables', async () => {
    sidebarState.timetables.push('foo-bar-part-two', 'kung-foo-part-two');
    const dataSelects = soroNavigationMenuContent.find('.data-selects');

    const selects = dataSelects.findAllComponents({ name: 'soro-select' });
    selects[1].vm.$emit('select', 'some-timetable');

    expect(selects[1].vm.$props.options).toStrictEqual([
      'foo-bar-part-two',
      'kung-foo-part-two'
    ]);
    expect(
      defaults.store[SidebarNamespace].actions.loadTimetable
    ).toHaveBeenCalledWith(expect.any(Object), 'some-timetable');
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
