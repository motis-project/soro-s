import { createApp } from 'vue'
import './style.ts'
import App from './App.vue'
import { InfrastructureStore, InfrastructureNameSpace } from './stores/infrastructure-store.js'
import { TimetableStore, TimetableNamespace } from './stores/timetable-store.js'
import { createStore } from "vuex";

const store = createStore({
    modules: {
        [InfrastructureNameSpace]: InfrastructureStore,
        [TimetableNamespace]: TimetableStore,
    }
})
const app = createApp(App);
app.use(store)
app.mount('#app')
