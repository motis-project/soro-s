import { SidebarNamespace, SidebarStore } from '@/stores/sidebar-store';
import { SettingsNamespace, SettingsStore } from '@/stores/settings-store';
import {
  GoldenLayoutNamespace,
  GoldenLayoutStore
} from '@/stores/golden-layout-store';
import { createStore, Store } from 'vuex';
import SoroClient from '@/util/SoroClient';

export interface GlobalState {
  soroClient: SoroClient;
  showOverlay: false;
  selectedOverlay: 'menu';
}

export const createGlobalStore = () => {
  const store = createStore<GlobalState>({
    state() {
      return {
        soroClient: new SoroClient(window.origin),
        showOverlay: false,
        selectedOverlay: 'menu'
      };
    },

    mutations: {
      setShowOverlay(state, showOverlay) {
        state.showOverlay = showOverlay;
      },
      setSelectedOverlay(state, selectedOverlay) {
        state.selectedOverlay = selectedOverlay;
      }
    }
  });

  store.registerModule(SidebarNamespace, SidebarStore);
  store.registerModule(SettingsNamespace, SettingsStore);
  store.registerModule(GoldenLayoutNamespace, GoldenLayoutStore);

  return store;
};

// this gives us the global state type safe via this.$store.state in components
// not pretty, but works for now
declare module '@vue/runtime-core' {
  interface ComponentCustomProperties {
    $store: Store<GlobalState>;
  }
}
