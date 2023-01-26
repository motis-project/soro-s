<template>
    <v-btn
        class="soro-button"
        :color="type"
        @click="(event: Event) => $emit('click', event)"
    >
        {{ label }}
    </v-btn>
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
        type: {
            type: String as PropType<'primary' | 'secondary'>,
            required: false,
            default: 'primary',
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