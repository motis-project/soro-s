import { createApp } from 'vue';
import App from './App.vue';
import { Store } from 'vuex';
import { vuetify, VuetifyExtension } from '@/vuetify';
import { GlobalState, createGlobalStore } from '@/stores/stores';
import { devMode } from '@/util/devMode';

const store = createGlobalStore() as Store<GlobalState> & VuetifyExtension;
// Make vuetify accessible in stores with this.$vuetify just like in components
store.$vuetify = vuetify;

// clear console on hot module reload for vite
// @ts-ignore
if (import.meta.hot && devMode()) {
  // @ts-ignore
  import.meta.hot.on(
    'vite:beforeUpdate',
    /* eslint-disable-next-line no-console */
    () => console.clear()
  );
}

createApp(App).use(store).use(vuetify).mount('#app');
