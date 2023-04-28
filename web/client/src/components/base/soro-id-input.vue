<template>
  <v-text-field
    v-model="trainIds"
    :label="label"
    class="soro-id-input"
    :rules="validationRules()"
    @keydown.enter.prevent="emitEnter"
  >
  </v-text-field>
</template>

<script lang="ts">
import { defineComponent, PropType } from 'vue';

export default defineComponent({
  name: 'SoroIdInput',

  props: {
    label: {
      type: String as PropType<string>,
      required: true
    }
  },

  emits: ['enter'],

  data(): {
    trainIds: string;
  } {
    return {
      trainIds: ''
    };
  },

  methods: {
    validationRules() {
      return [
        (v: string) => /^\d*(,\d+)*$/.test(v) || 'only comma separated numbers'
      ];
    },

    validate(content: string): boolean {
      return this.validationRules().reduce((result, rule) => {
        return result && rule(content) === true;
      }, true);
    },

    emitEnter() {
      if (this.trainIds === '' || !this.validate(this.trainIds)) {
        return;
      }

      this.$emit(
        'enter',
        this.trainIds.split(',').map((id) => parseInt(id))
      );
    }
  }
});
</script>
