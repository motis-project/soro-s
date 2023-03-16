import { mountWithDefaults } from '@/test-utils/mount-with-defaults';
import StationSearch from './station-search.vue';
import { InfrastructureNamespace } from '@/stores/infrastructure-store';
import { ElementType } from '@/components/infrastructure/element-types';
import { VueWrapper } from '@vue/test-utils';
import { VList } from 'vuetify/components';

vi.mock('@/components/infrastructure/add-icons', () => ({
    iconUrl: 'some-icon-url/',
    iconExtension: '.something',
}));

describe('station-search', async () => {
    let stationSearch: VueWrapper<any>;
    const searchPositionFromName = vi.fn();
    const setCurrentSearchedMapPosition = vi.fn();
    const infrastructureState = {
        currentInfrastructure: 'some-infrastructure',
        currentSearchTerm: '',
        currentSearchError: '',
        currentSearchedMapPositions: [{
            name: 'Kassel',
            type: ElementType.STATION,
            position: {
                lat: 42,
                lon: 69,
            },
        }],
    };

    const defaults = {
        store: {
            [InfrastructureNamespace]: {
                state: infrastructureState,
                mutations: { setCurrentSearchedMapPosition },
                actions: { searchPositionFromName },
            },
        },
    };

    beforeEach(async () => {
        vi.clearAllMocks();
        stationSearch = await mountWithDefaults(StationSearch, defaults);
    });

    describe('when setting \'showExtendedLink\' to true', function () {
        beforeEach(async () => {
            stationSearch = await mountWithDefaults(StationSearch, {
                ...defaults,
                props: { showExtendedLink: true },
            });
        });

        it('shows an extended link', async () => {
            const showExtendedLink = stationSearch.find('.station-search-extended-link');
            expect(showExtendedLink.exists()).toBe(true);
        });

        it('emits \'change-to-extended\' when clicking the extended link', async () => {
            const showExtendedLink = stationSearch.find('.station-search-extended-link');

            await showExtendedLink.find('a').trigger('click');

            expect(stationSearch.emitted('change-to-extended')).toHaveLength(1);
        });
    });

    it('does not show an extended link when setting \'showExtendedLink\' to false', async () => {
        stationSearch = await mountWithDefaults(StationSearch, {
            ...defaults,
            props: { showExtendedLink: false },
        });

        const showExtendedLink = stationSearch.find('.station-search-extended-link');
        expect(showExtendedLink.exists()).toBe(false);
    });

    describe('when the search text field emits an event following a \'enter\' key press', async () => {
        it('does not search for a position from a name if the text field does not contain anything', async () => {
            const searchTextField = stationSearch.findComponent({ ref: 'searchTextField' });

            await searchTextField.trigger('keydown', { keyCode: 13 });

            expect(searchPositionFromName).not.toHaveBeenCalled();
        });

        it('searches for a position from a name if the text field contains a search string', async () => {
            const searchTextField = stationSearch.findComponent({ ref: 'searchTextField' });
            stationSearch.vm.currentSearchTypes.length = 0;
            stationSearch.vm.currentSearchTypes.push(
                ElementType.HALT,
                ElementType.MAIN_SIGNAL,
            );
            searchTextField.vm.$emit('update:modelValue', 'some-search-string');

            await searchTextField.find('input').trigger('keydown.enter');

            expect(searchPositionFromName).toHaveBeenCalledWith(
                expect.any(Object),
                {
                    query: 'some-search-string',
                    includedTypes: {
                        [ElementType.STATION]: false,
                        [ElementType.HALT]: true,
                        [ElementType.MAIN_SIGNAL]: true,
                    },
                },
            );
        });
    });

    describe('when the search button emits a \'click\' event', async () => {
        it('does not call \'searchPositionFromName\' if no query is entered', async () => {
            await stationSearch.setData({
                currentQuery: null,
            });

            const searchButton = stationSearch.findComponent({ name: 'soro-button' });
            searchButton.vm.$emit('click');

            expect(searchPositionFromName).not.toHaveBeenCalled();
        });

        it(
            'calls \'searchPositionFromName\' with the query and all selected search types if a query is entered',
            async () => {
                await stationSearch.setData({
                    currentQuery: 'some-query',
                    currentSearchTypes: [
                        'station',
                        'foo',
                    ],
                });

                const searchButton = stationSearch.findComponent({ name: 'soro-button' });
                searchButton.vm.$emit('click');

                expect(searchPositionFromName).toHaveBeenCalledWith(
                    expect.any(Object),
                    {
                        query: 'some-query',
                        includedTypes: {
                            station: true,
                            hlt: false,
                            ms: false,
                        },
                    },
                );
            },
        );
    });

    it('shows a checkbox for each of the valid search types when setting \'showExtendedOptions\' to true', async () => {
        stationSearch = await mountWithDefaults(StationSearch, {
            ...defaults,
            props: { showExtendedOptions: true },
        });

        const extendedOptionList = stationSearch.find('.station-search-extended-options');

        expect(extendedOptionList.exists()).toBe(true);
        const checkboxes = extendedOptionList.findAllComponents({ name: 'v-checkbox' });
        expect(checkboxes).toHaveLength(3);
    });

    it('does not show checkboxes when setting \'showExtendedOptions\' to false', async () => {
        stationSearch = await mountWithDefaults(StationSearch, {
            ...defaults,
            props: { showExtendedOptions: false },
        });

        const extendedOptionList = stationSearch.find('.station-search-extended-options');
        expect(extendedOptionList.exists()).toBe(false);
    });

    describe('when there is more than one searched map positions', async () => {
        beforeEach(async () => {
            infrastructureState.currentSearchedMapPositions.length = 0;
            infrastructureState.currentSearchedMapPositions.push(
                {
                    name: 'some-name',
                    type: 'something',
                    position: {
                        lat: 42,
                        lon: 69,
                    },
                },
                {
                    name: 'some-other-name',
                    type: 'something-other',
                    position: {
                        lat: 100,
                        lon: 200,
                    },
                },
            );
            await stationSearch.vm.$forceUpdate();
        });

        it('shows a list of items for each position', async () => {
            infrastructureState.currentSearchedMapPositions[0].name = 'Wiesbaden';
            infrastructureState.currentSearchedMapPositions[1].name = 'Kassel';
            const resultList = stationSearch.findComponent<VList>('.station-search-result-list');
            await resultList.vm.$forceUpdate();

            const subHeader = resultList.findComponent({ name: 'v-list-subheader' });
            expect(subHeader.text()).toBe('SEARCH RESULTS');
            const listItems = resultList.findAllComponents({ name: 'v-list-item' });
            expect(listItems).toHaveLength(2);
            expect(listItems[0].text()).toBe('Wiesbaden');
            expect(listItems[1].text()).toBe('Kassel');
        });

        it('sets the current searched map position when a list item emits a \'click\' event', async () => {
            infrastructureState.currentSearchedMapPositions[0].position = {
                lat: 10,
                lon: 20,
            };
            const resultList = stationSearch.findComponent<VList>('.station-search-result-list');
            await resultList.vm.$forceUpdate();
            vi.clearAllMocks();

            const listItems = resultList.findAllComponents({ name: 'v-list-item' });
            listItems[0].vm.$emit('click');

            expect(setCurrentSearchedMapPosition).toHaveBeenCalledWith(
                infrastructureState,
                {
                    lat: 10,
                    lon: 20,
                },
            );
        });

        it('shows an icon of the specified position type for each list item if it is a valid search type', async () => {
            infrastructureState.currentSearchedMapPositions[0].type = 'station';
            infrastructureState.currentSearchedMapPositions[1].type = 'not-valid';
            const resultList = stationSearch.findComponent<VList>('.station-search-result-list');
            await resultList.vm.$forceUpdate();

            const listItems = resultList.findAllComponents({ name: 'v-list-item' });
            const firstImage = listItems[0].find('img.station-search-search-type-icon');
            expect(firstImage.exists()).toBe(true);
            // Expected values are defined with vi.mock()
            expect(firstImage.attributes('src')).toBe('some-icon-url/station.something');
            expect(listItems[1].find('img.station-search-search-type-icon').exists()).toBe(false);
        });

        it('shows the match of the result in a strong tag', async () => {
            infrastructureState.currentSearchTerm = 'some-some';
            infrastructureState.currentSearchedMapPositions[0].name = 'i-am-before--some-some--i-am-after';
            stationSearch = await mountWithDefaults(StationSearch, defaults);

            const resultList = stationSearch.findComponent<VList>('.station-search-result-list');

            const listItemName = resultList
                .findAllComponents({ name: 'v-list-item' })[0]
                .findComponent({ name: 'v-list-item-title' });
            expect(listItemName.text()).toBe('i-am-before--some-some--i-am-after');
            const nameSpans = listItemName.findAll('span');
            const nameStrong = listItemName.find('strong');
            expect(nameSpans[0].text()).toBe('i-am-before--');
            expect(nameStrong.text()).toBe('some-some');
            expect(nameSpans[1].text()).toBe('--i-am-after');
        });
    });
});
