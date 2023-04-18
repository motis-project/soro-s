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
    currentInfrastructure(newInfrastructure: string) {
      this.createOrderingGraph(
        newInfrastructure,
        this.currentTimetable,
        this.currentDateRange
      );
    },

    currentTimetable(newTimetable: string) {
      this.createOrderingGraph(
        this.currentInfrastructure,
        newTimetable,
        this.currentDateRange
      );
    },

    currentDateRange(newDateRange: UnixRange) {
      this.createOrderingGraph(
        this.currentInfrastructure,
        this.currentTimetable,
        newDateRange
      );
    }
  },

  methods: {
    createOrderingGraph(
      infrastructureName: string,
      timetableName: string,
      dateRange: UnixRange
    ) {
      if (!infrastructureName || !timetableName || !dateRange) {
        console.log('destroy sigma');
        return;
      }

      console.log('creating graph');

      const g = {
        a: {},
        // id, t
        nodes: [
          { id: 'a', attr: { train: 0, route: 0, offset: 0 } },
          { id: 'b', attr: { train: 0, route: 1, offset: 1 } },
          { id: 'c', attr: { train: 0, route: 2, offset: 2 } },
          { id: 'd', attr: { train: 0, route: 3, offset: 3 } }
        ],
        edges: [
          ['a', 'b'],
          ['b', 'c'],
          ['c', 'd'],
          ['d', 'a']
        ]
      };

      for (const node of g.nodes) {
        this.graph.addNode(node.id, node.attr);
      }

      for (const edge of g.edges) {
        this.graph.addEdge(edge[0], edge[1]);
      }

      this.graph.forEachNode((node: string, attr: Attributes) => {
        attr['x'] = attr['offset'];
        attr['y'] = attr['train'];
        attr['color'] = nodeColor;
        attr['type'] = 'base';
        attr['size'] = 2;
        attr['label'] = 'Node: ' + node;
      });

      const exported = this.graph.export();
      console.log('exported', exported);

      this.sigma = new Sigma(
        this.graph,
        this.$refs.graph as HTMLElement,
        rendererSettings
      );
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

.menu {
  position: absolute;
  right: 1em;
  top: 1em;
}

.search {
  display: none;
  position: absolute;
  margin: 5px;
  right: 1em;
  top: 2em;
  width: auto;
  height: auto;
  box-sizing: border-box;
  border: 2px solid #e0e0e0;
  background-color: #f5f5f5;
  padding: 8px;
  font-family: 'Roboto', 'Helvetica', 'Arial', sans-serif;
  font-weight: inherit;
}

.row {
  display: flex;
}

.warning {
  display: none;
}

.message {
  color: rgb(167, 11, 11);
}

.pad {
  margin-top: 10px;
}

.text-input {
  width: 100%;
  padding: 4px 4px;
  margin: 8px 0;
  box-sizing: border-box;
  border: 2px solid #e0e0e0;
}

.has-error {
  border-color: rgb(167, 11, 11);
  outline: none;
}

.neighbor-input,
.Scaling-input {
  width: 100%;
  padding: 4px 4px;
  margin: 8px 0;
}

.neighbor-input {
  width: 90%;
}

.neighbor-value {
  width: 10%;
  padding: 4px 4px;
  margin: 8px 0;
}

.label {
  color: #747474;
}
</style>
