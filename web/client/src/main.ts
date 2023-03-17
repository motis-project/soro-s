import { createApp } from 'vue';
import App from './App.vue';
import { Store } from 'vuex';
import { vuetify, VuetifyExtension } from '@/vuetify';
import { createGlobalStore } from '@/stores/stores';

const store = createGlobalStore() as Store<undefined> & VuetifyExtension;
// Make vuetify accessible in stores with this.$vuetify just like in components
store.$vuetify = vuetify;

createApp(App)
    .use(store)
    .use(vuetify)
    .mount('#app');
