import { createVuetify } from 'vuetify';
import { customDarkTheme, customLightTheme } from '@/style';
import { aliases, md } from 'vuetify/iconsets/md';

export type VuetifyExtension = { $vuetify: typeof vuetify };

export const vuetify =  createVuetify({
    theme: {
        defaultTheme: 'light',
        themes: {
            light: customLightTheme,
            dark: customDarkTheme,
        },
    },
    icons: {
        defaultSet: 'md',
        aliases,
        sets: {
            md,
        },
    },
});