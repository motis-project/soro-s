<template>
  <div class="material-select data-select">
    <select
        class="material-select-text"
        @change="(event) => $emit('change', event)"
    >
      <option
        v-for="option in extendedOptions"
        :key="option"
        :value="option"
        :selected="option === selectedOption"
        :label="option ?? ''"
      >
        {{ option }}
      </option>
    </select>
    <span class="material-select-highlight"></span>
    <span class="material-select-bar"></span>
    <label class="material-select-label">{{ label }}</label>
  </div>
</template>

<script>
export default {
  name: "soro-select",

  props: {
    options: {
      type: Array,
      required: false,
      default: []
    },
    label: {
      type: String,
      required: true,
    },
  },

  data() {
    return {
      extendedOptions: [],
      selectedOption: null,
    }
  },

  watch: {
    options() {
      this.extendedOptions = this.options.filter((option) => option !== '.' && option !== '..');
      this.extendedOptions.push(null);
    }
  },
}
</script>