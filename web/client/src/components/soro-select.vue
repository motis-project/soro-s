<template>
  <div class="material-select data-select">
    <select
        class="material-select-text"
        @change="emitChange"
    >
      <option
        v-for="option in extendedOptions"
        :key="option"
        :value="option"
        :selected="option === value"
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
	name: 'soro-select',

	props: {
		value: {
			type: String,
			required: false,
			default: null,
		},
		options: {
			type: Array,
			required: false,
			default: () => [],
		},
		label: {
			type: String,
			required: true,
		},
	},

	data() {
		return { extendedOptions: [] };
	},

	watch: {
		options() {
			this.extendedOptions = this.options.filter((option) => option !== '.' && option !== '..');
			this.extendedOptions.push(null);
		},
	},

	methods: {
		emitChange(event) {
			event.preventDefault();
			this.$emit('select', event.target.value);
		},
	},
};
</script>