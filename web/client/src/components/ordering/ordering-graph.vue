<template>
  <div ref="container">
    <div ref="graph" class="sigma-container" />
  </div>
</template>

<script lang="ts">
import { defineComponent } from 'vue';
import { mapActions, mapState } from 'vuex';

import { Sigma } from 'sigma';
import { DirectedGraph } from 'graphology';
import noverlap from 'graphology-layout-noverlap';

import { SidebarNamespace, UnixRange } from '@/stores/sidebar-store';

import { nodeColor, rendererSettings } from '@/components/ordering/settings';
import { Attributes } from 'graphology-types';

class timer {
  constructor() {
    this.start = Date.now();
  }

  elapsed(): number {
    return Date.now() - this.start;
  }

  printElapsed(message: string): void {
    console.log('[' + message + ']:', this.elapsed());
  }

  start: number;
}

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
      'currentDateRange',
      'trainIdsFilter'
    ])
  },

  watch: {
    currentDateRange(newDateRange: UnixRange) {
      this.createOrderingGraph(
        this.currentInfrastructure,
        this.currentTimetable,
        newDateRange,
        this.trainIdsFilter
      );
    },

    trainIdsFilter(newTrainIds: number[]) {
      this.createOrderingGraph(
        this.currentInfrastructure,
        this.currentTimetable,
        this.currentDateRange,
        newTrainIds
      );
    }
  },

  mounted() {
    this.createOrderingGraph(
      this.currentInfrastructure,
      this.currentTimetable,
      this.currentDateRange,
      this.trainIdsFilter
    );
  },

  methods: {
    ...mapActions(SidebarNamespace, [
      'addHighlightedInterlockingRoute',
      'deleteHighlightedInterlockingRoute'
    ]),

    async createOrderingGraph(
      infrastructureName: string,
      timetableName: string,
      dateRange: UnixRange,
      trainIds: number[]
    ) {
      if (!infrastructureName || !timetableName || !dateRange) {
        return;
      }

      this.graph.clear();
      this.graph = new DirectedGraph();

      const tim: timer = new timer();
      const og = await this.$store.state.soroClient
        .infrastructure(infrastructureName)
        .timetable(timetableName)
        .ordering(dateRange[0], dateRange[1], trainIds);

      tim.printElapsed('fetched ordering graph');

      this.graph.import(og);

      tim.printElapsed('imported ordering graph');

      const totalTrains = this.graph.getAttribute('totalTrains');
      const maxTrainLength = this.graph.getAttribute('maxTrainLength');

      const xSpacing = 500;
      const ySpacing = 500;

      this.graph.forEachNode((node: string, attr: Attributes) => {
        attr['x'] = attr['offset'] * 1000 * totalTrains;
        attr['y'] = attr['relativeTrainId'] * -1000 * totalTrains;

        attr['color'] = nodeColor;
        attr['type'] = 'fast';
        attr['size'] = 2;
        attr['label'] = 'Train: ' + attr['train'] + '\nRoute: ' + attr['route'];
      });

      this.graph.forEachEdge((edge: string, attr: Attributes) => {
        attr['type'] = 'arrow';
        attr['size'] = 1;
      });

      tim.printElapsed('set node attributes');

      this.sigma = new Sigma(
        this.graph,
        this.$refs.graph as HTMLElement,
        rendererSettings
      );

      tim.printElapsed('created sigma');

      this.sigma.on('clickNode', ({ node }) => {
        this.addHighlightedInterlockingRoute(
          this.graph.getNodeAttributes(node)['route']
        );
      });

      this.sigma.on('rightClickNode', ({ node }) => {
        this.deleteHighlightedInterlockingRoute(
          this.graph.getNodeAttributes(node)['route']
        );
      });

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
