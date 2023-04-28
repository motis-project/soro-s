import { Module } from 'vuex';
import { GlobalState } from '@/stores/stores';
import { LngLatBounds, GeoJSONFeature } from 'maplibre-gl';
import {
  InterlockingRoute,
  SearchResult,
  StationRoute
} from '@/util/SoroClient';

export type UnixRange = [number, number];

export type SidebarState = {
  // infrastructure state
  infrastructures: string[];
  currentInfrastructure?: string;
  // timetable state
  timetables: string[];
  currentTimetable?: string;
  // date state
  currentDateRange?: UnixRange;
  // sidebar search state
  currentSearchResults: SearchResult[];
  currentBoundingBox?: LngLatBounds;
  currentSearchError?: string;
  // sidebar station state
  currentStation?: number;
  highlightedStationRoutes: GeoJSONFeature[];
  highlightedInterlockingRoutes: GeoJSONFeature[];
  // dev tools state
  trainIdsFilter: number[];
};

function routeToGeoJSON(route: StationRoute | InterlockingRoute) {
  if (!route.path) {
    throw new Error('Route has no path, was it fetched directly?');
  }

  return {
    type: 'Feature',
    properties: { id: route.id },
    geometry: {
      coordinates: route.path.map((point) => [point.lon, point.lat]),
      type: 'LineString'
    }
  };
}

export const SidebarNamespace = 'sidebar';

export const SidebarStore: Module<SidebarState, GlobalState> = {
  namespaced: true,

  state(): SidebarState {
    return {
      // infrastructure state
      infrastructures: [],
      currentInfrastructure: undefined,
      // timetable state
      timetables: [],
      currentTimetable: undefined,
      // date state
      currentDateRange: undefined,
      // search state
      currentSearchResults: [],
      currentBoundingBox: undefined,
      currentSearchError: undefined,
      // station state
      currentStation: undefined,
      // map state
      highlightedStationRoutes: [],
      highlightedInterlockingRoutes: [],
      // dev tools state:
      trainIdsFilter: []
    };
  },

  mutations: {
    setInfrastructures(state, infrastructures) {
      state.infrastructures = infrastructures;
    },

    setCurrentInfrastructure(state, currentInfrastructure) {
      state.currentInfrastructure = currentInfrastructure;
    },

    setTimetables(state, timetables) {
      state.timetables = timetables;
    },

    setCurrentTimetable(state, currentTimetable) {
      state.currentTimetable = currentTimetable;
    },

    setCurrentDateRange(state, currentDateRange: UnixRange) {
      state.currentDateRange = currentDateRange;
    },

    setCurrentSearchResults(state, currentSearchResults) {
      state.currentSearchResults = currentSearchResults;
    },

    setCurrentBoundingBox(state, currentBoundingBox: LngLatBounds) {
      state.currentBoundingBox = currentBoundingBox;
    },

    setCurrentSearchError(state, currentSearchError) {
      state.currentSearchError = currentSearchError;
    },

    setCurrentStation(state, currentStation) {
      state.currentStation = currentStation;
    },

    addHighlightedStationRoute(state, route: GeoJSONFeature) {
      state.highlightedStationRoutes.push(route);
    },

    deleteHighlightedStationRoute(state, routeId: number) {
      state.highlightedStationRoutes = state.highlightedStationRoutes.filter(
        (route) => route.properties.id !== routeId
      );
    },

    addHighlightedInterlockingRoute(state, route: GeoJSONFeature) {
      state.highlightedInterlockingRoutes.push(route);
    },

    deleteHighlightedInterlockingRoute(state, routeId: number) {
      state.highlightedInterlockingRoutes =
        state.highlightedInterlockingRoutes.filter(
          (route) => route.properties.id !== routeId
        );
    },

    setTrainIdsFilter(state, trainIdsFilter) {
      state.trainIdsFilter = trainIdsFilter;
    }
  },

  actions: {
    async initialLoad({ commit, rootState }) {
      rootState.soroClient
        .infrastructures()
        .then((r) => commit('setInfrastructures', r.infrastructures))
        .catch(console.error);
    },

    async addHighlightedStationRoute({ commit, rootState, state }, routeId) {
      if (!state.currentInfrastructure) {
        return;
      }

      const route = await rootState.soroClient
        .infrastructure(state.currentInfrastructure)
        .stationRoute(routeId);

      commit('addHighlightedStationRoute', routeToGeoJSON(route));
    },

    deleteHighlightedStationRoute({ commit }, routeId) {
      commit('deleteHighlightedStationRoute', routeId);
    },

    async addHighlightedInterlockingRoute(
      { commit, rootState, state },
      routeId
    ) {
      if (!state.currentInfrastructure) {
        return;
      }

      const route = await rootState.soroClient
        .infrastructure(state.currentInfrastructure)
        .interlockingRoute(routeId);

      commit('addHighlightedInterlockingRoute', routeToGeoJSON(route));
    },

    deleteHighlightedInterlockingRoute({ commit }, routeId) {
      commit('deleteHighlightedInterlockingRoute', routeId);
    },

    loadInfrastructure({ commit, rootState }, infrastructureName) {
      commit('setCurrentInfrastructure', infrastructureName);
      commit('setCurrentTimetable', undefined);

      if (!infrastructureName) {
        return;
      }

      rootState.soroClient
        .infrastructure(infrastructureName)
        .timetables()
        .then((response) => commit('setTimetables', response.timetables))
        .catch(console.error);
    },

    loadTimetable({ commit, rootState, state }, timetableName) {
      if (!state.currentInfrastructure) {
        return;
      }

      rootState.soroClient
        .infrastructure(state.currentInfrastructure)
        .timetable(timetableName)
        .get()
        .then((response) => {
          commit('setCurrentDateRange', [
            response.timetable.interval.start,
            response.timetable.interval.end
          ]);
        })
        .catch(console.error);

      commit('setCurrentTimetable', timetableName);
    },

    setCurrentDateRange({ commit }, dateRange: UnixRange) {
      commit('setCurrentDateRange', dateRange);
    },

    setTrainIdsFilter({ commit }, trainIdsFilter: number[]) {
      commit('setTrainIdsFilter', trainIdsFilter);
    }
  }
};
