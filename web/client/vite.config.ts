import { defineConfig } from 'vite';
import vue from '@vitejs/plugin-vue';
import vuetify from 'vite-plugin-vuetify';
import tsconfigPaths from 'vite-tsconfig-paths';

export default defineConfig({
  plugins: [vue(), vuetify({ autoImport: true }), tsconfigPaths()],
  server: {
    proxy: {
      // everything except /assets, /node_modules and /src from localhost
      '^(?!/(assets|node_modules|src))/[0-9a-zA-Z/]+': {
        target: 'http://localhost:8080',
        changeOrigin: true
      }
    }
  }
});
