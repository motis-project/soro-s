import { defineConfig } from 'vite';
import vue from '@vitejs/plugin-vue';
import tsconfigPaths from 'vite-tsconfig-paths';

const relayServer = 'http://0.0.0.0:8080';
const relayLocations = [
	'/infrastructure',
	'/timetable',
	'/font',
	'/icons',
	'/interlocking_route_simple',
	'/small',
];

const proxy = {};
relayLocations.forEach((location) => proxy[location] = {
	target: relayServer,
	changeOrigin: true,
});
export default defineConfig({
	plugins: [vue(), tsconfigPaths()],
	server: { proxy },
});
