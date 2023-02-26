<template>
    <div>
        <infrastructure-map ref="infrastructureMap" />
    </div>
</template>

<script setup lang="ts">
import InfrastructureMap from '@/components/infrastructure/infrastructure-map.vue';
</script>

<script lang="ts">
import { defineComponent } from 'vue';

export default defineComponent({
    name: 'InfrastructureComponent',

    provide() {
        return {
            goldenLayoutKeyInjection: this.goldenLayoutKey,
        };
    },

    props: {
        goldenLayoutKey: {
            type: String,
            required: true,
        },
    },
});
</script>
