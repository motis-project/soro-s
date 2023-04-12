import {
  InfrastructureState,
  InfrastructureStore
} from '@/stores/infrastructure-store';
import { Commit, Dispatch } from 'vuex';

vi.mock('@/api/api-client', () => ({
  sendPostData: vi.fn(async () => ({})),
  sendRequest: vi.fn(async () => ({}))
}));

describe('infrastructure-store', async () => {
  let state: InfrastructureState;
  let commit: Commit;
  let dispatch: Dispatch;

  beforeEach(async () => {
    state = {
      currentInfrastructure: 'some-infrastructure',
      infrastructures: [],
      currentSearchResults: [],
      highlightedStationRoutes: [],
      highlightedInterlockingRoutes: []
    };
    commit = vi.fn((mutationName: string, args: any) =>
      InfrastructureStore.mutations?.[mutationName]?.(state, args)
    );
    dispatch = vi.fn((actionName: string, value: any) =>
      // @ts-ignore
      InfrastructureStore.actions[actionName](
        {
          state,
          dispatch,
          commit
        },
        value
      )
    );
    window.localStorage.clear();
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
});
