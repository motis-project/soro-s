import { SidebarState, SidebarStore } from '@/stores/sidebar-store';
import { Commit, Dispatch } from 'vuex';

vi.mock('@/api/api-client', () => ({
  sendPostData: vi.fn(async () => ({})),
  sendRequest: vi.fn(async () => ({}))
}));

describe('sidebar-store', async () => {
  let state: SidebarState;
  let commit: Commit;
  let dispatch: Dispatch;

  beforeEach(async () => {
    state = {
      infrastructures: [],
      currentInfrastructure: 'some-infrastructure',
      timetables: [],
      currentTimetable: '',
      currentSearchResults: [],
      highlightedStationRoutes: [],
      highlightedInterlockingRoutes: []
    };
    commit = vi.fn((mutationName: string, args: any) =>
      SidebarStore.mutations?.[mutationName]?.(state, args)
    );
    dispatch = vi.fn((actionName: string, value: any) =>
      // @ts-ignore
      SidebarStore.actions[actionName](
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
    await SidebarStore.actions.loadInfrastructure({ commit }, undefined);

    expect(state.currentInfrastructure).toBe(undefined);
  });
});
