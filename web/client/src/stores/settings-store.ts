import { Module } from 'vuex';

type SettingsState = {
    darkLightModePreference: 'dark' | 'light',
}

export const SettingsNamespace = 'settings';

export const SettingsStore: Module<SettingsState, undefined> = {
    namespaced: true,

    state() {
        return {
            darkLightModePreference: 'light',
        };
    },

    mutations: {
        setDarkLightModePreference(state, darkLightModePreference) {
            state.darkLightModePreference = darkLightModePreference;
        },
    },
};