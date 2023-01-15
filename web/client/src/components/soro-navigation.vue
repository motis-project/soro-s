<template>
    <v-navigation-drawer
        v-model="overlay"
        permanent
        width="auto"
    >
        <div
            ref="subOverlayTabs"
            class="overlay-tabs"
        >
            <v-btn
                class="overlay-tab-button"
                @click="overlay = !overlay"
            >
                <i class="material-icons">menu</i>
            </v-btn>
            <v-btn
                ref="stationDetailButton"
                class="overlay-tab-button sub-overlay-tab-button"
                disabled
            >
                <i class="material-icons">home</i>
            </v-btn>
            <v-btn
                ref="disruptionDetailButton"
                class="overlay-tab-button sub-overlay-tab-button"
                disabled
            >
                <i class="material-icons">train</i>
            </v-btn>
        </div>
        <soro-navigation-content @add-golden-layout-tab="addTab" />
    </v-navigation-drawer>
</template>

<script setup lang="ts">
import SoroNavigationContent from '@/components/soro-navigation-content.vue';
</script>

<script lang="ts">
import { defineComponent } from 'vue';

export default defineComponent({
    name: 'SoroNavigation',

    emits: ['add-golden-layout-tab'],
    
    data() {
        return {
            overlay: true,
        };
    },
    
    methods: {
        addTab(tab: any) {
            this.$emit('add-golden-layout-tab', tab); 
        }
    }
});
</script>

<style scoped>
.overlay-tabs {
    margin-top: 40px;
    z-index: 10;
}

nav.v-navigation-drawer--active .overlay-tabs {
    position: absolute;
    left: var(--overlay-width);
}

nav:not(.v-navigation-drawer--active) .overlay-tabs {
    position: fixed;
    left: calc(1.1 * var(--overlay-width));
}

.overlay-tab-button {
    border-bottom-left-radius: 0;
    border-top-left-radius: 0;
}
</style>