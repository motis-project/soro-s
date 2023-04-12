import { flushPromises, MountingOptions, mount } from '@vue/test-utils';
import { vuetify } from '@/vuetify';
import { Store } from 'vuex';

import { Configuration, StoreConfiguration } from '@/test/helpers/index';
import { createGlobalStore } from '@/stores/stores';

export async function mountWithDefaults(
  vueComponent: any,
  configuration: Configuration = {}
) {
  vueComponent.mixins = configuration.mixins || vueComponent.mixins;

  let mountConfiguration: MountingOptions<any> & Record<string, any> = {
    global: {
      ...(configuration.global || {}),
      plugins: [vuetify, ...(configuration.global?.plugins || [])],
      stubs: configuration.stubs || {},
      provide: configuration.injections || {}
    },
    data:
      configuration.data ||
      function () {
        return {};
      },
    propsData: configuration.props || {},
    filters: configuration.filters || {},
    mocks: configuration.mocks || {}
  };
  mountConfiguration = addStore(mountConfiguration, configuration);

  const wrapper = mount(vueComponent, mountConfiguration);

  await flushPromises();

  return wrapper;
}

const addStore = (
  mountConfiguration: MountingOptions<any> & Record<string, any>,
  configuration: Configuration
) => {
  const store = createModifiedStore(configuration.store ?? {});

  return {
    ...mountConfiguration,
    global: {
      ...mountConfiguration.global,
      plugins: [...(mountConfiguration.global?.plugins || []), store]
    }
  };
};

export function createModifiedStore(
  configuration: StoreConfiguration
): Store<any> {
  const store = createGlobalStore();
  Object.keys(configuration).forEach((storeName) => {
    store.unregisterModule(storeName);
    store.registerModule(storeName, {
      namespaced: true,
      getters: {},
      actions: {},
      mutations: {},
      state: {},
      ...configuration[storeName]
    });
  });

  return store;
}
