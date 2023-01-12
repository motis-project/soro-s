<template>
    <div class="soro-n-way-switch">
        <span class="n-way-switch-label">{{ label }}</span>
        <div class="n-way-switch-buttons">
            <soro-button
                v-for="option in options"
                :key="option.value"
                :label="option.text"
                :type="value === option.value ? 'primary' : 'secondary'"
                @click="emitChange(option.value)"
            />
        </div>
    </div>
</template>

<script setup lang="ts">
import SoroButton from '@/components/soro-button.vue';
</script>

<script lang="ts">
import { defineComponent, PropType } from 'vue';

export type NWaySwitchOption = {
    value: string,
    text: string,
};

export default defineComponent({
    name: 'SoroNWaySwitch',

    props: {
        value: {
            type: String,
            required: false,
            default: null,
        },
        options: {
            type: Array as PropType<NWaySwitchOption[]>,
            required: false,
            default: () => [],
        },
        label: {
            type: String,
            required: true,
        },
    },

    emits: ['select'],

    methods: {
        emitChange(selectedValue: string) {
            this.$emit('select', selectedValue);
        },
    },
});
</script>

<style scoped>
.soro-n-way-switch {
    width: 100%;
}

.n-way-switch-label {
    display: block;
    color: var(--text-color);
    font-family: var(--matter-font-family, "Roboto", "Segoe UI", BlinkMacSystemFont, system-ui, -apple-system);
    margin-bottom: 10px;
}

.n-way-switch-buttons {
    display: flex;
}

.n-way-switch-buttons .soro-button {
    width: 100%;
}

.n-way-switch-buttons .soro-button:not(:first-child) {
    margin-left: 0.5em;
}
</style>
