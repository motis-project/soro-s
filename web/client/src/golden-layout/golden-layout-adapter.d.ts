import { LayoutConfig, ResolvedLayoutConfig } from 'golden-layout';
import { ComponentTechnicalName } from '@/golden-layout/golden-layout-constants';

export type GoldenLayoutAdapter = {
    addGLComponent: (componentType: ComponentTechnicalName, title?: string) => Promise<void>,
    loadGLLayout: (layout: LayoutConfig | ResolvedLayoutConfig) => void,
    getLayoutConfig: () => ResolvedLayoutConfig,
}

declare module 'golden-layout-adapter.vue' {
    const adapter: GoldenLayoutAdapter;
    export default adapter;
}
