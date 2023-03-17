import { mountWithDefaults } from '@/test-utils/mount-with-defaults';
import SoroNavigationSearchContent from './soro-navigation-search-content.vue';

describe('soro-navigation-search-content', async () => {
    it('contains a station search with extended options', async () => {
        const soroNavigationSearchContent = await mountWithDefaults(SoroNavigationSearchContent);
        const stationSearch = soroNavigationSearchContent.findComponent({ name: 'station-search' });

        expect(stationSearch.exists()).toBe(true);
        expect(stationSearch.vm.$props).toStrictEqual(expect.objectContaining({
            showExtendedOptions: true,
        }));
    });
});
