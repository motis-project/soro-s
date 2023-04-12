<template>
  <v-switch
    v-model="isChecked"
    :label="label"
    :color="color"
    @change="onChange"
  ></v-switch>
</template>

<script lang="ts">
import { defineComponent, PropType } from 'vue';

export default defineComponent({
  name: 'SoroSwitch',

  props: {
    internalValue: {
      type: Number,
      required: true
    },
    label: {
      type: String,
      required: true
    },
    onChecked: {
      type: Function,
      required: true
    },
    onUnchecked: {
      type: Function,
      required: true
    },
    initiallyChecked: {
      type: Boolean,
      required: false,
      default: false
    },
    color: {
      type: String as PropType<'primary' | 'secondary'>,
      required: false,
      default: 'primary'
    }
  },

  data(): { isChecked: boolean } {
    return {
      isChecked: this.initiallyChecked
    };
  },

  methods: {
    onChange() {
      if (this.isChecked) {
        this.onChecked(this.internalValue);
      } else {
        this.onUnchecked(this.internalValue);
      }
    }
  }
});
</script>
