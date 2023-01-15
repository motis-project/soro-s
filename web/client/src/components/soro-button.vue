<template>
    <button
        class="matter-button-contained soro-button"
        @click="(event: Event) => $emit('click', event)"
    >
        {{ label }}
    </button>
</template>

<script lang="ts">
import { defineComponent, PropType } from 'vue';

export default defineComponent({
    name: 'SoroButton',

    props: {
        label: {
            type: String as PropType<string>,
            required: true,
        },
    },

    emits: ['click'],
});
</script>

<style scoped>
.soro-button {
    margin-top: 0.2em;
    margin-bottom: 0.2em;
}
</style>