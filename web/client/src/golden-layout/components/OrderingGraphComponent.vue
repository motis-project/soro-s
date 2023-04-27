<template>
  <div>
    <ordering-graph ref="orderingGraph" />
  </div>
</template>

<script setup lang="ts">
import OrderingGraph from '@/components/ordering/ordering-graph.vue';
</script>

<script lang="ts">
import { defineComponent } from 'vue';

export default defineComponent({
  name: 'OrderingGraphComponent',

  provide() {
    return {
      goldenLayoutKeyInjection: this.goldenLayoutKey
    };
  },

  props: {
    goldenLayoutKey: {
      type: String,
      required: true
    }
  }
});
</script>
