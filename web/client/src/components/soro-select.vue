<template>
    <div class="material-select">
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
        <span class="material-select-highlight" />
        <span class="material-select-bar" />
        <label class="material-select-label">{{ label }}</label>
    </div>
</template>

<script lang="ts">
import { defineComponent, PropType } from 'vue';

export default defineComponent({
    name: 'SoroSelect',

    props: {
        value: {
            type: String,
            required: false,
            default: null,
        },
        options: {
            type: Array as PropType<string[]>,
            required: false,
            default: () => [],
        },
        label: {
            type: String,
            required: true,
        },
    },

    emits: ['select'],

    data() {
        return {
            extendedOptions: new Array<string>(),
        };
    },

    watch: {
        options() {
            this.extendedOptions.push('');
            this.extendedOptions.push(...this.options as string[]);
        },
    },

    methods: {
        emitChange(event: Event) {
            event.preventDefault();
            this.$emit('select', (event.target as HTMLInputElement).value);
        },
    },
});
</script>