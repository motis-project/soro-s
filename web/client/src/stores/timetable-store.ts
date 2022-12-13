// @ts-nocheck as this store was (and is) unfinished

// import {
//   exists,
//   saveToPersistent,
//   saveFileToIDBFS,
//   timetableFileExists
// } from "../utl/IDBFSHelper.js";
import { InfrastructureNamespace } from './infrastructure-store';
import type { Module } from 'vuex';

// import { Module } from "../soro-client.js";

function loadTimetableFromIDBFS(filePath, currentInfrastructure) {
	/* Loading from cache is very slow atm, so disabled
     const cachePath = new Module.FilesystemPath('/idbfs/cache/' + infrastructureFilename);
     if (exists(cachePath.string())) {
       this._current = new Module.Infrastructure(cachePath, true);
     } else {
     */

	const timetableOpts = new Module.TimetableOptions();
	timetableOpts.timetable_path = new Module.FilesystemPath(filePath);

	try {
		return new Module.Timetable(timetableOpts, currentInfrastructure);
	} catch (e) {
		console.error(e);
	}

	/* try {
       this._current.save(cachePath);
       saveToPersistent();
     } catch (e) {
       console.error("Could not write serialized cache, maybe compiled without SERIALIZE?");
       console.error(e);
     }
  }
     */
}

type TimetableState = {
    timetables: string[],
    currentTimetable?: string,
}

export const TimetableNamespace = 'timetable';

export const TimetableStore: Module<TimetableState, undefined> = {
	namespaced: true,

	state() {
		return {
			timetables: [],
			currentTimetable: undefined,
		};
	},

	mutations: {
		setTimetables(state, timetables) {
			state.timetables = timetables;
		},

		setCurrentTimetable(state, currentTimetable) {
			state.currentTimetable = currentTimetable;
		},
	},

	actions: {
		initialLoad({ commit }) {
			fetch(window.origin + '/timetable/')
				.then(response => response.json())
				.then(dir => {
					commit('setTimetables', dir.dirs);
				});
		},

		load({ commit, rootState }, timetableFilename) {
			const currentInfrastructure = rootState[`${InfrastructureNamespace}/currentInfrastructure`];
			if (!currentInfrastructure) {
				console.error('Tried loading a timetable with the currentInfrastructure undefined!');
				return;
			}

			const timetablePath = timetableFileExists(timetableFilename);
			if (timetablePath) {
				console.log('Reading', timetablePath, 'from IDBFS.');
				this._current = loadTimetableFromIDBFS(timetablePath, currentInfrastructure);
			} else {
				console.log('Fetching', timetableFilename, 'from server.');
				fetch(window.origin + '/timetable/' + timetableFilename)
					.then(response => response.arrayBuffer())
					.then(buf => saveFileToIDBFS(timetableFilename, buf))
					.then(filePath => this._current = loadTimetableFromIDBFS(filePath, currentInfrastructure))
					.catch(e => console.error(e));
			}
		},

		unload({ commit }) {
			commit('setCurrentTimetable', null);
		},
	},
};