<template>
    <div>
        <v-navigation-drawer
            v-model="showOverlay"
            permanent
            width="auto"
        >
            <div
                ref="subOverlayTabs"
                class="overlay-tabs"
            >
                <v-btn
                    v-for="item in overlayTypes"
                    :key="item"
                    :color="selectedOverlay !== item ? '' : 'primary'"
                    class="overlay-tab-button"
                    flat
                    @click="onOverlayButtonClicked(item)"
                >
                    <i class="material-icons">{{ item }}</i>
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
            <div
                ref="overlayContainer"
                class="overlay-container"
            >
                <soro-navigation-menu-content
                    v-if="selectedOverlay === 'menu'"
                    class="overlay"
                    @change-overlay="overlay => selectedOverlay = overlay"
                />
                <soro-navigation-search-content
                    v-if="selectedOverlay === 'search'"
                    class="overlay"
                />
            </div>
        </v-navigation-drawer>
    </div>
</template>

<script setup lang="ts">
import SoroNavigationMenuContent from '@/components/navigation/soro-navigation-menu-content.vue';
import SoroNavigationSearchContent from '@/components/navigation/station-search/soro-navigation-search-content.vue';
</script>

<script lang="ts">
import { defineComponent } from 'vue';

const overlayTypes = ['menu', 'search'];

export default defineComponent({
    name: 'SoroNavigation',
    
    data() {
        return {
            overlayTypes,
            showOverlay: false,
            selectedOverlay: 'menu',
        };
    },

    methods: {
        onOverlayButtonClicked(selectedItem: string) {
            if (this.selectedOverlay === selectedItem) {
                this.showOverlay = !this.showOverlay;

                return;
            }

            if (!this.showOverlay) {
                this.showOverlay = true;
            }

            this.selectedOverlay = selectedItem;
        },
    },
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
    margin-bottom: 10px;
    color: rgb(var(--v-theme-on-surface));
}

.sub-overlay-tab-button {
    width: 30px;
    height: 40px;
    cursor: pointer;
    display: flex;
    align-items: center;
    justify-content: center;
    opacity: 1;
    transition: all 0.2s ease;
}

.overlay {
    width: var(--overlay-width);
    height: 95%;
}

.overlay-container {
    display: flex;
    align-items: flex-start;
    justify-content: center;
    transition: all 0.4s ease;
}
</style>