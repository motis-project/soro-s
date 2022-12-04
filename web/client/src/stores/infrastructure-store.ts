import { Module } from 'vuex';

type InfrastructureState = {
    infrastructures: string[],
    currentInfrastructure?: string,
    highlightedSignalStationRouteID?: string,
    highlightedStationRouteID?: string,
}

export const InfrastructureNamespace: string = 'infrastructure';

export const InfrastructureStore: Module<InfrastructureState, undefined> = { // TODO rewrite with global namespace
    namespaced: true,

    state() {
        return {
            infrastructures: [],
            currentInfrastructure: undefined,
            highlightedSignalStationRouteID: undefined,
            highlightedStationRouteID: undefined,
        }
    },

    mutations: {
        setInfrastructures(state, infrastructures) {
            state.infrastructures = infrastructures;
        },

        setCurrentInfrastructure(state, currentInfrastructure) {
            state.currentInfrastructure = currentInfrastructure;
        },

        setHighlightedSignalStationRouteID(state, highlightedSignalStationRouteID) {
            state.highlightedSignalStationRouteID = highlightedSignalStationRouteID;
        },

        setHighlightedStationRouteID(state, highlightedStationRouteID) {
            state.highlightedStationRouteID = highlightedStationRouteID;
        },
    },

    actions: {
        initialLoad({ commit }) {
            fetch(window.origin + '/infrastructure/')
                .then(response => response.json())
                .then(dir => {
                    commit('setInfrastructures', dir.dirs);
                });
        },

        load({ commit }, infrastructureFilename) {
            console.log("Switching to infrastructure to", infrastructureFilename);
            commit('setCurrentInfrastructure', infrastructureFilename);
        },

        unload({ commit }) {
            commit('setCurrentInfrastructure', null);
        },
    },
};