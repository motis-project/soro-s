import { defineComponent } from 'vue';
import { VueWrapper } from '@vue/test-utils';
import { VLayout } from 'vuetify/components';

import SoroSidebar from '@/components/sidebar/sidebar.vue';

import { mountWithDefaults } from '@/test/helpers/mount-with-defaults';

// As the component defines a v-navigation-drawer which requires a layout to be injected, we need to wrap it in
// a v-layout here to ensure it can mount and render.
const testableComponent = defineComponent({
  name: 'SoroSidebarTest',
  components: {
    SoroSidebar,
    VLayout
  },
  template: '<v-layout><soro-sidebar /></v-layout>'
});

describe('soro-sidebar', async () => {
  let soroNavigationTestWrapper: VueWrapper<any>;
  let soroNavigation: VueWrapper<any>;

  beforeEach(async () => {
    soroNavigationTestWrapper = await mountWithDefaults(testableComponent);
    soroNavigation = soroNavigationTestWrapper.findComponent({
      name: 'soro-sidebar'
    });
  });

  it('displays an overlay button for each of the overlay types', async () => {
    const overlayTabs = soroNavigation.find('.overlay-tabs');

    const tabButtons = overlayTabs.findAllComponents({ name: 'v-btn' });

    expect(tabButtons.length).toBeGreaterThanOrEqual(2);
    expect(tabButtons[0].find('i').text()).toBe('menu');
    expect(tabButtons[1].find('i').text()).toBe('search');
  });

  it('enables the sidebar when it is disabled and the overlay button of the selected overlay is clicked', async () => {
    const overlayTabs = soroNavigation.find('.overlay-tabs');
    soroNavigation.vm.setShowOverlay(false);

    const tabButtons = overlayTabs.findAllComponents({ name: 'v-btn' });
    await tabButtons[0].find('button').trigger('click');
    expect(soroNavigation.vm.showOverlay).toBe(true);
  });

  it('disables the sidebar when it is enabled and the overlay button of the selected overlay is clicked', async () => {
    const overlayTabs = soroNavigation.find('.overlay-tabs');
    soroNavigation.vm.setShowOverlay(true);
    soroNavigation.vm.setSelectedOverlay('search');

    const tabButtons = overlayTabs.findAllComponents({ name: 'v-btn' });
    await tabButtons[1].find('button').trigger('click');
    expect(soroNavigation.vm.showOverlay).toBe(false);
  });

  it(
    'enables the sidebar when it is disabled and the overlay button another than the selected ' +
      'overlay is clicked',
    async () => {
      const overlayTabs = soroNavigation.find('.overlay-tabs');
      soroNavigation.vm.setShowOverlay(false);
      soroNavigation.vm.setSelectedOverlay('menu');

      const tabButtons = overlayTabs.findAllComponents({ name: 'v-btn' });
      await tabButtons[1].find('button').trigger('click');
      expect(soroNavigation.vm.showOverlay).toBe(true);
    }
  );

  it(
    'does not disable the sidebar when it is enabled and the overlay button another than the selected ' +
      'overlay is clicked',
    async () => {
      const overlayTabs = soroNavigation.find('.overlay-tabs');
      soroNavigation.vm.setShowOverlay(true);
      soroNavigation.vm.setSelectedOverlay('menu');

      const tabButtons = overlayTabs.findAllComponents({ name: 'v-btn' });
      await tabButtons[1].find('button').trigger('click');
      expect(soroNavigation.vm.showOverlay).toBe(true);
    }
  );

  it("displays only the search content when the selected overlay is 'search'", async () => {
    const overlayContainer = soroNavigation.find('.overlay-container');
    const overlayTabs = soroNavigation.find('.overlay-tabs');

    const tabButtons = overlayTabs.findAllComponents({ name: 'v-btn' });
    await tabButtons[1].find('button').trigger('click');

    expect(
      overlayContainer.findComponent({ name: 'sidebar-menu' }).exists()
    ).toBe(false);
    expect(
      overlayContainer.findComponent({ name: 'sidebar-search' }).exists()
    ).toBe(true);
  });
});
