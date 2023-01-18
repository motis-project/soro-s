import { createApp } from 'vue';
import { applicationTheme } from './style';
import App from './App.vue';
import { InfrastructureStore, InfrastructureNamespace } from '@/stores/infrastructure-store';
import { TimetableStore, TimetableNamespace } from '@/stores/timetable-store';
import { createStore } from 'vuex';
import { SettingsNamespace, SettingsStore } from '@/stores/settings-store';
import { createVuetify } from 'vuetify';
import { aliases, md } from 'vuetify/iconsets/md';

const store = createStore({
    modules: {
        [InfrastructureNamespace]: InfrastructureStore,
        [TimetableNamespace]: TimetableStore,
        [SettingsNamespace]: SettingsStore,
    }
});

createApp(App)
    .use(store)
    .use(createVuetify({
        theme: applicationTheme,
        icons: {
            defaultSet: 'md',
            aliases,
            sets: {
                md,
            }
        },
    }))
    .mount('#app');
