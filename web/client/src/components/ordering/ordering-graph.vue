<template>
  <div ref="container">
    <div ref="graph" class="sigma-container" />
  </div>
</template>

<script lang="ts">
import { defineComponent } from 'vue';
import { mapState } from 'vuex';

import { Sigma } from 'sigma';
import { DirectedGraph } from 'graphology';

import { SidebarNamespace, UnixRange } from '@/stores/sidebar-store';

import { nodeColor, rendererSettings } from '@/components/ordering/settings';
import { Attributes } from 'graphology-types';

export default defineComponent({
  name: 'OrderingGraph',

  inject: {
    goldenLayoutKeyInjection: {
      default: ''
    }
  },

  data(): {
    graph: DirectedGraph;
    sigma?: Sigma<DirectedGraph>;
  } {
    return {
      graph: new DirectedGraph(),
      sigma: undefined
    };
  },

  computed: {
    ...mapState(SidebarNamespace, [
      'currentInfrastructure',
      'currentTimetable',
      'currentDateRange'
    ])
  },

  watch: {
    currentDateRange(newDateRange: UnixRange) {
      this.createOrderingGraph(
        this.currentInfrastructure,
        this.currentTimetable,
        newDateRange
      );
    }
  },

  mounted() {
    this.createOrderingGraph(
      this.currentInfrastructure,
      this.currentTimetable,
      this.currentDateRange
    );
  },

  methods: {
    async createOrderingGraph(
      infrastructureName: string,
      timetableName: string,
      dateRange: UnixRange
    ) {
      if (!infrastructureName || !timetableName || !dateRange) {
        return;
      }

      this.graph.clear();
      this.graph = new DirectedGraph();

      const og = await this.$store.state.soroClient
        .infrastructure(infrastructureName)
        .timetable(timetableName)
        .ordering(dateRange[0], dateRange[1]);

      this.graph.import(og);

      this.graph.forEachNode((node: string, attr: Attributes) => {
        attr['x'] = attr['offset'];
        attr['y'] = attr['train'] * -10;
        attr['color'] = nodeColor;
        attr['type'] = 'base';
        attr['size'] = 10;
        attr['label'] = 'Train: ' + attr['train'] + '\nRoute: ' + attr['route'];
      });

      this.graph.forEachEdge((edge: string, attr: Attributes) => {
        attr['type'] = 'arrow';
        attr['size'] = 5;
      });

      this.sigma = new Sigma(
        this.graph,
        this.$refs.graph as HTMLElement,
        rendererSettings
      );
      this.sigma.refresh();
    }
  }
});
</script>

<style>
.sigma-container {
  padding: 0;
  margin: 0;
  position: absolute;
  height: 100%;
  width: 100%;
}
</style>
