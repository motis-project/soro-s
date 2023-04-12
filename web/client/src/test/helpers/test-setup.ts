import ResizeObserver from 'resize-observer-polyfill';

// Allows vuetify to access the ResizeObserver API implemented in all modern browsers also in tests,
// where the environment is not a browser but jsdom.
window.ResizeObserver = ResizeObserver;
window.URL.createObjectURL = vi.fn();
