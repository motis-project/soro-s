import { flushPromises, MountingOptions, mount } from '@vue/test-utils';
import { Module } from 'vuex';
import { Configuration } from './index';
import { vuetify } from '@/vuetify';
import { createGlobalStore } from '@/stores/stores';

export async function mountWithDefaults(vueComponent: any, configuration: Configuration = {}) {
    vueComponent.mixins = configuration.mixins || vueComponent.mixins;

    let mountConfiguration: MountingOptions<any> & Record<string, any> = {
        global: {
            ...configuration.global || {},
            plugins: [
                vuetify,
                ...(configuration.global?.plugins || []),
            ],
            stubs: configuration.stubs || {},
        },
        data: configuration.data || function () {
            return {}; 
        },
        propsData: configuration.props || {},
        filters: configuration.filters || {},
        mocks: configuration.mocks || {},
        provide: configuration.injections || {},
    };
    mountConfiguration = addStore(mountConfiguration, configuration);

    const wrapper = mount(vueComponent, mountConfiguration);

    await flushPromises();

    return wrapper;
}

const addStore = (mountConfiguration: MountingOptions<any> & Record<string, any>, configuration: Configuration) => {
    const storesWithDefaults: { [moduleName: string]: Module<any, any> } = {};

    const store = createGlobalStore();
    Object.keys(configuration.store ?? {}).forEach((storeName) => {
        store.unregisterModule(storeName);
        store.registerModule(storeName, {
            namespaced: true,
            getters: {},
            actions: {},
            mutations: {},
            state: {},
            ...configuration.store[storeName],
        });

        storesWithDefaults[storeName] = {
            namespaced: true,
            getters: {},
            actions: {},
            mutations: {},
            state: {},
            ...configuration.store[storeName],
        };
    });
    mountConfiguration.global?.plugins?.push(store);

    return mountConfiguration;
};