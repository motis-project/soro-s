import { Module } from 'vuex';
import { VuetifyExtension } from '@/vuetify';

export type SettingsState = {
    darkLightModePreference: typeof DarkLightModes[keyof typeof DarkLightModes],
    theme: typeof DarkLightModes.DARK | typeof DarkLightModes.LIGHT,
    primaryColor: string | null,
}

export const DarkLightModes = {
    DARK: 'dark',
    LIGHT: 'light',
    OS: 'os',
};

export const SettingsNamespace = 'settings';

export const SettingsStore: Module<SettingsState, unknown> = {
    namespaced: true,

    state() {
        return {
            darkLightModePreference: DarkLightModes.OS,
            theme: DarkLightModes.DARK,
            primaryColor: null,
        };
    },

    mutations: {
        setDarkLightModePreference(state, darkLightModePreference) {
            state.darkLightModePreference = darkLightModePreference;
        },

        setTheme(this: VuetifyExtension, state, theme) {
            state.theme = theme;
            // We need to set the theme globally in vuetify to access its properties in components
            this.$vuetify.theme.global.name.value = theme;
        },

        setPrimaryColor(this: VuetifyExtension, state, primaryColor) {
            state.primaryColor = primaryColor;
            // We need to set the primary color globally in vuetify to access its properties in components
            const themes = this.$vuetify.theme.themes.value;
            Object.keys(themes).forEach((themeKey) => themes[themeKey].colors.primary = primaryColor);
        },

        restoreVuetifyThemePrimaryColor(this: VuetifyExtension, state) {
            state.primaryColor = this.$vuetify.theme.global.current.value.colors.primary;
        },
    },

    actions: {
        async loadSettings({ dispatch, commit }) {
            const storage = window.localStorage;
            const darkLightModePreference = storage.getItem('darkLightModePreference');
            if (darkLightModePreference) {
                commit('setDarkLightModePreference', darkLightModePreference);
            }

            await dispatch('initThemeListener');
            await dispatch('applyTheme');

            const primaryColor = storage.getItem('primaryColor');
            if (primaryColor) {
                commit('setPrimaryColor', primaryColor);
            } else {
                commit('restoreVuetifyThemePrimaryColor');
            }
        },

        saveSettings({ state }) {
            window.localStorage.setItem('darkLightModePreference', state.darkLightModePreference);
            if (state.primaryColor) {
                window.localStorage.setItem('primaryColor', state.primaryColor);
            }
        },

        async setDarkLightModePreference({ commit, dispatch }, preference) {
            commit('setDarkLightModePreference', preference);
            await dispatch('applyTheme');
            await dispatch('saveSettings');
        },

        initThemeListener({ commit, state }) {
            const themeMediaQuery = window.matchMedia('(prefers-color-scheme: dark)');

            themeMediaQuery.addEventListener('change', (event) => {
                if (state.darkLightModePreference !== DarkLightModes.OS) {
                    return;
                }

                commit('setTheme', event.matches ? DarkLightModes.DARK : DarkLightModes.LIGHT);
            });
        },

        applyTheme({ commit, state }) {
            if (state.darkLightModePreference !== DarkLightModes.OS) {
                commit('setTheme', state.darkLightModePreference);

                return;
            }

            const themeMediaQuery = window.matchMedia('(prefers-color-scheme: dark)');
            commit('setTheme', themeMediaQuery.matches ? DarkLightModes.DARK : DarkLightModes.LIGHT);
        },

        async setPrimaryColor({ commit, dispatch }, primaryColor) {
            commit('setPrimaryColor', primaryColor);
            await dispatch('saveSettings');
        },
    },
};