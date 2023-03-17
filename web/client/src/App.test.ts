import { mountWithDefaults } from '@/test-utils/mount-with-defaults';
import { GoldenLayoutNamespace } from '@/stores/golden-layout-store';
import { TimetableNamespace } from '@/stores/timetable-store';
import { SettingsNamespace } from '@/stores/settings-store';
import { InfrastructureNamespace } from '@/stores/infrastructure-store';
import App from './App.vue';
import { ComponentTechnicalName, GLComponentNames, GLComponentTitles } from '@/golden-layout/golden-layout-constants';

describe('App', async () => {
    const infrastructureStoreMock = {
        // State must be given for a full mount to work even though not used directly in this component
        state: { currentSearchedMapPositions: [] },
        actions: { initialLoad: vi.fn() },
    };
    const timetableStoreMock = { actions: { initialLoad: vi.fn() } };
    const settingsStoreMock = {
        state: { theme: 'some-theme' },
        actions: { loadSettings: vi.fn() },
    };
    const goldenLayoutStoreMock = {
        mutations: { setRootComponent: vi.fn() },
        actions: { initGoldenLayout: vi.fn() },
    };
    const defaults = {
        store: {
            [InfrastructureNamespace]: infrastructureStoreMock,
            [TimetableNamespace]: timetableStoreMock,
            [SettingsNamespace]: settingsStoreMock,
            [GoldenLayoutNamespace]: goldenLayoutStoreMock,
        },
    };

    it('contains a theme provider that provides the theme set in the state to navigation and golden layout', async () => {
        settingsStoreMock.state.theme = 'some-theme';

        const app = await mountWithDefaults(App, defaults);
        const themeProvider = app.findComponent({ name: 'v-theme-provider' });

        expect(themeProvider.exists()).toBe(true);
        expect(themeProvider.props('theme')).toBe('some-theme');
        const navigation = themeProvider.findComponent({ name: 'soro-navigation' });
        const goldenLayoutAdapter = themeProvider.findComponent({ name: 'golden-layout-adapter' });
        expect(navigation.exists()).toBe(true);
        expect(goldenLayoutAdapter.exists()).toBe(true);
    });

    it('initializes the entire app context when it is mounted', async () => {
        const app = await mountWithDefaults(App, defaults);

        expect(settingsStoreMock.actions.loadSettings).toHaveBeenCalled();
        expect(infrastructureStoreMock.actions.initialLoad).toHaveBeenCalled();
        expect(timetableStoreMock.actions.initialLoad).toHaveBeenCalled();

        const goldenLayoutRoot = app.findComponent({ ref: 'GLayoutRoot' });
        expect(goldenLayoutStoreMock.mutations.setRootComponent).toHaveBeenCalledWith(
            expect.any(Object),
            goldenLayoutRoot.vm,
        );
        expect(goldenLayoutStoreMock.actions.initGoldenLayout).toHaveBeenCalledWith(
            expect.any(Object),
            {
                dimensions: { headerHeight: 36 },
                root: {
                    type: 'row',
                    content: [
                        {
                            type: 'column',
                            content: [
                                {
                                    title: GLComponentTitles[ComponentTechnicalName.INFRASTRUCTURE],
                                    type: 'component',
                                    componentType: GLComponentNames[ComponentTechnicalName.INFRASTRUCTURE],
                                },
                            ],
                        },
                    ],
                },
            },
        );
    });
});
