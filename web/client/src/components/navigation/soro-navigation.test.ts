import { mountWithDefaults } from '@/test-utils/mount-with-defaults';
import SoroNavigation from './soro-navigation.vue';
import { defineComponent } from 'vue';
import { VueWrapper } from '@vue/test-utils';
import { VLayout } from 'vuetify/components';

// As the component defines a v-navigation-drawer which requires a layout to be injected, we need to wrap it in
// a v-layout here to ensure it can mount and render.
const testableComponent = defineComponent({
    name: 'SoroNavigationTest',
    components: {
        SoroNavigation,
        VLayout,
    },
    template: '<v-layout><soro-navigation /></v-layout>',
});

describe('soro-navigation', async () => {
    let soroNavigationTestWrapper: VueWrapper<any>;
    let soroNavigation: VueWrapper<any>;

    beforeEach(async () => {
        soroNavigationTestWrapper = await mountWithDefaults(testableComponent);
        soroNavigation = soroNavigationTestWrapper.findComponent({ name: 'soro-navigation' });
    });

    it('displays an overlay button for each of the overlay types', async () => {
        const overlayTabs = soroNavigation.find('.overlay-tabs');

        const tabButtons = overlayTabs.findAllComponents({ name: 'v-btn' });

        expect(tabButtons.length).toBeGreaterThanOrEqual(2);
        expect(tabButtons[0].find('i').text()).toBe('menu');
        expect(tabButtons[1].find('i').text()).toBe('search');
    });

    it('enables the navigation drawer when it is disabled and the overlay button of the selected overlay is clicked', async () => {
        const overlayTabs = soroNavigation.find('.overlay-tabs');
        soroNavigation.vm.showOverlay = false;

        const tabButtons = overlayTabs.findAllComponents({ name: 'v-btn' });
        await tabButtons[0].find('button').trigger('click');
        expect(soroNavigation.vm.showOverlay).toBe(true);
    });

    it('disables the navigation drawer when it is enabled and the overlay button of the selected overlay is clicked', async () => {
        const overlayTabs = soroNavigation.find('.overlay-tabs');
        soroNavigation.vm.showOverlay = true;
        soroNavigation.vm.selectedOverlay = 'search';

        const tabButtons = overlayTabs.findAllComponents({ name: 'v-btn' });
        await tabButtons[1].find('button').trigger('click');
        expect(soroNavigation.vm.showOverlay).toBe(false);
    });

    it('enables the navigation drawer when it is disabled and the overlay button another than the selected ' +
        'overlay is clicked', async () => {
        const overlayTabs = soroNavigation.find('.overlay-tabs');
        soroNavigation.vm.showOverlay = false;
        soroNavigation.vm.selectedOverlay = 'menu';

        const tabButtons = overlayTabs.findAllComponents({ name: 'v-btn' });
        await tabButtons[1].find('button').trigger('click');
        expect(soroNavigation.vm.showOverlay).toBe(true);
    });

    it('does not disable the navigation drawer when it is enabled and the overlay button another than the selected ' +
        'overlay is clicked', async () => {
        const overlayTabs = soroNavigation.find('.overlay-tabs');
        soroNavigation.vm.showOverlay = true;
        soroNavigation.vm.selectedOverlay = 'menu';

        const tabButtons = overlayTabs.findAllComponents({ name: 'v-btn' });
        await tabButtons[1].find('button').trigger('click');
        expect(soroNavigation.vm.showOverlay).toBe(true);
    });

    it('displays only the menu content when the \'menu\' overlay button is clicked', async () => {
        const overlayContainer = soroNavigation.find('.overlay-container');
        const overlayTabs = soroNavigation.find('.overlay-tabs');

        const tabButtons = overlayTabs.findAllComponents({ name: 'v-btn' });
        await tabButtons[0].find('button').trigger('click');

        expect(overlayContainer.findComponent({ name: 'soro-navigation-menu-content' }).exists()).toBe(true);
        expect(overlayContainer.findComponent({ name: 'soro-navigation-search-content' }).exists()).toBe(false);
    });

    it('displays only the search content when the selected overlay is \'search\'', async () => {
        const overlayContainer = soroNavigation.find('.overlay-container');
        const overlayTabs = soroNavigation.find('.overlay-tabs');

        const tabButtons = overlayTabs.findAllComponents({ name: 'v-btn' });
        await tabButtons[1].find('button').trigger('click');

        expect(overlayContainer.findComponent({ name: 'soro-navigation-menu-content' }).exists()).toBe(false);
        expect(overlayContainer.findComponent({ name: 'soro-navigation-search-content' }).exists()).toBe(true);
    });
});
