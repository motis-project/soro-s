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
        // Enable coverage using the test:coverage script
        coverage: {
            provider: 'c8',
            all: true,
            include: [
                'src/api/**',
                'src/components/**',
                'src/golden-layout/**',
                'src/stores/**',
            ],
            // These files are not used as of right now and not ensured to work properly, so they do not have to partake
            // in coverage measurement.
            exclude: [
                'src/stores/timetable-store.ts',
                'src/components/disruption-detail.vue',
                'src/components/station-detail.vue',
                'src/components/simulation/**',
                'src/golden-layout/components/OrderingGraphComponent.vue',
                'src/golden-layout/components/SimulationComponent.vue',
                'src/golden-layout/components/TimetableComponent.vue',
                // defaults
                '**/*.d.ts',
                '**/*.test.ts',
            ],
        },
    },
}));
