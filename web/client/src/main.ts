import { createApp } from 'vue';
import App from './App.vue';
import { InfrastructureStore, InfrastructureNamespace } from '@/stores/infrastructure-store';
import { TimetableStore, TimetableNamespace } from '@/stores/timetable-store';
import { createStore, Store } from 'vuex';
import { SettingsNamespace, SettingsStore } from '@/stores/settings-store';
import { vuetify, VuetifyExtension } from '@/vuetify';

const store = createStore({
    modules: {
        [InfrastructureNamespace]: InfrastructureStore,
        [TimetableNamespace]: TimetableStore,
        [SettingsNamespace]: SettingsStore,
    }
}) as Store<undefined> & VuetifyExtension;
// Make vuetify accessible in stores with this.$vuetify just like in components
store.$vuetify = vuetify;

createApp(App)
    .use(store)
    .use(vuetify)
    .mount('#app');
