import { mountWithDefaults } from '@/test-utils/mount-with-defaults';
import SoroNavigationMenuContent from './soro-navigation-menu-content.vue';
import { VueWrapper } from '@vue/test-utils';
import { GoldenLayoutNamespace } from '@/stores/golden-layout-store';
import { ComponentTechnicalName, GLComponentTitles } from '@/golden-layout/golden-layout-constants';
import { VExpansionPanels } from 'vuetify/components';
import { InfrastructureNamespace, InfrastructureState } from '@/stores/infrastructure-store';
import { TimetableNamespace, TimetableState } from '@/stores/timetable-store';

vi.mock('@/golden-layout/golden-layout-constants', () => ({
    ComponentTechnicalName: {
        INFRASTRUCTURE: 'infrastructure',
        ORDERING_GRAPH: 'ordering_graph',
    },
    GLComponentTitles: {
        infrastructure: 'some-infrastructure-title',
        ordering_graph: 'some-ordering-graph-title',
    },
}));

describe('soro-navigation-menu-content', async () => {
    let soroNavigationMenuContent: VueWrapper<any>;
    const goldenLayoutActions = { addGoldenLayoutTab: vi.fn() };
    const infrastructureState: InfrastructureState = {
        infrastructures: [],
        currentInfrastructure: '',
        currentSearchedMapPositions: [],
    };
    const timetableState: TimetableState = {
        timetables: [],
        currentTimetable: '',
    };
    const infrastructureActions = { load: vi.fn() };
    const timetableActions = { load: vi.fn() };

    const defaults = {
        store: {
            [GoldenLayoutNamespace]: { actions: goldenLayoutActions },
            [InfrastructureNamespace]: {
                state: infrastructureState,
                actions: infrastructureActions,
            },
            [TimetableNamespace]: {
                state: timetableState,
                actions: timetableActions,
            },
        },
    };

    beforeEach(async () => {
        goldenLayoutActions.addGoldenLayoutTab.mockImplementation(() => ({}));
        soroNavigationMenuContent = await mountWithDefaults(SoroNavigationMenuContent, defaults);
        vi.clearAllMocks();
        window.localStorage.clear();
    });

    it('displays several buttons to allow adding new golden layout tabs', async () => {
        const windowControls = soroNavigationMenuContent.find('.window-controls');

        const tabButtons = windowControls.findAllComponents({ name: 'soro-button' });
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
            {
                componentTechnicalName: ComponentTechnicalName.INFRASTRUCTURE,
                title: GLComponentTitles[ComponentTechnicalName.INFRASTRUCTURE],
            },
        );
        expect(goldenLayoutActions.addGoldenLayoutTab).toHaveBeenNthCalledWith(
            2,
            expect.any(Object),
            {
                componentTechnicalName: ComponentTechnicalName.ORDERING_GRAPH,
                title: GLComponentTitles[ComponentTechnicalName.ORDERING_GRAPH],
            },
        );
    });

    it('displays a select component for infrastructures', async () => {
        infrastructureState.infrastructures.push(
            'foo-bar',
            'kung-foo',
        );
        const dataSelects = soroNavigationMenuContent.find('.data-selects');

        const selects = dataSelects.findAllComponents({ name: 'soro-select' });
        selects[0].vm.$emit('select', 'some-infrastructure');

        expect(selects[0].vm.$props.options).toStrictEqual([
            'foo-bar',
            'kung-foo',
        ]);
        expect(infrastructureActions.load).toHaveBeenCalledWith(
            expect.any(Object),
            'some-infrastructure',
        );
    });

    it('displays a select component for timetables', async () => {
        timetableState.timetables.push(
            'foo-bar-part-two',
            'kung-foo-part-two',
        );
        const dataSelects = soroNavigationMenuContent.find('.data-selects');

        const selects = dataSelects.findAllComponents({ name: 'soro-select' });
        selects[1].vm.$emit('select', 'some-timetable');

        expect(selects[1].vm.$props.options).toStrictEqual([
            'foo-bar-part-two',
            'kung-foo-part-two',
        ]);
        expect(timetableActions.load).toHaveBeenCalledWith(
            expect.any(Object),
            'some-timetable',
        );
    });

    it('contains a station search with extended options', async () => {
        const stationSearch = soroNavigationMenuContent.findComponent({ name: 'station-search' });

        expect(stationSearch.exists()).toBe(true);
        expect(stationSearch.vm.$props).toStrictEqual(expect.objectContaining({
            showExtendedLink: true,
        }));
    });

    it('emits a \'change-overlay\' event when the station search signals to change to extended search', async () => {
        const stationSearch = soroNavigationMenuContent.findComponent({ name: 'station-search' });

        stationSearch.vm.$emit('change-to-extended');
        const changeOverlayEvents = soroNavigationMenuContent.emitted('change-overlay');
        expect(changeOverlayEvents).toHaveLength(1);
        expect(changeOverlayEvents?.[0]).toStrictEqual(['search']);
    });

    it('contains menu settings', async () => {
        const menuSettings = soroNavigationMenuContent.findComponent({ name: 'menu-settings' });

        expect(menuSettings.exists()).toBe(true);
    });

    it('displays a dev tool to clear local storage', async () => {
        const devTools = soroNavigationMenuContent.findComponent<VExpansionPanels>('.dev-tools');
        window.localStorage.setItem('some-key', 'some-value');

        const devToolButtons = devTools.findAllComponents({ name: 'soro-button' });
        devToolButtons[0].vm.$emit('click');
        expect(window.localStorage).toHaveLength(0);
    });
});
