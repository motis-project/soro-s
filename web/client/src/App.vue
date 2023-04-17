<template>
  <div class="full-height">
    <v-theme-provider :theme="theme">
      <v-layout>
        <soro-sidebar />
      </v-layout>
      <golden-layout-adapter
        ref="GLayoutRoot"
        class="golden-layout-root"
        :class="$vuetify.theme.themeClasses"
      />
    </v-theme-provider>
  </div>
</template>

<script setup lang="ts">
import GoldenLayoutAdapter from '@/golden-layout/golden-layout-adapter.vue';
import SoroSidebar from '@/components/sidebar/sidebar.vue';
</script>

<script lang="ts">
import { mapActions, mapMutations, mapState } from 'vuex';
import { SidebarNamespace } from '@/stores/sidebar-store';
import { defineComponent, ref } from 'vue';
import { LayoutConfig } from 'golden-layout';
import {
  ComponentTechnicalName,
  GLComponentNames,
  GLComponentTitles
} from '@/golden-layout/golden-layout-constants';
import { SettingsNamespace } from '@/stores/settings-store';
import { GoldenLayoutNamespace } from '@/stores/golden-layout-store';

const initLayout: LayoutConfig = {
  dimensions: { headerHeight: 36 },
  root: {
    type: 'row',
    content: [
      {
        type: 'column',
        content: [
          {
            title: GLComponentTitles[ComponentTechnicalName.INFRASTRUCTURE],
            type: 'component',
            componentType:
              GLComponentNames[ComponentTechnicalName.INFRASTRUCTURE]
          }
        ]
      }
    ]
  }
};

const GLayoutRoot = ref();

export default defineComponent({
  computed: {
    ...mapState(SettingsNamespace, ['theme'])
  },

  mounted() {
    this.loadSettings();
    this.loadInfrastructures();
    this.setGoldenLayoutRootComponent(GLayoutRoot.value);
    this.initGoldenLayout(initLayout);
  },

  methods: {
    ...mapActions(SidebarNamespace, {
      loadInfrastructures: 'initialLoad'
    }),
    ...mapActions(SettingsNamespace, ['loadSettings']),
    ...mapMutations(GoldenLayoutNamespace, {
      setGoldenLayoutRootComponent: 'setRootComponent'
    }),
    ...mapActions(GoldenLayoutNamespace, ['initGoldenLayout'])
  }
});
</script>

<style scoped>
.golden-layout-root {
  width: 100%;
  height: calc(100% - 90px);
}

.full-height {
  height: 100%;
}
</style>
