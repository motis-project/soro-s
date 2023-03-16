import { InfrastructureState, InfrastructureStore, SearchResult } from '@/stores/infrastructure-store';
import { Commit, Dispatch } from 'vuex';
import { sendPostData, sendRequest } from '@/api/api-client';
import { Mock } from 'vitest';

vi.mock('@/api/api-client', () => ({
    sendPostData: vi.fn(async () => ({})),
    sendRequest: vi.fn(async () => ({})),
}));

describe('infrastructure-store', async () => {
    let state: InfrastructureState;
    let commit: Commit;
    let dispatch: Dispatch;

    beforeEach(async () => {
        state = {
            currentInfrastructure: 'some-infrastructure',
            infrastructures: [],
            currentSearchedMapPositions: [],
        };
        commit = vi.fn((mutationName: string, args: any) =>
            InfrastructureStore.mutations?.[mutationName]?.(state, args));
        dispatch = vi.fn((actionName: string, value: any) =>
            // @ts-ignore
            InfrastructureStore.actions[actionName]({
                state,
                dispatch,
                commit, 
            }, value));
        window.localStorage.clear();
    });

    it('sends a request to the \'infrastructure\' endpoint and sets the filtered result in the state when initally loading', async () => {
        state.infrastructures = [];
        (sendRequest as Mock).mockImplementation(async () => ({
            json: () => ({
                dirs: [
                    '..',
                    'some-first-directory',
                    'some-second-directory',
                    '.',
                ],
            }),
        }));

        // @ts-ignore
        await InfrastructureStore.actions.initialLoad({ commit }, { some: 'layout' });

        expect(sendRequest).toHaveBeenCalledWith({ url: 'infrastructure' });
        expect(state.infrastructures).toStrictEqual([
            'some-first-directory',
            'some-second-directory',
        ]);
    });

    it('can load a given infrastructure into the state', async () => {
        state.currentInfrastructure = 'foo';

        // @ts-ignore
        await InfrastructureStore.actions.load({ commit }, 'some-infrastructure');

        expect(state.currentInfrastructure).toBe('some-infrastructure');
    });

    it('can unload the current infrastructure from the state', async () => {
        state.currentInfrastructure = 'foo';

        // @ts-ignore
        await InfrastructureStore.actions.unload({ commit });

        expect(state.currentInfrastructure).toBe(undefined);
    });

    describe('when searching for a position from a name', async () => {
        it('does not call the \'search\' api endpoint when no infrastructure is set in the state', async () => {
            state.currentInfrastructure = undefined;

            // @ts-ignore
            await InfrastructureStore.actions.searchPositionFromName({
                commit,
                state, 
            }, {
                query: 'some-query',
                includedTypes: [],
            });

            expect(sendPostData).not.toHaveBeenCalled();
        });

        it('does not call the \'search\' api endpoint and resets the current search position when no query is given', async () => {
            state.currentSearchedMapPosition = {
                lat: 50,
                lon: 60, 
            };

            // @ts-ignore
            await InfrastructureStore.actions.searchPositionFromName({
                commit,
                state,
            }, {
                query: null,
                includedTypes: [],
            });

            expect(sendPostData).not.toHaveBeenCalled();
            expect(state.currentSearchedMapPosition).toBe(undefined);
        });

        it('calls the \'search\' api endpoint with the given parameters and the current infrastructure from the state', async () => {
            state.currentInfrastructure = 'foo-infrastructure';

            // @ts-ignore
            await InfrastructureStore.actions.searchPositionFromName({
                commit,
                state,
            }, {
                query: 'some-query',
                includedTypes: ['some-first-type', 'some-second-type'],
            });

            expect(sendPostData).toHaveBeenCalledWith({
                url: 'search',
                data: {
                    query: 'some-query',
                    infrastructure: 'foo-infrastructure',
                    options: {
                        includedTypes: ['some-first-type', 'some-second-type'],
                    },
                },
            });
        });

        it('calls the \'search\' api endpoint with the given parameters and the current infrastructure from the state', async () => {
            state.currentInfrastructure = 'foo-infrastructure';

            // @ts-ignore
            await InfrastructureStore.actions.searchPositionFromName({
                commit,
                state,
            }, {
                query: 'some-query',
                includedTypes: ['some-first-type', 'some-second-type'],
            });

            expect(sendPostData).toHaveBeenCalledWith({
                url: 'search',
                data: {
                    query: 'some-query',
                    infrastructure: 'foo-infrastructure',
                    options: {
                        includedTypes: ['some-first-type', 'some-second-type'],
                    },
                },
            });
        });

        describe('when calling the \'search\' api endpoint successfully returns a json object', async () => {
            const searchResults: SearchResult[] = [];

            beforeEach(async () => {
                (sendPostData as Mock).mockImplementation(async () => ({
                    json: () => searchResults,
                }));
            });

            it('sets the current map position and the current search term', async () => {
                searchResults.length = 0;
                searchResults.push(
                    {
                        name: 'some-name',
                        position: {
                            lat: 40,
                            lon: 60,
                        },
                    },
                    {
                        name: 'some-other--name',
                        position: {
                            lat: 100,
                            lon: 200,
                        },
                    },
                );

                // @ts-ignore
                await InfrastructureStore.actions.searchPositionFromName({
                    commit,
                    state,
                }, {
                    query: 'some-query',
                    includedTypes: [],
                });

                expect(state.currentSearchTerm).toBe('some-query');
                expect(state.currentSearchedMapPositions).toStrictEqual([
                    {
                        name: 'some-name',
                        position: {
                            lat: 40,
                            lon: 60,
                        },
                    },
                    {
                        name: 'some-other--name',
                        position: {
                            lat: 100,
                            lon: 200,
                        },
                    },
                ]);
            });

            it('sets a \'not found\' error and does not modify the current map position when no positions were found', async () => {
                state.currentSearchedMapPosition = {
                    lat: 20,
                    lon: 40,
                };
                searchResults.length = 0;

                // @ts-ignore
                await InfrastructureStore.actions.searchPositionFromName({
                    commit,
                    state,
                }, {
                    query: 'some-query',
                    includedTypes: [],
                });

                expect(state.currentSearchError).toBe('Not found!');
                expect(state.currentSearchedMapPosition).toStrictEqual({
                    lat: 20,
                    lon: 40,
                });
            });

            it('resets the error and sets the current map position to the first position when positions were found', async () => {
                state.currentSearchError = 'some-error';
                searchResults.length = 0;
                searchResults.push(
                    {
                        name: 'some-name',
                        position: {
                            lat: 40,
                            lon: 60,
                        },
                    },
                    {
                        name: 'some-other--name',
                        position: {
                            lat: 100,
                            lon: 200,
                        },
                    },
                );

                // @ts-ignore
                await InfrastructureStore.actions.searchPositionFromName({
                    commit,
                    state,
                }, {
                    query: 'some-query',
                    includedTypes: [],
                });

                expect(state.currentSearchError).toBe(undefined);
                expect(state.currentSearchedMapPosition).toStrictEqual({
                    lat: 40,
                    lon: 60,
                });
            });
        });

        it('sets a generic error message in the state when converting the search response to json throws an error', async () => {
            (sendPostData as Mock).mockImplementation(async () => ({
                json: () => {
                    throw new Error(); 
                },
            }));
            state.currentInfrastructure = 'foo-infrastructure';

            // @ts-ignore
            await InfrastructureStore.actions.searchPositionFromName({
                commit,
                state,
            }, {
                query: 'some-query',
                includedTypes: ['some-first-type', 'some-second-type'],
            });

            expect(state.currentSearchError).toBe('An error occurred!');
        });
    });

    it('provides a mutation to set the highlighted signal station route id', async () => {
        // @ts-ignore
        InfrastructureStore.mutations.setHighlightedSignalStationRouteID(state, 'some-id');

        expect(state.highlightedSignalStationRouteID).toStrictEqual('some-id');
    });

    it('provides a mutation to set the highlighted station route id', async () => {
        // @ts-ignore
        InfrastructureStore.mutations.setHighlightedStationRouteID(state, 'some-id');

        expect(state.highlightedStationRouteID).toStrictEqual('some-id');
    });
});
