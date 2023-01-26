<template>
    <div>
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
                    color="primary"
                    flat
                    @click="overlay = !overlay"
                >
                    <i class="material-icons">menu</i>
                </v-btn>
                <v-btn
                    ref="stationDetailButton"
                    class="overlay-tab-button sub-overlay-tab-button"
                    flat
                    disabled
                >
                    <i class="material-icons">home</i>
                </v-btn>
                <v-btn
                    ref="disruptionDetailButton"
                    class="overlay-tab-button sub-overlay-tab-button"
                    flat
                    disabled
                >
                    <i class="material-icons">train</i>
                </v-btn>
            </div>
            <soro-navigation-content @add-golden-layout-tab="addTab" />
        </v-navigation-drawer>
    </div>
</template>

<script setup lang="ts">
import SoroNavigationContent from '@/components/navigation/soro-navigation-content.vue';
</script>

<script lang="ts">
import { defineComponent } from 'vue';

export default defineComponent({
    name: 'SoroNavigation',

    emits: ['add-golden-layout-tab'],
    
    data() {
        return {
            overlay: false,
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

nav.v-navigation-drawer--active .overlay-tabs { /* stylelint-disable-line selector-class-pattern */
    position: absolute;
    left: var(--overlay-width);
}

nav:not(.v-navigation-drawer--active) .overlay-tabs { /* stylelint-disable-line selector-class-pattern */
    position: fixed;
    left: calc(1.1 * var(--overlay-width));
}

.overlay-tab-button {
    border-bottom-left-radius: 0;
    border-top-left-radius: 0;
}

.sub-overlay-tab-button {
    width: 30px;
    height: 40px;
    margin-top: 10px;
    background: var(--dialog-color);
    color: var(--secondary-text-color);
    cursor: pointer;
    display: flex;
    align-items: center;
    justify-content: center;
    opacity: 1;
    transition: all 0.2s ease;
}

.sub-overlay-tab-button.enabled {
    color: white;
    background: var(--highlight-color);
}
</style>