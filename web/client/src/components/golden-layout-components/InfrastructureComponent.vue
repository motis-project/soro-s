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
import { ComponentContainer } from 'golden-layout';

export default defineComponent({
    name: 'InfrastructureComponent',

    props: {
        container: {
            type: ComponentContainer,
            required: false,
            default: undefined,
        },
    },

    created() {
        this.configureContainer(); 
    },

    methods: {
        configureContainer() {
            if (!this.container) {
                return;
            }

            this.container.on('resize', () => (this.$refs.infrastructureMap as { resize: () => void }).resize());
        },
    },
});
</script>
