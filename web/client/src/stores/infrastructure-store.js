export const InfrastructureNameSpace = 'infrastructure';

export const InfrastructureStore = {
    namespaced: true,

    state() {
        return {
            currentInfrastructure: null,
            highlightedSignalStationRouteID: null,
            highlightedStationRouteID: null,
        }
    },

    mutations: {
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
        load({ commit }, infrastructureFilename) {
            console.log("Switching to infrastructure to", infrastructureFilename);
            commit('setCurrentInfrastructure', infrastructureFilename);
        },

        unload({ commit }) {
            commit('setCurrentInfrastructure', null);
        },
    },
};