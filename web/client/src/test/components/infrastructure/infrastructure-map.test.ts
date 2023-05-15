import { ComponentPublicInstance } from 'vue';
import { VueWrapper } from '@vue/test-utils';

import MaplibreGl, { Map } from 'maplibre-gl';
import InfrastructureMap, {
  initiallyCheckedControls
} from '@/components/infrastructure/infrastructure-map.vue';
import { SidebarNamespace, SidebarState } from '@/stores/sidebar-store';
import {
  ElementType,
  ElementTypes
} from '@/components/infrastructure/element-types';
import { Mock } from 'vitest';
import * as AddIcons from '@/components/infrastructure/add-icons';
import * as MapStyle from '@/components/infrastructure/map-style';
import { SpecialLegendControl } from '@/components/infrastructure/infrastructure-legend.vue';

import { mountWithDefaults } from '@/test/helpers/mount-with-defaults';

vi.mock('./map-style', async () => {
  const originalMapStyle = await vi.importActual<typeof MapStyle>(
    './map-style'
  );

  return {
    ...originalMapStyle,
    createInfrastructureMapStyle: vi.fn(
      originalMapStyle.createInfrastructureMapStyle
    )
  };
});
vi.mock('./add-icons', async () => ({
  ...(await vi.importActual<typeof AddIcons>('./add-icons')),
  addIcons: vi.fn()
}));
vi.mock('maplibre-gl', async () => {
  const originalMaplibreGL = await vi.importActual<typeof MaplibreGl>(
    'golden-layout'
  );

  return {
    ...originalMaplibreGL,
    Map: vi.fn(() => {
      return {
        on: vi.fn(),
        jumpTo: vi.fn(),
        fitBounds: vi.fn(),
        remove: vi.fn(),
        setLayoutProperty: vi.fn(),
        setFilter: vi.fn(),
        setStyle: vi.fn(),
        dragPan: { enable: vi.fn() }
      };
    })
  };
});

const getLastMapLibreGlMapInstance = () =>
  (Map as Mock).mock.results.at(-1)?.value;

