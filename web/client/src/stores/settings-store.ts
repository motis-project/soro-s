import { Module } from 'vuex';
import { VuetifyExtension } from '@/vuetify';

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
        setDarkLightModePreference(this: VuetifyExtension, state, darkLightModePreference) {
            state.darkLightModePreference = darkLightModePreference;
            // We need to set the theme globally in vuetify to access its properties in components
            this.$vuetify.theme.global.name.value = darkLightModePreference;
        },
    },
};