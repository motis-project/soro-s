import { InfrastructureNamespace, InfrastructureStore } from '@/stores/infrastructure-store';
import { TimetableNamespace, TimetableStore } from '@/stores/timetable-store';
import { SettingsNamespace, SettingsStore } from '@/stores/settings-store';
import { GoldenLayoutNamespace, GoldenLayoutStore } from '@/stores/golden-layout-store';
import { createStore } from 'vuex';

export const createGlobalStore = () => {
    const store = createStore({});

    store.registerModule(InfrastructureNamespace, InfrastructureStore);
    store.registerModule(TimetableNamespace, TimetableStore);
    store.registerModule(SettingsNamespace, SettingsStore);
    store.registerModule(GoldenLayoutNamespace, GoldenLayoutStore);

    return store;
};