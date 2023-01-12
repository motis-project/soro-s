import { createApp } from 'vue';
import './style.ts';
import App from './App.vue';
import { InfrastructureStore, InfrastructureNamespace } from '@/stores/infrastructure-store';
import { TimetableStore, TimetableNamespace } from '@/stores/timetable-store';
import { createStore } from 'vuex';
import { SettingsNamespace, SettingsStore } from '@/stores/settings-store';

const store = createStore({
    modules: {
        [InfrastructureNamespace]: InfrastructureStore,
        [TimetableNamespace]: TimetableStore,
        [SettingsNamespace]: SettingsStore,
    }
});
const app = createApp(App);
app.use(store);
app.mount('#app');
