<template>
  <VueDatePicker
    v-model="dateTimeRange"
    range
    text-input
    enable-seconds
    @update:model-value="emitChange"
  >
  </VueDatePicker>
</template>

<script setup lang="ts">
import VueDatePicker from '@vuepic/vue-datepicker';
import '@vuepic/vue-datepicker/dist/main.css';
</script>

<script lang="ts">
import { defineComponent, PropType } from 'vue';

function dateToUnix(date: Date): number {
  return Math.floor(date.getTime() / 1000);
}

function unixToDate(unix: number): Date {
  return new Date(unix * 1000);
}

export default defineComponent({
  name: 'SoroDateTimeRange',

  props: {
    watchValue: {
      type: Array as PropType<number[]>,
      required: false,
      default: () => []
    }
  },

  emits: ['change'],

  data(): {
    dateTimeRange?: [Date, Date];
  } {
    return {
      dateTimeRange: undefined
    };
  },

  watch: {
    watchValue(newRange: [number, number]) {
      this.dateTimeRange = [unixToDate(newRange[0]), unixToDate(newRange[1])];
    }
  },

  methods: {
    emitChange() {
      if (!this.dateTimeRange || this.dateTimeRange.length !== 2) {
        return;
      }

      this.$emit('change', [
        dateToUnix(this.dateTimeRange[0]),
        dateToUnix(this.dateTimeRange[1])
      ]);
    }
  }
});
</script>
