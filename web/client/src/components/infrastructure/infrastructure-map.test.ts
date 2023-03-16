import { mountWithDefaults } from '@/test-utils/mount-with-defaults';
import { VueWrapper } from '@vue/test-utils';
import { ComponentPublicInstance } from 'vue';
import InfrastructureMap, { initiallyCheckedControls } from './infrastructure-map.vue';
import MaplibreGl, { Map } from 'maplibre-gl';
import { InfrastructureNamespace, InfrastructureState } from '@/stores/infrastructure-store';
import { ElementType, ElementTypes } from './element-types';
import { Mock } from 'vitest';
import * as AddIcons from './add-icons';
import { addIcons } from './add-icons';
import { transformUrl } from '@/api/api-client';
import * as MapStyle from './map-style';
import { createInfrastructureMapStyle } from './map-style';
import {
    highlightSignalStationRoute,
    deHighlightSignalStationRoute,
    highlightStationRoute,
    deHighlightStationRoute,
} from './highlight-helpers';
import { SpecialLegendControl } from './infrastructure-legend.vue';

vi.mock('./map-style', async () => {
    const originalMapStyle = await vi.importActual<typeof MapStyle>('./map-style');

    return {
        ...originalMapStyle,
        createInfrastructureMapStyle: vi.fn(originalMapStyle.createInfrastructureMapStyle),
    };
});
vi.mock('@/api/api-client',() => ({ transformUrl: vi.fn() }));
vi.mock('./add-icons',async () => ({
    ...(await  vi.importActual<typeof AddIcons>('./add-icons')),
    addIcons: vi.fn(),
}));
vi.mock('./highlight-helpers', () => ({
    deHighlightSignalStationRoute: vi.fn(),
    deHighlightStationRoute: vi.fn(),
    highlightSignalStationRoute: vi.fn(),
    highlightStationRoute: vi.fn(),
}));
vi.mock('maplibre-gl', async () => {
    const originalMaplibreGL = await vi.importActual<typeof MaplibreGl>('golden-layout');

    return {
        ...originalMaplibreGL,
        Map: vi.fn(() => {
            return {
                on: vi.fn(),
                jumpTo: vi.fn(),
                remove: vi.fn(),
                setLayoutProperty: vi.fn(),
                setFilter: vi.fn(),
                setStyle: vi.fn(),
                dragPan: { enable: vi.fn() },
            };
        }),
    };
});

const getLastMapLibreGlMapInstance = () => (Map as Mock).mock.results.at(-1)?.value;

