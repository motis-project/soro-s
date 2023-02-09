import { Module } from 'vuex';
import { VuetifyExtension } from '@/vuetify';

type SettingsState = {
    darkLightModePreference: typeof DarkLightModes[keyof typeof DarkLightModes],
    theme: typeof DarkLightModes.DARK | typeof DarkLightModes.LIGHT,
}

export const DarkLightModes = {
    DARK: 'dark',
    LIGHT: 'light',
    OS: 'os',
};

export const SettingsNamespace = 'settings';

export const SettingsStore: Module<SettingsState, undefined> = {
    namespaced: true,

    state() {
        return {
            darkLightModePreference: DarkLightModes.OS,
            theme: DarkLightModes.DARK,
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
        }
    },

    actions: {
        loadSettings({ dispatch, commit }) {
            const storage = window.localStorage;
            const darkLightModePreference = storage.getItem('darkLightModePreference');
            if (darkLightModePreference) {
                commit('setDarkLightModePreference', darkLightModePreference);
            }

            dispatch('initThemeListener');
            dispatch('applyTheme');
        },

        saveSettings({ state }) {
            window.localStorage.setItem('darkLightModePreference', state.darkLightModePreference);
        },

        setDarkLightModePreference({ commit, dispatch }, preference) {
            commit('setDarkLightModePreference', preference);
            dispatch('applyTheme');
            dispatch('saveSettings');
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
            const themeMediaQuery = window.matchMedia('(prefers-color-scheme: dark)');
            if (state.darkLightModePreference === DarkLightModes.OS) {
                commit('setTheme', themeMediaQuery.matches ? DarkLightModes.DARK : DarkLightModes.LIGHT);

                return;
            }

            commit('setTheme', state.darkLightModePreference);
        }
    }
};