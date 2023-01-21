import { defineConfig } from 'vite';
import vue from '@vitejs/plugin-vue';
import vuetify from 'vite-plugin-vuetify';
import tsconfigPaths from 'vite-tsconfig-paths';

const relayServer = 'http://0.0.0.0:8080';

export default defineConfig({
    plugins: [
        vue(),
        vuetify({ autoImport: true }),
        tsconfigPaths(),
    ],
    server: {
        proxy: {
            '/api': {
                target: relayServer,
                changeOrigin: true,
                rewrite: (path) => path.replace(/^\/api/, ''),
            }
        },
    },
});