describe('infrastructure-map', async () => {
    let infrastructureMap: VueWrapper<ComponentPublicInstance<any>>;
    let maplibreGLMapInstance: Map & Mock;
    const infrastructureState: InfrastructureState =  {
        infrastructures: [],
        currentSearchedMapPositions: [],
    };

    const defaults = {
        store: {
            [InfrastructureNamespace]: { state: infrastructureState },
        },
        injections: {
            goldenLayoutKeyInjection: 'some-golden-key-injection',
        },
    };

    beforeEach(async () => {
        window.localStorage.clear();
        infrastructureState.infrastructures = [];
        infrastructureState.currentSearchedMapPositions = [];
        infrastructureState.currentInfrastructure = 'some-infrastructure';
        infrastructureMap = await mountWithDefaults(InfrastructureMap, defaults);
        maplibreGLMapInstance = getLastMapLibreGlMapInstance();
        vi.clearAllMocks();
    });

    describe('when the component is created', async () => {
        it('sets the currently checked controls when the local storage contains a value for this window using the injection', async () => {
            window.localStorage.setItem(
                'infrastructure[some-golden-key-injection].checkedControls',
                JSON.stringify([
                    ElementType.STATION,
                    ElementType.BUMPER,
                ]),
            );
            infrastructureMap = await mountWithDefaults(InfrastructureMap, defaults);

            expect(infrastructureMap.vm.checkedControls).toStrictEqual([
                ElementType.STATION,
                ElementType.BUMPER,
            ]);
        });

        it('does not set the currently checked controls when the local storage contains no value for this window using the injection', async () => {
            window.localStorage.removeItem('infrastructure[some-golden-key-injection].checkedControls');
            infrastructureMap = await mountWithDefaults(InfrastructureMap, defaults);

            expect(infrastructureMap.vm.checkedControls).toStrictEqual(initiallyCheckedControls);
        });

        describe('when an infrastructure is selected', async () => {
            beforeEach(async () => {
                infrastructureState.currentInfrastructure = 'some-current-infrastructure';
            });

            it('creates a new map instance', async () => {
                infrastructureMap = await mountWithDefaults(InfrastructureMap, defaults);
                const map = getLastMapLibreGlMapInstance();

                const mapElement = infrastructureMap.find({ ref: 'map' });
                expect(Map).toHaveBeenCalledWith({
                    attributionControl: false,
                    zoom: 18,
                    hash: 'location',
                    center: [10, 50],
                    maxBounds: [[-5, 40], [25, 60]],
                    bearing: 0,
                    container: mapElement.element,
                    transformRequest: expect.any(Function),
                    style: expect.any(Object),
                });
                expect(map.dragPan.enable).toHaveBeenCalledWith({
                    linearity: 0.01,
                    maxSpeed: 1400,
                    deceleration: 2500,
                });
            });

            it('creates the map instance with a request transformer that prepends the passed infrastructure name', async () => {
                infrastructureState.currentInfrastructure = 'fancy-infrastructure';
                await mountWithDefaults(InfrastructureMap, defaults);
                const passedOptions = (Map as Mock).mock.lastCall[0];

                (transformUrl as Mock).mockImplementation((url: string) => `transformed:${url}`);

                const slashResult =  passedOptions.transformRequest('/some-slash-url');
                const nonSlashResult = passedOptions.transformRequest('some-non-slash-url');
                expect(slashResult).toStrictEqual({
                    url: 'transformed:/fancy-infrastructure/some-slash-url',
                });
                expect(nonSlashResult).toBeUndefined();
            });

            it('registers a load handler that adds the icons to the map and sets the visibility of all controls', async () => {
                infrastructureMap = await mountWithDefaults(InfrastructureMap, defaults);
                await infrastructureMap.setData({ checkedControls: [ElementType.HALT] });
                const map = getLastMapLibreGlMapInstance();

                expect(map.on).toHaveBeenCalledWith('load', expect.any(Function));
                const registeredHandler = map.on.mock.lastCall[1];
                await registeredHandler();
                expect(addIcons).toHaveBeenCalledWith(map);
                expect(map.setLayoutProperty)
                    .toHaveBeenCalledWith(`${ElementType.HALT}-layer`, 'visibility', 'visible');
                expect(map.setLayoutProperty)
                    .toHaveBeenCalledWith(`${ElementType.STATION}-layer`, 'visibility', 'none');
            });

            it('does not attempt to set the visibility of all controls when the libreGLMap is not set at loading time', async () => {
                infrastructureMap = await mountWithDefaults(InfrastructureMap, defaults);
                await infrastructureMap.setData({ checkedControls: [ElementType.HALT] });
                const map = getLastMapLibreGlMapInstance();
                const registeredHandler = map.on.mock.lastCall[1];
                infrastructureMap.vm.libreGLMap = undefined;
                vi.clearAllMocks();

                await registeredHandler();

                expect(addIcons).not.toHaveBeenCalled();
                expect(map.setLayoutProperty).not.toHaveBeenCalled();
            });

            it('creates a map style with the current theme and the currently checked controls', async () => {
                infrastructureMap = await mountWithDefaults(InfrastructureMap, defaults);

                expect(createInfrastructureMapStyle).toHaveBeenCalledWith({
                    currentTheme: infrastructureMap.vm.$vuetify.theme.current,
                    activatedElements: initiallyCheckedControls,
                });
            });
        });

        it('creates no new map instance when no infrastructure is selected', async () => {
            infrastructureState.currentInfrastructure = undefined;
            infrastructureMap = await mountWithDefaults(InfrastructureMap, defaults);

            expect(Map).not.toHaveBeenCalled();
        });
    });

    it('displays an infrastructure legend with the initially checked controls', async () => {
        const legend = infrastructureMap.findComponent({ name: 'infrastructure-legend' });

        expect(legend.exists());
        expect(legend.props().checkedControls).toStrictEqual(initiallyCheckedControls);
    });

    describe('when the legend emits a \'reset\' event', async () => {
        it('resets the checked controls to the initially checked controls', async () => {
            infrastructureMap.vm.checkedControls = ['something-something'];

            const legend = infrastructureMap.findComponent({ name: 'infrastructure-legend' });
            legend.vm.$emit('reset');

            expect(infrastructureMap.vm.checkedControls).toStrictEqual(initiallyCheckedControls);
        });

        it('stores the currently checked controls in local storage', async () => {
            window.localStorage.setItem('infrastructure[some-golden-key-injection].checkedControls', '[]');

            const legend = infrastructureMap.findComponent({ name: 'infrastructure-legend' });
            legend.vm.$emit('reset');

            expect(window.localStorage.getItem('infrastructure[some-golden-key-injection].checkedControls'))
                .toStrictEqual(JSON.stringify(initiallyCheckedControls));
        });

        it.each(ElementTypes.map((type: string) => [
            type,
            initiallyCheckedControls.includes(type),
        ]))('sets the element type visibility for the map element %s to %s', async (elementType, visibility) => {
            const legend = infrastructureMap.findComponent({ name: 'infrastructure-legend' });
            legend.vm.$emit('reset');

            expect(maplibreGLMapInstance.setLayoutProperty)
                .toHaveBeenCalledWith(`${elementType}-layer`, 'visibility', visibility ? 'visible' : 'none');
        });

        it('sets an unrestricted filter on the element types', async () => {
            await infrastructureMap.setData({ checkedControls: [] });
            const legend = infrastructureMap.findComponent({ name: 'infrastructure-legend' });
            legend.vm.$emit('reset');

            expect(maplibreGLMapInstance.setFilter).toHaveBeenCalledWith(`${ElementType.HALT}-layer`, undefined);
            expect(maplibreGLMapInstance.setFilter).toHaveBeenCalledWith(`circle-${ElementType.HALT}-layer`, undefined);
        });
    });

    describe('when the legend emits a \'change\' event', async () => {
        it('stores the currently checked controls in local storage', async () => {
            window.localStorage.setItem('infrastructure[some-golden-key-injection].checkedControls', '[]');
            infrastructureMap.vm.checkedControls = ['something-something'];

            const legend = infrastructureMap.findComponent({ name: 'infrastructure-legend' });
            legend.vm.$emit('change', 'some-new-control', true);

            expect(window.localStorage.getItem('infrastructure[some-golden-key-injection].checkedControls'))
                .toStrictEqual(JSON.stringify([
                    'something-something',
                    'some-new-control',
                ]));
        });

        it('does not set the elements layer visibility when no map is set', async () => {
            infrastructureMap.vm.libreGLMap = undefined;

            const legend = infrastructureMap.findComponent({ name: 'infrastructure-legend' });
            legend.vm.$emit('change', 'some-new-control', true);

            expect(maplibreGLMapInstance.setLayoutProperty).not.toHaveBeenCalled();
        });

        describe('when the given element should be made visible', async () => {
            it('pushes the element to the checked controls', async () => {
                infrastructureMap.vm.checkedControls = ['something-something'];

                const legend = infrastructureMap.findComponent({ name: 'infrastructure-legend' });
                legend.vm.$emit('change', 'some-new-control', true);

                expect(infrastructureMap.vm.checkedControls).toStrictEqual([
                    'something-something',
                    'some-new-control',
                ]);
            });

            it('sets the visibility of the elements layer on the map to \'visible\'', async () => {
                const legend = infrastructureMap.findComponent({ name: 'infrastructure-legend' });
                legend.vm.$emit('change', 'some-new-control', true);

                expect(maplibreGLMapInstance.setLayoutProperty).toHaveBeenCalledWith(
                    'some-new-control-layer',
                    'visibility',
                    'visible',
                );
            });

            it('sets the visibility of the elements circle layer on the map to \'visible\' when the element is not of type \'station\'', async () => {
                const legend = infrastructureMap.findComponent({ name: 'infrastructure-legend' });
                legend.vm.$emit('change', 'not-station', true);

                expect(maplibreGLMapInstance.setLayoutProperty).toHaveBeenCalledWith(
                    'circle-not-station-layer',
                    'visibility',
                    'visible',
                );
            });

            it('does not set the visibility of the elements circle layer on the map to \'visible\' when the element is of type \'station\'', async () => {
                const legend = infrastructureMap.findComponent({ name: 'infrastructure-legend' });
                legend.vm.$emit('change', 'station', true);

                expect(maplibreGLMapInstance.setLayoutProperty).not.toHaveBeenCalledWith(
                    'circle-station-layer',
                    'visibility',
                    'visible',
                );
            });
        });

        describe('when the given element should be made invisible', async () => {
            it('removes the element from the checked controls', async () => {
                infrastructureMap.vm.checkedControls = [
                    'something-something',
                    'some-old-control',
                ];

                const legend = infrastructureMap.findComponent({ name: 'infrastructure-legend' });
                legend.vm.$emit('change', 'some-old-control', false);

                expect(infrastructureMap.vm.checkedControls).toStrictEqual(['something-something']);
            });

            it('sets the visibility of the elements layer on the map to \'none\'', async () => {
                const legend = infrastructureMap.findComponent({ name: 'infrastructure-legend' });
                legend.vm.$emit('change', 'some-old-control', false);

                expect(maplibreGLMapInstance.setLayoutProperty).toHaveBeenCalledWith(
                    'some-old-control-layer',
                    'visibility',
                    'none',
                );
            });

            it('sets the visibility of the elements circle layer on the map to \'none\' when the element is not of type \'station\'', async () => {
                const legend = infrastructureMap.findComponent({ name: 'infrastructure-legend' });
                legend.vm.$emit('change', 'not-station', false);

                expect(maplibreGLMapInstance.setLayoutProperty).toHaveBeenCalledWith(
                    'circle-not-station-layer',
                    'visibility',
                    'none',
                );
            });

            it('does not set the visibility of the elements circle layer on the map to \'none\' when the element is of type \'station\'', async () => {
                const legend = infrastructureMap.findComponent({ name: 'infrastructure-legend' });
                legend.vm.$emit('change', 'station', false);

                expect(maplibreGLMapInstance.setLayoutProperty).not.toHaveBeenCalledWith(
                    'circle-station-layer',
                    'visibility',
                    'none',
                );
            });
        });

        describe('when the given element is of a special legend control type', async () => {
            it('does not set the elements layer visibility', async () => {
                const legend = infrastructureMap.findComponent({ name: 'infrastructure-legend' });
                legend.vm.$emit('change', SpecialLegendControl.RISING, true);

                expect(maplibreGLMapInstance.setLayoutProperty).not.toHaveBeenCalled();
            });

            it.each([
                ['all passing', true, true, undefined],
                ['only falling', false, true, ['!', ['get', 'rising']]],
                ['only rising', true, false, ['get', 'rising']],
                ['all blocked', false, false, ['boolean', false]],
            ])(
                'can set an %s filter on every element type except station when rising is %s and falling is %s',
                async (name: string, risingCheck: boolean, fallingCheck: boolean, expectedFilter?: Array<any>) => {
                    infrastructureMap.vm.checkedControls = [];

                    const legend = infrastructureMap.findComponent({ name: 'infrastructure-legend' });
                    legend.vm.$emit('change', SpecialLegendControl.RISING, risingCheck);
                    legend.vm.$emit('change', SpecialLegendControl.FALLING, fallingCheck);

                    expect(maplibreGLMapInstance.setFilter)
                        .toHaveBeenCalledWith(`${ElementType.BUMPER}-layer`, expectedFilter);
                    expect(maplibreGLMapInstance.setFilter)
                        .toHaveBeenCalledWith(`circle-${ElementType.BUMPER}-layer`, expectedFilter);
                    expect(maplibreGLMapInstance.setFilter)
                        .not.toHaveBeenCalledWith(`${ElementType.STATION}-layer`, expectedFilter);
                    expect(maplibreGLMapInstance.setFilter)
                        .not.toHaveBeenCalledWith(`circle-${ElementType.STATION}-layer`, expectedFilter);
                },
            );
        });
    });

    describe('when the current infrastructure changes', async () => {
        it('removes a previously instantiated map', async () => {
            const mapInstance = { remove: vi.fn() };
            infrastructureMap.vm.libreGLMap = mapInstance;

            // Currently we need to call watchers like this as we did not yet migrate to the vue3 options api
            infrastructureMap.vm.$options.watch.currentInfrastructure.call(infrastructureMap.vm);

            expect(mapInstance.remove).toHaveBeenCalled();
        });

        it('instantiates a new map when a new infrastructure is given', async () => {
            await infrastructureMap.vm.$options.watch.currentInfrastructure.call(
                infrastructureMap.vm,
                'new-infrastructure',
            );

            expect(Map).toHaveBeenCalled();
        });

        it('instantiates no new map when no new infrastructure is given', async () => {
            infrastructureMap.vm.$options.watch.currentInfrastructure.call(infrastructureMap.vm, null);

            expect(Map).not.toHaveBeenCalled();
        });
    });

    describe('when the currently searched map position changes', async () => {
        it('lets the map jump to the given map position', async () => {
            infrastructureMap.vm.$options.watch.currentSearchedMapPosition.call(
                infrastructureMap.vm,
                { some: 'position' },
            );

            expect(maplibreGLMapInstance.jumpTo).toHaveBeenCalledWith({
                center: { some: 'position' },
                zoom: 14,
            });
        });

        it('does not attempt to focus the map on the given position when no map is given', async () => {
            infrastructureMap.vm.libreGLMap = null;

            infrastructureMap.vm.$options.watch.currentSearchedMapPosition.call(
                infrastructureMap.vm,
                { some: 'position' },
            );

            expect(maplibreGLMapInstance.jumpTo).not.toHaveBeenCalled();
        });
    });

    describe('when the current theme changes', async () => {
        it('creates a new map style and sets it on the map', async () => {
            await infrastructureMap.setData({ checkedControls: [ElementType.BUMPER] });

            infrastructureMap.vm.$options.watch.currentTheme.handler.call(
                infrastructureMap.vm,
                { current: { value: { some: 'current-theme' } } },
            );

            expect(createInfrastructureMapStyle).toHaveBeenCalledWith({
                currentTheme: { some: 'current-theme' },
                activatedElements: [ElementType.BUMPER],
            });
            const expectedResult = (createInfrastructureMapStyle as Mock).mock.results[0].value;
            expect(maplibreGLMapInstance.setStyle).toHaveBeenCalledWith(expectedResult);
        });

        it('does not create a new map style when no map is given', async () => {
            infrastructureMap.vm.libreGLMap = null;

            infrastructureMap.vm.$options.watch.currentTheme.handler.call(
                infrastructureMap.vm,
                { current: { value: { some: 'current-theme' } } },
            );

            expect(createInfrastructureMapStyle).not.toHaveBeenCalled();
            expect(maplibreGLMapInstance.setStyle).not.toHaveBeenCalled();
        });

        it('has a deeply reactive handler set on the current theme', async () => {
            expect(infrastructureMap.vm.$options.watch.currentTheme.deep).toBe(true);
        });
    });

    describe('when the highlighted signal station route id changes', async () => {
        it('highlights the signal station route when a new signal station route id is given', async () => {
            infrastructureState.currentInfrastructure = 'some-current-infrastructure';
            infrastructureMap = await mountWithDefaults(InfrastructureMap, defaults);
            const map = getLastMapLibreGlMapInstance();

            infrastructureMap.vm.$options.watch.highlightedSignalStationRouteID.call(
                infrastructureMap.vm,
                'new-signal-station-route-id',
                'old-signal-station-route-id',
            );

            expect(highlightSignalStationRoute).toHaveBeenCalledWith(
                map,
                'some-current-infrastructure',
                'new-signal-station-route-id',
            );
        });

        it('dehighlights the signal station route when no new signal station route id is given', async () => {
            infrastructureMap = await mountWithDefaults(InfrastructureMap, defaults);
            const map = getLastMapLibreGlMapInstance();

            infrastructureMap.vm.$options.watch.highlightedSignalStationRouteID.call(
                infrastructureMap.vm,
                null,
                'old-signal-station-route-id',
            );

            expect(deHighlightSignalStationRoute).toHaveBeenCalledWith(
                map,
                'old-signal-station-route-id',
            );
        });

        it('does not attempt to highlight or dehighlight the signal station route when no map is given', async () => {
            infrastructureMap.vm.libreGLMap = null;

            infrastructureMap.vm.$options.watch.highlightedSignalStationRouteID.call(
                infrastructureMap.vm,
                'new-signal-station-route-id',
                'old-signal-station-route-id',
            );

            expect(highlightSignalStationRoute).not.toHaveBeenCalled();
            expect(deHighlightSignalStationRoute).not.toHaveBeenCalled();
        });
    });

    describe('when the highlighted station route id changes', async () => {
        it('highlights the station route when a new station route id is given', async () => {
            infrastructureState.currentInfrastructure = 'some-current-infrastructure';
            infrastructureMap = await mountWithDefaults(InfrastructureMap, defaults);
            const map = getLastMapLibreGlMapInstance();

            infrastructureMap.vm.$options.watch.highlightedStationRouteID.call(
                infrastructureMap.vm,
                'new-station-route-id',
                'old-station-route-id',
            );

            expect(highlightStationRoute).toHaveBeenCalledWith(
                map,
                'some-current-infrastructure',
                'new-station-route-id',
            );
        });

        it('dehighlights the station route when no new station route id is given', async () => {
            infrastructureMap = await mountWithDefaults(InfrastructureMap, defaults);
            const map = getLastMapLibreGlMapInstance();

            infrastructureMap.vm.$options.watch.highlightedStationRouteID.call(
                infrastructureMap.vm,
                null,
                'old-station-route-id',
            );

            expect(deHighlightStationRoute).toHaveBeenCalledWith(
                map,
                'old-station-route-id',
            );
        });

        it('does not attempt to highlight or dehighlight the station route when no map is given', async () => {
            infrastructureMap.vm.libreGLMap = null;

            infrastructureMap.vm.$options.watch.highlightedStationRouteID.call(
                infrastructureMap.vm,
                'new-station-route-id',
                'old-station-route-id',
            );

            expect(highlightStationRoute).not.toHaveBeenCalled();
            expect(deHighlightStationRoute).not.toHaveBeenCalled();
        });
    });
});
