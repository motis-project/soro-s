export const InfrastructureNameSpace = 'infrastructure';

export const InfrastructureStore = {
    namespaced: true,

    state() {
        return {
            infrastructures: [],
            currentInfrastructure: null,
            highlightedSignalStationRouteID: null,
            highlightedStationRouteID: null,
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