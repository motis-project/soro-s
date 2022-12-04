<template>
	<div>
		<infrastructure-map />
	</div>
</template>

<script>
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

	methods: {
		configureContainer() {
			if (!this.container)
				return;

			// TODO find solution for this
			this.container.on('resize', () => this.libreGLMap?.resize());
		}
	}
});
</script>
