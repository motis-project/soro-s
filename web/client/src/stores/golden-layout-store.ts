import { Module } from 'vuex';
import { LayoutConfig } from 'golden-layout';
import { ComponentTechnicalName, GLComponentNames } from '@/golden-layout/golden-layout-constants';
import { GoldenLayoutAdapter } from '@/golden-layout/golden-layout-adapter';

type GoldenLayoutState = {
    rootComponent?: GoldenLayoutAdapter
    layout?: LayoutConfig,
}

export const GoldenLayoutNamespace = 'goldenLayout';

export const GoldenLayoutStore: Module<GoldenLayoutState, unknown> = {
    namespaced: true,

    state() {
        return {
            rootComponent: undefined,
            layout: undefined,
        };
    },

    mutations: {
        setRootComponent(state, rootComponent) {
            state.rootComponent = rootComponent;
        },
    },

    actions: {
        async loadSettings({ state }) {
            const storage = window.localStorage;
            const layout = storage.getItem('goldenLayout.layout');
            if (layout) {
                await state.rootComponent?.loadGLLayout(JSON.parse(layout));
            }
        },

        saveSettings({ state }) {
            const layout = state.rootComponent?.getLayoutConfig();
            if (layout) {
                window.localStorage.setItem('goldenLayout.layout', JSON.stringify(layout));
            }
        },

        async initGoldenLayout({ state, dispatch }, goldenLayoutLayout) {
            await state.rootComponent?.loadGLLayout(goldenLayoutLayout);
            await dispatch('loadSettings');
        },

        async addGoldenLayoutTab({ state }, { componentTechnicalName, title }: { componentTechnicalName: ComponentTechnicalName, title: string}) {
            await state.rootComponent?.addGLComponent(GLComponentNames[componentTechnicalName], title);
        },
    },
};