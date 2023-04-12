import { Commit, Dispatch } from 'vuex';
import { DarkLightModes, SettingsState, SettingsStore } from '@/stores/settings-store';

describe('settings-store', async () => {
    let state: SettingsState;
    let commit: Commit;
    let dispatch: Dispatch;
    let themeMediaQuery: any;
    const thisContextWithVuetify = {
        $vuetify: {
            theme: {
                global: {
                    current: {
                        value: {
                            colors: { primary: 'vuetify-primary-color' },
                        },
                    },
                    name: { value: 'some-global-theme-name' },
                },
                themes: { value: ({} as Record<string, any>) },
            },
        },
    };

    beforeEach(async () => {
        state = {
            darkLightModePreference: DarkLightModes.DARK,
            theme: DarkLightModes.DARK,
            primaryColor: null,
        };
        commit = vi.fn((mutationName: string, args: any) =>
            SettingsStore.mutations?.[mutationName]?.call(thisContextWithVuetify, state, args));
        dispatch = vi.fn((actionName: string, value: any) =>
            // @ts-ignore
            SettingsStore.actions[actionName].call(
                thisContextWithVuetify,
                {
                    state,
                    dispatch,
                    commit, 
                },
                value,
            ));
        themeMediaQuery = { addEventListener: vi.fn() };
        vi.spyOn(window, 'matchMedia').mockImplementation(() => themeMediaQuery);
        window.localStorage.clear();
    });

    it('provides a mutation which can set the primary color on all vuetify themes and in the state', async () => {
        state.primaryColor = 'old-color';
        thisContextWithVuetify.$vuetify.theme.themes.value['theme-a'] = {
            colors: {
                primary: 'theme-a-primary',
                secondary: 'theme-a-secondary',
            },
        };
        thisContextWithVuetify.$vuetify.theme.themes.value['theme-b'] = {
            colors: {
                primary: 'theme-b-primary',
                secondary: 'theme-b-secondary',
            },
        };

        // @ts-ignore
        SettingsStore.mutations.setPrimaryColor.call(thisContextWithVuetify, state, 'fancy-color');

        expect(state.primaryColor).toBe('fancy-color');
        expect(thisContextWithVuetify.$vuetify.theme.themes.value['theme-a'].colors).toStrictEqual({
            primary: 'fancy-color',
            secondary: 'theme-a-secondary',
        });
        expect(thisContextWithVuetify.$vuetify.theme.themes.value['theme-b'].colors).toStrictEqual({
            primary: 'fancy-color',
            secondary: 'theme-b-secondary',
        });
    });

    describe('loadSettings()', async () => {
        it('initializes the theme listener and applies the current theme', async () => {
            // @ts-ignore
            await SettingsStore.actions.loadSettings({
                commit,
                dispatch,
            });

            expect(dispatch).toHaveBeenCalledWith('initThemeListener');
            expect(dispatch).toHaveBeenCalledWith('applyTheme');
        });

        it('sets the preference in the state when it is given in local storage', async () => {
            state.darkLightModePreference = DarkLightModes.LIGHT;
            window.localStorage.setItem('darkLightModePreference', DarkLightModes.DARK);

            // @ts-ignore
            await SettingsStore.actions.loadSettings({
                commit,
                dispatch,
            });

            expect(state.darkLightModePreference).toBe(DarkLightModes.DARK);
        });

        it('does not set the preference in the state when it is not given in local storage', async () => {
            state.darkLightModePreference = DarkLightModes.LIGHT;
            window.localStorage.removeItem('darkLightModePreference');

            // @ts-ignore
            await SettingsStore.actions.loadSettings({
                commit,
                dispatch,
            });

            expect(state.darkLightModePreference).toBe(DarkLightModes.LIGHT);
        });

        it('sets the primary color in the state when it is given in local storage', async () => {
            state.primaryColor = 'foo';
            window.localStorage.setItem('primaryColor', 'some-primary-color');

            // @ts-ignore
            await SettingsStore.actions.loadSettings({
                commit,
                dispatch,
            });

            expect(state.primaryColor).toBe('some-primary-color');
        });

        it('restores the vuetify theme primary color to the state when the primary color is not given in local storage', async () => {
            state.primaryColor = 'foo';
            thisContextWithVuetify.$vuetify.theme.global.current.value.colors.primary = 'some-new-primary';
            window.localStorage.removeItem('primaryColor');

            // @ts-ignore
            await SettingsStore.actions.loadSettings({
                commit,
                dispatch,
            });

            expect(state.primaryColor).toBe('some-new-primary');
        });
    });

    describe('saveSettings()', async () => {
        it('sets the dark light mode preference in local storage', async () => {
            window.localStorage.setItem('darkLightModePreference', 'some-old-preference');
            state.darkLightModePreference = DarkLightModes.DARK;

            // @ts-ignore
            await SettingsStore.actions.saveSettings({ state });

            expect(window.localStorage.getItem('darkLightModePreference')).toBe(DarkLightModes.DARK);
        });

        it('sets the primary color in local storage when it is set in the state', async () => {
            window.localStorage.setItem('primaryColor', 'previous-color');
            state.primaryColor = 'new-color';

            // @ts-ignore
            await SettingsStore.actions.saveSettings({ state });

            expect(window.localStorage.getItem('primaryColor')).toBe('new-color');
        });

        it('does not set the primary color in local storage when it is not set in the state', async () => {
            window.localStorage.setItem('primaryColor', 'previous-color');
            state.primaryColor = null;

            // @ts-ignore
            await SettingsStore.actions.saveSettings({ state });

            expect(window.localStorage.getItem('primaryColor')).toBe('previous-color');
        });
    });

    it('saves the preference in the state, applies the theme and saves the preference when calling \'setDarkLightModePreference()\'', async () => {
        state.darkLightModePreference = DarkLightModes.LIGHT;

        // @ts-ignore
        await SettingsStore.actions.setDarkLightModePreference(
            {
                commit,
                dispatch,
            },
            DarkLightModes.DARK,
        );

        expect(state.darkLightModePreference).toBe(DarkLightModes.DARK);
        expect(dispatch).toHaveBeenCalledWith('applyTheme');
        expect(window.localStorage.getItem('darkLightModePreference')).toBe(DarkLightModes.DARK);
    });

    describe('initThemeListener()', async () => {
        it('registers a \'change\' event listener on the color scheme media query of the window', async () => {
            // @ts-ignore
            await SettingsStore.actions.initThemeListener({
                commit,
                state,
            });

            expect(window.matchMedia).toHaveBeenCalledWith('(prefers-color-scheme: dark)');
            expect(themeMediaQuery.addEventListener).toHaveBeenCalledWith('change', expect.any(Function));
        });

        it('does not set the theme when the media query emits a \'change\' event and the preference is on LIGHT', async () => {
            state.theme = DarkLightModes.DARK;
            state.darkLightModePreference = DarkLightModes.LIGHT;

            // @ts-ignore
            await SettingsStore.actions.initThemeListener({
                commit,
                state,
            });

            const handler = themeMediaQuery.addEventListener.mock.lastCall[1];
            handler();

            expect(state.theme).toBe(DarkLightModes.DARK);
        });

        it('does not set the theme when the media query emits a \'change\' event and the preference is on DARK', async () => {
            state.theme = DarkLightModes.LIGHT;
            state.darkLightModePreference = DarkLightModes.DARK;

            // @ts-ignore
            await SettingsStore.actions.initThemeListener({
                commit,
                state,
            });

            const handler = themeMediaQuery.addEventListener.mock.lastCall[1];
            handler();

            expect(state.theme).toBe(DarkLightModes.LIGHT);
        });

        it('sets the theme to dark when the media query emits a positive \'change\' event and the preference is on OS', async () => {
            state.theme = DarkLightModes.LIGHT;
            state.darkLightModePreference = DarkLightModes.OS;

            // @ts-ignore
            await SettingsStore.actions.initThemeListener({
                commit,
                state,
            });

            const handler = themeMediaQuery.addEventListener.mock.lastCall[1];
            handler({ matches: true });

            expect(state.theme).toBe(DarkLightModes.DARK);
        });

        it('sets the theme to light when the preference is on OS and the window prefers a light color scheme', async () => {
            state.theme = DarkLightModes.DARK;
            state.darkLightModePreference = DarkLightModes.OS;

            // @ts-ignore
            await SettingsStore.actions.initThemeListener({
                commit,
                state,
            });

            const handler = themeMediaQuery.addEventListener.mock.lastCall[1];
            handler({ matches: false });

            expect(state.theme).toBe(DarkLightModes.LIGHT);
        });
    });

    describe('applyTheme()', async () => {
        it('sets the theme to dark when the preference is on DARK', async () => {
            state.darkLightModePreference = DarkLightModes.DARK;

            // @ts-ignore
            await SettingsStore.actions.applyTheme({
                commit,
                state,
            });

            expect(state.theme).toBe(DarkLightModes.DARK);
            expect(thisContextWithVuetify.$vuetify.theme.global.name.value).toBe(DarkLightModes.DARK);
        });

        it('sets the theme to light when the preference is on LIGHT', async () => {
            state.darkLightModePreference = DarkLightModes.LIGHT;

            // @ts-ignore
            await SettingsStore.actions.applyTheme({
                commit,
                state,
            });

            expect(state.theme).toBe(DarkLightModes.LIGHT);
            expect(thisContextWithVuetify.$vuetify.theme.global.name.value).toBe(DarkLightModes.LIGHT);
        });

        it('sets the theme to dark when the preference is on OS and the window prefers a dark color scheme', async () => {
            const mediaQuery: any = { matches: true };
            vi.spyOn(window, 'matchMedia').mockImplementation(() => mediaQuery);
            state.darkLightModePreference = DarkLightModes.OS;

            // @ts-ignore
            await SettingsStore.actions.applyTheme({
                commit,
                state,
            });

            expect(window.matchMedia).toHaveBeenCalledWith('(prefers-color-scheme: dark)');
            expect(state.theme).toBe(DarkLightModes.DARK);
            expect(thisContextWithVuetify.$vuetify.theme.global.name.value).toBe(DarkLightModes.DARK);
        });

        it('sets the theme to light when the preference is on OS and the window prefers a light color scheme', async () => {
            const mediaQuery: any = { matches: false };
            vi.spyOn(window, 'matchMedia').mockImplementation(() => mediaQuery);
            state.darkLightModePreference = DarkLightModes.OS;

            // @ts-ignore
            await SettingsStore.actions.applyTheme({
                commit,
                state,
            });

            expect(window.matchMedia).toHaveBeenCalledWith('(prefers-color-scheme: dark)');
            expect(state.theme).toBe(DarkLightModes.LIGHT);
            expect(thisContextWithVuetify.$vuetify.theme.global.name.value).toBe(DarkLightModes.LIGHT);
        });
    });

    it('sets the primary color in the state and saves it in local storage when setting the primary color', async () => {
        // @ts-ignore
        await SettingsStore.actions.setPrimaryColor(
            {
                commit,
                dispatch,
            },
            'some-primary-color',
        );

        expect(state.primaryColor).toBe('some-primary-color');
        expect(window.localStorage.getItem('primaryColor')).toBe('some-primary-color');
    });
});