describe('infrastructure-map', async () => {
  let infrastructureMap: VueWrapper<ComponentPublicInstance<any>>;
  let maplibreGLMapInstance: Map & Mock;
  const sidebarState: SidebarState = {
    infrastructures: [],
    currentInfrastructure: '',
    timetables: [],
    currentTimetable: '',
    currentSearchResults: [],
    highlightedStationRoutes: [],
    highlightedInterlockingRoutes: [],
    trainIdsFilter: []
  };

  const defaults = {
    store: {
      [SidebarNamespace]: { state: sidebarState }
    },
    injections: {
      goldenLayoutKeyInjection: 'some-golden-key-injection'
    }
  };

  beforeEach(async () => {
    window.localStorage.clear();
    sidebarState.infrastructures = [];
    sidebarState.currentSearchResults = [];
    sidebarState.currentInfrastructure = 'some-infrastructure';
    infrastructureMap = await mountWithDefaults(InfrastructureMap, defaults);
    maplibreGLMapInstance = getLastMapLibreGlMapInstance();
    vi.clearAllMocks();
  });

  describe('when the component is created', async () => {
    it('sets the currently checked controls when the local storage contains a value for this window using the injection', async () => {
      window.localStorage.setItem(
        'infrastructure[some-golden-key-injection].checkedControls',
        JSON.stringify([ElementType.STATION, ElementType.BUMPER])
      );
      infrastructureMap = await mountWithDefaults(InfrastructureMap, defaults);

      expect(infrastructureMap.vm.checkedControls).toStrictEqual([
        ElementType.STATION,
        ElementType.BUMPER
      ]);
    });

    it('does not set the currently checked controls when the local storage contains no value for this window using the injection', async () => {
      window.localStorage.removeItem(
        'infrastructure[some-golden-key-injection].checkedControls'
      );
      infrastructureMap = await mountWithDefaults(InfrastructureMap, defaults);

      expect(infrastructureMap.vm.checkedControls).toStrictEqual(
        initiallyCheckedControls
      );
    });

    describe('when an infrastructure is selected', async () => {
      beforeEach(async () => {
        sidebarState.currentInfrastructure = 'some-current-infrastructure';
      });

      it('creates a new map instance', async () => {
        infrastructureMap = await mountWithDefaults(
          InfrastructureMap,
          defaults
        );

        const mapElement = infrastructureMap.find({ ref: 'map' });
        expect(Map).toHaveBeenCalledWith({
          attributionControl: false,
          zoom: 10,
          hash: 'location',
          bearing: 0,
          container: mapElement.element,
          transformRequest: expect.any(Function),
          style: expect.any(Object)
        });
      });
    });

    it('creates no new map instance when no infrastructure is selected', async () => {
      sidebarState.currentInfrastructure = undefined;
      infrastructureMap = await mountWithDefaults(InfrastructureMap, defaults);

      expect(Map).not.toHaveBeenCalled();
    });
  });

  it('displays an infrastructure legend with the initially checked controls', async () => {
    const legend = infrastructureMap.findComponent({
      name: 'infrastructure-legend'
    });

    expect(legend.exists());
    expect(legend.props().checkedControls).toStrictEqual(
      initiallyCheckedControls
    );
  });

  describe("when the legend emits a 'reset' event", async () => {
    it('resets the checked controls to the initially checked controls', async () => {
      infrastructureMap.vm.checkedControls = ['something-something'];

      const legend = infrastructureMap.findComponent({
        name: 'infrastructure-legend'
      });
      legend.vm.$emit('reset');

      expect(infrastructureMap.vm.checkedControls).toStrictEqual(
        initiallyCheckedControls
      );
    });

    it('stores the currently checked controls in local storage', async () => {
      window.localStorage.setItem(
        'infrastructure[some-golden-key-injection].checkedControls',
        '[]'
      );

      const legend = infrastructureMap.findComponent({
        name: 'infrastructure-legend'
      });
      legend.vm.$emit('reset');

      expect(
        window.localStorage.getItem(
          'infrastructure[some-golden-key-injection].checkedControls'
        )
      ).toStrictEqual(JSON.stringify(initiallyCheckedControls));
    });

    it.each(
      ElementTypes.map((type: string) => [
        type,
        initiallyCheckedControls.includes(type)
      ])
    )(
      'sets the element type visibility for the map element %s to %s',
      async (elementType, visibility) => {
        const legend = infrastructureMap.findComponent({
          name: 'infrastructure-legend'
        });
        legend.vm.$emit('reset');

        expect(maplibreGLMapInstance.setLayoutProperty).toHaveBeenCalledWith(
          `${elementType}-layer`,
          'visibility',
          visibility ? 'visible' : 'none'
        );
      }
    );

    it('sets an unrestricted filter on the element types', async () => {
      await infrastructureMap.setData({ checkedControls: [] });
      const legend = infrastructureMap.findComponent({
        name: 'infrastructure-legend'
      });
      legend.vm.$emit('reset');

      expect(maplibreGLMapInstance.setFilter).toHaveBeenCalledWith(
        `${ElementType.HALT}-layer`,
        undefined
      );
      expect(maplibreGLMapInstance.setFilter).toHaveBeenCalledWith(
        `circle-${ElementType.HALT}-layer`,
        undefined
      );
    });
  });

  describe("when the legend emits a 'change' event", async () => {
    it('stores the currently checked controls in local storage', async () => {
      window.localStorage.setItem(
        'infrastructure[some-golden-key-injection].checkedControls',
        '[]'
      );
      infrastructureMap.vm.checkedControls = ['something-something'];

      const legend = infrastructureMap.findComponent({
        name: 'infrastructure-legend'
      });
      legend.vm.$emit('change', 'some-new-control', true);

      expect(
        window.localStorage.getItem(
          'infrastructure[some-golden-key-injection].checkedControls'
        )
      ).toStrictEqual(
        JSON.stringify(['something-something', 'some-new-control'])
      );
    });

    it('does not set the elements layer visibility when no map is set', async () => {
      infrastructureMap.vm.libreGLMap = undefined;

      const legend = infrastructureMap.findComponent({
        name: 'infrastructure-legend'
      });
      legend.vm.$emit('change', 'some-new-control', true);

      expect(maplibreGLMapInstance.setLayoutProperty).not.toHaveBeenCalled();
    });

    describe('when the given element should be made visible', async () => {
      it('pushes the element to the checked controls', async () => {
        infrastructureMap.vm.checkedControls = ['something-something'];

        const legend = infrastructureMap.findComponent({
          name: 'infrastructure-legend'
        });
        legend.vm.$emit('change', 'some-new-control', true);

        expect(infrastructureMap.vm.checkedControls).toStrictEqual([
          'something-something',
          'some-new-control'
        ]);
      });

      it("sets the visibility of the elements layer on the map to 'visible'", async () => {
        const legend = infrastructureMap.findComponent({
          name: 'infrastructure-legend'
        });
        legend.vm.$emit('change', 'some-new-control', true);

        expect(maplibreGLMapInstance.setLayoutProperty).toHaveBeenCalledWith(
          'some-new-control-layer',
          'visibility',
          'visible'
        );
      });

      it("sets the visibility of the elements circle layer on the map to 'visible' when the element is not of type 'station'", async () => {
        const legend = infrastructureMap.findComponent({
          name: 'infrastructure-legend'
        });
        legend.vm.$emit('change', 'not-station', true);

        expect(maplibreGLMapInstance.setLayoutProperty).toHaveBeenCalledWith(
          'circle-not-station-layer',
          'visibility',
          'visible'
        );
      });

      it("does not set the visibility of the elements circle layer on the map to 'visible' when the element is of type 'station'", async () => {
        const legend = infrastructureMap.findComponent({
          name: 'infrastructure-legend'
        });
        legend.vm.$emit('change', 'station', true);

        expect(
          maplibreGLMapInstance.setLayoutProperty
        ).not.toHaveBeenCalledWith(
          'circle-station-layer',
          'visibility',
          'visible'
        );
      });
    });

    describe('when the given element should be made invisible', async () => {
      it('removes the element from the checked controls', async () => {
        infrastructureMap.vm.checkedControls = [
          'something-something',
          'some-old-control'
        ];

        const legend = infrastructureMap.findComponent({
          name: 'infrastructure-legend'
        });
        legend.vm.$emit('change', 'some-old-control', false);

        expect(infrastructureMap.vm.checkedControls).toStrictEqual([
          'something-something'
        ]);
      });

      it("sets the visibility of the elements layer on the map to 'none'", async () => {
        const legend = infrastructureMap.findComponent({
          name: 'infrastructure-legend'
        });
        legend.vm.$emit('change', 'some-old-control', false);

        expect(maplibreGLMapInstance.setLayoutProperty).toHaveBeenCalledWith(
          'some-old-control-layer',
          'visibility',
          'none'
        );
      });

      it("sets the visibility of the elements circle layer on the map to 'none' when the element is not of type 'station'", async () => {
        const legend = infrastructureMap.findComponent({
          name: 'infrastructure-legend'
        });
        legend.vm.$emit('change', 'not-station', false);

        expect(maplibreGLMapInstance.setLayoutProperty).toHaveBeenCalledWith(
          'circle-not-station-layer',
          'visibility',
          'none'
        );
      });

      it("does not set the visibility of the elements circle layer on the map to 'none' when the element is of type 'station'", async () => {
        const legend = infrastructureMap.findComponent({
          name: 'infrastructure-legend'
        });
        legend.vm.$emit('change', 'station', false);

        expect(
          maplibreGLMapInstance.setLayoutProperty
        ).not.toHaveBeenCalledWith(
          'circle-station-layer',
          'visibility',
          'none'
        );
      });
    });

    describe('when the given element is of a special legend control type', async () => {
      it('does not set the elements layer visibility', async () => {
        const legend = infrastructureMap.findComponent({
          name: 'infrastructure-legend'
        });
        legend.vm.$emit('change', SpecialLegendControl.RISING, true);

        expect(maplibreGLMapInstance.setLayoutProperty).not.toHaveBeenCalled();
      });

      it.each([
        ['all passing', true, true, undefined],
        ['only falling', false, true, ['!', ['get', 'rising']]],
        ['only rising', true, false, ['get', 'rising']],
        ['all blocked', false, false, ['boolean', false]]
      ])(
        'can set an %s filter on every element type except station when rising is %s and falling is %s',
        async (
          name: string,
          risingCheck: boolean,
          fallingCheck: boolean,
          expectedFilter?: Array<any>
        ) => {
          infrastructureMap.vm.checkedControls = [];

          const legend = infrastructureMap.findComponent({
            name: 'infrastructure-legend'
          });
          legend.vm.$emit('change', SpecialLegendControl.RISING, risingCheck);
          legend.vm.$emit('change', SpecialLegendControl.FALLING, fallingCheck);

          expect(maplibreGLMapInstance.setFilter).toHaveBeenCalledWith(
            `${ElementType.BUMPER}-layer`,
            expectedFilter
          );
          expect(maplibreGLMapInstance.setFilter).toHaveBeenCalledWith(
            `circle-${ElementType.BUMPER}-layer`,
            expectedFilter
          );
          expect(maplibreGLMapInstance.setFilter).not.toHaveBeenCalledWith(
            `${ElementType.STATION}-layer`,
            expectedFilter
          );
          expect(maplibreGLMapInstance.setFilter).not.toHaveBeenCalledWith(
            `circle-${ElementType.STATION}-layer`,
            expectedFilter
          );
        }
      );
    });
  });

  describe('when the current infrastructure changes', async () => {
    it('removes a previously instantiated map', async () => {
      const mapInstance = { remove: vi.fn() };
      infrastructureMap.vm.libreGLMap = mapInstance;

      // Currently we need to call watchers like this as we did not yet migrate to the vue3 options api
      infrastructureMap.vm.$options.watch.currentInfrastructure.call(
        infrastructureMap.vm
      );

      expect(mapInstance.remove).toHaveBeenCalled();
    });

    it('instantiates a new map when a new infrastructure is given', async () => {
      await infrastructureMap.vm.$options.watch.currentInfrastructure.call(
        infrastructureMap.vm,
        'new-infrastructure'
      );

      expect(Map).toHaveBeenCalled();
    });

    it('instantiates no new map when no new infrastructure is given', async () => {
      infrastructureMap.vm.$options.watch.currentInfrastructure.call(
        infrastructureMap.vm,
        null
      );

      expect(Map).not.toHaveBeenCalled();
    });
  });

  describe('when the currently searched map position changes', async () => {
    it('lets the map jump to the given map position', async () => {
      infrastructureMap.vm.$options.watch.currentBoundingBox.call(
        infrastructureMap.vm,
        { some: 'position' }
      );

      expect(maplibreGLMapInstance.fitBounds).toHaveBeenCalled();
    });

    it('does not attempt to focus the map on the given position when no map is given', async () => {
      infrastructureMap.vm.libreGLMap = null;

      infrastructureMap.vm.$options.watch.currentBoundingBox.call(
        infrastructureMap.vm,
        { some: 'position' }
      );

      expect(maplibreGLMapInstance.fitBounds).not.toHaveBeenCalled();
    });
  });
});
