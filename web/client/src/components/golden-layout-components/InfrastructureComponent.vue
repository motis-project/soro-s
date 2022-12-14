<template>
	<div>
		<infrastructure-map ref="infrastructureMap" />
	</div>
</template>

<script lang="ts">
import { defineComponent } from 'vue';
import { ComponentContainer } from 'golden-layout';
import InfrastructureMap from '../infrastructure/infrastructure-map.vue';

export default defineComponent({
	name: 'InfrastructureComponent',
	components: { InfrastructureMap },
	props: {
		container: {
			type: ComponentContainer,
			required: false,
			default: undefined,
		},
	},

	created() { this.configureContainer(); },

	methods: {
		configureContainer() {
			if (!this.container)
				return;

			// TODO make this less hacky
			this.container.on('resize', () => (this.$refs.infrastructureMap as { resize: () => void }).resize());
		}
	}
});
</script>
