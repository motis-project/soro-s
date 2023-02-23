import { InfrastructureNamespace, InfrastructureStore } from '@/stores/infrastructure-store';
import { TimetableNamespace, TimetableStore } from '@/stores/timetable-store';
import { SettingsNamespace, SettingsStore } from '@/stores/settings-store';
import { GoldenLayoutNamespace, GoldenLayoutStore } from '@/stores/golden-layout-store';
import { createStore } from 'vuex';

export const createGlobalStore = () => createStore({
    modules: {
        [InfrastructureNamespace]: InfrastructureStore,
        [TimetableNamespace]: TimetableStore,
        [SettingsNamespace]: SettingsStore,
        [GoldenLayoutNamespace]: GoldenLayoutStore,
    }
});