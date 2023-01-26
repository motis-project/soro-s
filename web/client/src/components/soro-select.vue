<template>
    <v-select
        v-bind="$attrs"
        v-model="currentValue"
        :label="label"
        :items="options"
        @update:model-value="emitChange"
    >
        <template #prepend-item>
            <v-list-item
                title="Clear selection"
                @click="() => { currentValue = null; emitChange(); }"
            />
            <v-divider class="mt-2" />
        </template>
    </v-select>
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

    data(): { currentValue: string | null } {
        return {
            currentValue: null,
        };
    },

    watch: {
        value(newValue) {
            this.currentValue = newValue; 
        },
    },

    methods: {
        emitChange() {
            this.$emit('select', this.currentValue);
        },
    },
});
</script>