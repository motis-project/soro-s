// @ts-nocheck as this store was (and is) unfinished
import type { Module } from 'vuex';

import { InfrastructureNamespace } from '@/stores/infrastructure-store';

export type TimetableState = {
  timetables: string[];
  currentTimetable?: string;
};

export const TimetableNamespace = 'timetable';

export const TimetableStore: Module<TimetableState, unknown> = {
  namespaced: true,

  state() {
    return {
      timetables: [],
      currentTimetable: undefined
    };
  },

  mutations: {
    setTimetables(state, timetables) {
      state.timetables = timetables;
    },

    setCurrentTimetable(state, currentTimetable) {
      state.currentTimetable = currentTimetable;
    }
  },

  actions: {
    initialLoad() {
      console.error('not implemented');
    },

    load({ rootState }, timetableFilename) {
      const currentInfrastructure =
        rootState[`${InfrastructureNamespace}/currentInfrastructure`];
      if (!currentInfrastructure) {
        console.error(
          'Tried loading a timetable with the currentInfrastructure undefined!'
        );
        return;
      }

      const timetablePath = timetableFileExists(timetableFilename);
      if (timetablePath) {
        // console.log('Reading', timetablePath, 'from IDBFS.');
        this._current = loadTimetableFromIDBFS(
          timetablePath,
          currentInfrastructure
        );
      } else {
        // console.log('Fetching', timetableFilename, 'from server.');
        sendRequest({ url: `timetable/${timetableFilename}` })
          .then((response) => response.arrayBuffer())
          .then((buf) => saveFileToIDBFS(timetableFilename, buf))
          .then(
            (filePath) =>
              (this._current = loadTimetableFromIDBFS(
                filePath,
                currentInfrastructure
              ))
          )
          .catch((e) => console.error(e));
      }
    },

    unload({ commit }) {
      commit('setCurrentTimetable', null);
    }
  }
};
