<template>
  <div>
    <v-navigation-drawer :model-value="showOverlay" permanent width="auto">
      <div ref="subOverlayTabs" class="overlay-tabs">
        <v-btn
          v-for="overlay in overlays"
          :key="overlay.name"
          :color="selectedOverlay !== overlay.name ? '' : 'primary'"
          class="overlay-tab-button"
          flat
          @click="onOverlayButtonClicked(overlay.name)"
        >
          <i class="material-icons">{{ overlay.icon }}</i>
        </v-btn>
      </div>

      <div class="overlay-container">
        <sidebar-menu
          v-if="selectedOverlay === 'menu'"
          class="overlay"
          @change-overlay="(overlay) => (selectedOverlay = overlay)"
        />

        <sidebar-search v-if="selectedOverlay === 'search'" class="overlay" />

        <sidebar-station v-if="selectedOverlay === 'station'" class="overlay" />
      </div>
    </v-navigation-drawer>
  </div>
</template>

<script setup lang="ts">
import SidebarMenu from '@/components/sidebar/menu/menu.vue';
import SidebarSearch from '@/components/sidebar/search/search.vue';
import SidebarStation from '@/components/sidebar/station/station.vue';
</script>

<script lang="ts">
import { defineComponent } from 'vue';
import { mapMutations, mapState } from 'vuex';
import { SidebarNamespace } from '@/stores/sidebar-store';

type Overlay = {
  name: string;
  icon: string;
};

const overlays: Overlay[] = [
  {
    name: 'menu',
    icon: 'menu'
  },
  {
    name: 'search',
    icon: 'search'
  },
  {
    name: 'station',
    icon: 'home'
  }
];

export default defineComponent({
  name: 'SoroSidebar',

  data() {
    return {
      overlays
    };
  },

  computed: {
    ...mapState(SidebarNamespace, ['currentStation']),
    ...mapState(['showOverlay', 'selectedOverlay'])
  },

  watch: {
    currentStation(newStationId?: number) {
      if (!newStationId) {
        return;
      }

      this.setShowOverlay(true);
      this.setSelectedOverlay('station');
    }
  },

  methods: {
    ...mapMutations(['setShowOverlay', 'setSelectedOverlay']),

    onOverlayButtonClicked(selectedItem: string) {
      if (this.selectedOverlay === selectedItem) {
        this.setShowOverlay(!this.showOverlay);

        return;
      }

      if (!this.showOverlay) {
        this.setShowOverlay(true);
      }

      this.setSelectedOverlay(selectedItem);
    }
  }
});
</script>

<style scoped>
.overlay-tabs {
  margin-top: 40px;
  z-index: 10;
}

nav.v-navigation-drawer-active .overlay-tabs {
  position: absolute;
  left: var(--overlay-width);
}

nav:not(.v-navigation-drawer-active) .overlay-tabs {
  /* stylelint-disable-line selector-class-pattern */
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
