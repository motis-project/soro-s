<template>
     <v-expansion-panels>
        <v-expansion-panel :title="label">
            <v-expansion-panel-text><slot /></v-expansion-panel-text>
        </v-expansion-panel>
    </v-expansion-panels>
</template>

<script lang="ts">
import { defineComponent } from 'vue';

export default defineComponent({
    name: 'SoroCollapsible',

    props: {
        label: {
            type: String,
            required: true
        },
    },
});
</script>
