import { Module } from 'vuex';
import { GlobalState } from '@/stores/stores';
import { LngLatBounds, GeoJSONFeature } from 'maplibre-gl';
import {
  InterlockingRoute,
  SearchResult,
  StationRoute
} from '@/util/SoroClient';

export type InfrastructureState = {
  infrastructures: string[];
  currentInfrastructure?: string;
  // sidebar search state
  currentSearchResults: SearchResult[];
  currentBoundingBox?: LngLatBounds;
  currentSearchError?: string;
  // sidebar station state
  currentStation?: number;
  highlightedStationRoutes: GeoJSONFeature[];
  highlightedInterlockingRoutes: GeoJSONFeature[];
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

export const InfrastructureNamespace = 'infrastructure';

export const InfrastructureStore: Module<InfrastructureState, GlobalState> = {
  namespaced: true,

  state(): InfrastructureState {
    return {
      infrastructures: [],
      currentInfrastructure: undefined,
      // search state
      currentSearchResults: [],
      currentBoundingBox: undefined,
      currentSearchError: undefined,
      // station state
      currentStation: undefined,
      // map state
      highlightedStationRoutes: [],
      highlightedInterlockingRoutes: []
    };
  },

  mutations: {
    setInfrastructures(state, infrastructures) {
      state.infrastructures = infrastructures;
    },

    setCurrentInfrastructure(state, currentInfrastructure) {
      state.currentInfrastructure = currentInfrastructure;
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
    }
  },

  actions: {
    async initialLoad({ commit, rootState }) {
      rootState.soroClient
        .infrastructures()
        .then((response) => {
          commit('setInfrastructures', response.infrastructures);
        })
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

    load({ commit }, infrastructureFilename) {
      commit('setCurrentInfrastructure', infrastructureFilename);
    },

    unload({ commit }) {
      commit('setCurrentInfrastructure', undefined);
    }
  }
};
