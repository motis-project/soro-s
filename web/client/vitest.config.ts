import { defineConfig } from 'vitest/config';
import viteConfig from './vite.config';
import { mergeConfig } from 'vite';

export default mergeConfig(viteConfig, defineConfig({
    test: {
        watch: false,
        globals: true,
        // Multithreading must be disabled as we have to use the node-canvas addition for support of the <canvas> tag.
        // See https://github.com/vitest-dev/vitest/issues/740 for further information.
        threads: false,
        setupFiles: ['src/test-utils/test-setup.ts'],
        reporters: 'verbose',
        environment: 'jsdom',
        deps: {
            inline: ['vuetify'],
        },
    },
}));
