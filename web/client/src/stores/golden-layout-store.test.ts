import { GoldenLayoutStore } from '@/stores/golden-layout-store';
import { Commit, Dispatch } from 'vuex';

describe('golden-layout-store', async () => {
    let state: Record<string, any>;
    let commit: Commit;
    let dispatch: Dispatch;

    beforeEach(async () => {
        state = {};
        commit = vi.fn((mutationName: string, args: any) =>
            GoldenLayoutStore.mutations?.[mutationName]?.(state, args));
        dispatch = vi.fn((actionName: string, value: any) =>
            // @ts-ignore
            GoldenLayoutStore.actions[actionName]({
                state,
                dispatch,
                commit, 
            }, value));
        window.localStorage.clear();
    });

    it('loads the given layout into gl and dispatches \'loadSettings\' when initializing golden layout', async () => {
        const GLRootComponentMock = { loadGLLayout: vi.fn() };
        state.rootComponent = GLRootComponentMock;

        // @ts-ignore
        await GoldenLayoutStore.actions.initGoldenLayout({
            state,
            dispatch,
        }, { some: 'layout' });

        expect(GLRootComponentMock.loadGLLayout).toHaveBeenCalledWith({ some: 'layout' });
        expect(dispatch).toHaveBeenCalledWith('loadSettings');
    });

    it('can add a new component via the golden layout root component', async () => {
        const GLRootComponentMock = { addGLComponent: vi.fn() };
        state.rootComponent = GLRootComponentMock;

        // @ts-ignore
        await GoldenLayoutStore.actions.addGoldenLayoutTab({ state }, {
            componentTechnicalName: 'some-technical-name',
            title: 'some-title', 
        });

        expect(GLRootComponentMock.addGLComponent).toHaveBeenCalledWith(
            'some-technical-name',
            'some-title',
        );
    });

    describe('saveSettings()', async () => {
        it('gets the layout config from the gl root component and sets it in the local storage', async () => {
            const GLRootComponentMock = { getLayoutConfig: vi.fn(() => ({ some: 'layout' })) };
            state.rootComponent = GLRootComponentMock;

            // @ts-ignore
            await GoldenLayoutStore.actions.saveSettings({ state });

            expect(GLRootComponentMock.getLayoutConfig).toHaveBeenCalledTimes(1);
            expect(window.localStorage.getItem('goldenLayout.layout')).toBe(JSON.stringify({ some: 'layout' }));
        });

        it('does not set the layout in the local storage if it is not defined', async () => {
            const GLRootComponentMock = { getLayoutConfig: vi.fn(() => undefined) };
            state.rootComponent = GLRootComponentMock;

            // @ts-ignore
            await GoldenLayoutStore.actions.saveSettings({ state });

            expect(GLRootComponentMock.getLayoutConfig).toHaveBeenCalledTimes(1);
            expect(window.localStorage).toHaveLength(0);
        });
    });

    describe('loadSettings()', async () => {
        it('loads the layout from the local storage into the gl root component', async () => {
            window.localStorage.setItem('goldenLayout.layout', JSON.stringify({ some: 'saved-layout' }));
            const GLRootComponentMock = { loadGLLayout: vi.fn() };
            state.rootComponent = GLRootComponentMock;

            // @ts-ignore
            await GoldenLayoutStore.actions.loadSettings({ state });

            expect(GLRootComponentMock.loadGLLayout).toHaveBeenCalledWith({ some: 'saved-layout' });
        });

        it('does not load the layout from the local storage into the gl root component if the local storage does not define the layout', async () => {
            window.localStorage.removeItem('goldenLayout.layout');
            const GLRootComponentMock = { loadGLLayout: vi.fn() };
            state.rootComponent = GLRootComponentMock;

            // @ts-ignore
            await GoldenLayoutStore.actions.loadSettings({ state });

            expect(GLRootComponentMock.loadGLLayout).not.toHaveBeenCalled();
        });
    });

    it('can set the root component via a mutation', async () => {
        // @ts-ignore
        GoldenLayoutStore.mutations.setRootComponent(state, { some: 'component' });

        expect(state.rootComponent).toStrictEqual({ some: 'component' });
    });
});
