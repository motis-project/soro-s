import { defineConfig } from 'vite'
import vue from '@vitejs/plugin-vue'

export default defineConfig({
    plugins: [vue()],
    server: {
        proxy: {
            '/infrastructure': {
                target: 'http://0.0.0.0:8080',
                changeOrigin: true,
            },
            '/timetable': {
                target: 'http://0.0.0.0:8080',
                changeOrigin: true,
            },
        },
    },
})
