import { mountWithDefaults } from '@/test-utils/mount-with-defaults';
import { ComponentPublicInstance } from 'vue';
import { VueWrapper } from '@vue/test-utils';
import GoldenLayoutAdapter from '@/golden-layout/golden-layout-adapter.vue';
import { ComponentTechnicalName, GLComponentNames, GLComponentTitles } from '@/golden-layout/golden-layout-constants';
import { GoldenLayoutAdapter as AdapterType } from '@/golden-layout/golden-layout-adapter';
import GoldenLayout, { LayoutConfig } from 'golden-layout';
import { VirtualLayout } from 'golden-layout';
import { GoldenLayoutNamespace } from '@/stores/golden-layout-store';
import { Mock } from 'vitest';
import GoldenLayoutComponent from '@/golden-layout/golden-layout-component.vue';

vi.mock('golden-layout', async () => {
    const originalGoldenLayout = await vi.importActual<typeof GoldenLayout>('golden-layout');

    return {
        ...originalGoldenLayout,
        VirtualLayout: vi.fn((...args) => {
            const virtualLayoutInstance: any = new originalGoldenLayout.VirtualLayout(...args);

            vi.spyOn(virtualLayoutInstance, 'on');
            vi.spyOn(virtualLayoutInstance, 'bindComponentEvent');
            vi.spyOn(virtualLayoutInstance, 'addComponent');
            vi.spyOn(virtualLayoutInstance, 'saveLayout');
            vi.spyOn(virtualLayoutInstance, 'loadLayout');
            vi.spyOn(virtualLayoutInstance, 'clear');
            vi.spyOn(virtualLayoutInstance, 'setSize');

            return virtualLayoutInstance;
        }),
    };
});

const getLastVirtualLayoutInstance = () => (VirtualLayout as unknown as Mock).mock.results.at(-1)?.value;

describe('golden-layout-adapter', async () => {
    let adapter: VueWrapper<ComponentPublicInstance<AdapterType>>;
    let virtualLayoutInstance: any;
    const goldenLayoutActions = { saveSettings: vi.fn() };

    const defaults = {
        store: {
            [GoldenLayoutNamespace]: {
                actions: goldenLayoutActions,
            },
        },
    };

    beforeEach(async () => {
        adapter = await mountWithDefaults(GoldenLayoutAdapter, defaults);
        virtualLayoutInstance = getLastVirtualLayoutInstance();
        vi.clearAllMocks();
    });

    describe('mounted()', async () => {
        it('creates a new virtual layout to hook it into the DOM', async () => {
            adapter = await mountWithDefaults(GoldenLayoutAdapter, defaults);

            expect(VirtualLayout).toHaveBeenCalledWith(
                adapter.vm.$refs.GLRoot,
                expect.any(Function),
                expect.any(Function),
            );
        });

        it('registers a state change handler that will dispatch the \'saveSettings\' action', async () => {
            adapter = await mountWithDefaults(GoldenLayoutAdapter, defaults);
            virtualLayoutInstance = getLastVirtualLayoutInstance();

            expect(virtualLayoutInstance.on).toHaveBeenCalledWith(
                'stateChanged',
                expect.any(Function),
            );
            await virtualLayoutInstance.on.mock.lastCall[1]();
            expect(goldenLayoutActions.saveSettings).toHaveBeenCalledTimes(1);
        });

        it('registers a window resize handler that sets the size of the golden layout instance to the offset size of the root dom', async () => {
            vi.spyOn(window, 'addEventListener');
            adapter = await mountWithDefaults(GoldenLayoutAdapter, defaults);
            virtualLayoutInstance = getLastVirtualLayoutInstance();

            window.dispatchEvent(new Event('resize'));

            expect(window.addEventListener).toHaveBeenCalledWith(
                'resize',
                expect.any(Function),
                { passive: true },
            );
            const rootDom = adapter.find({ ref: 'GLRoot' });
            expect(virtualLayoutInstance.setSize).toHaveBeenCalledWith(
                (rootDom.element as HTMLElement).offsetWidth,
                (rootDom.element as HTMLElement).offsetHeight,
            );
        });
    });

    describe('addGLComponent()', async () => {
        it('creates a golden layout component containing the specified component with its default title', async () => {
            await adapter.vm.addGLComponent(ComponentTechnicalName.SIMULATION);
            await vi.dynamicImportSettled();

            const glComponents = adapter.findAllComponents({ name: 'golden-layout-component' });
            expect(glComponents).toHaveLength(1);
            const orderingGraphComponent = glComponents[0].findComponent({
                name: GLComponentNames[ComponentTechnicalName.SIMULATION],
            });
            expect(orderingGraphComponent.exists()).toBe(true);
        });

        it('uses the given title if a title is given', async () => {
            await adapter.vm.addGLComponent(ComponentTechnicalName.SIMULATION, 'some-fancy-title');

            expect(virtualLayoutInstance.addComponent).toHaveBeenCalledWith(
                GLComponentNames[ComponentTechnicalName.SIMULATION],
                expect.any(Object),
                'some-fancy-title',
            );
        });

        it('uses the default title if no title is given', async () => {
            await adapter.vm.addGLComponent(ComponentTechnicalName.SIMULATION, undefined);

            expect(virtualLayoutInstance.addComponent).toHaveBeenCalledWith(
                GLComponentNames[ComponentTechnicalName.SIMULATION],
                expect.any(Object),
                GLComponentTitles[ComponentTechnicalName.SIMULATION],
            );
        });

        it('uses consecutive reference ids for components', async () => {
            await adapter.vm.addGLComponent(ComponentTechnicalName.SIMULATION);
            await adapter.vm.addGLComponent(ComponentTechnicalName.SIMULATION);

            expect(virtualLayoutInstance.addComponent).toHaveBeenNthCalledWith(
                1,
                GLComponentNames[ComponentTechnicalName.SIMULATION],
                { refId: 0 },
                GLComponentTitles[ComponentTechnicalName.SIMULATION],
            );
            expect(virtualLayoutInstance.addComponent).toHaveBeenNthCalledWith(
                2,
                GLComponentNames[ComponentTechnicalName.SIMULATION],
                { refId: 1 },
                GLComponentTitles[ComponentTechnicalName.SIMULATION],
            );
        });

        it('reuses old reference ids for components from previous unbound components', async () => {
            // Bind first and second component
            await adapter.vm.addGLComponent(ComponentTechnicalName.SIMULATION);
            await adapter.vm.addGLComponent(ComponentTechnicalName.SIMULATION);

            // Unbind first component with its container
            const usedContainer = virtualLayoutInstance.bindComponentEvent.mock.calls[0][0];
            virtualLayoutInstance.unbindComponentEvent(usedContainer);

            vi.clearAllMocks();
            // Add fresh third component
            await adapter.vm.addGLComponent(ComponentTechnicalName.ORDERING_GRAPH);

            expect(virtualLayoutInstance.addComponent).toHaveBeenLastCalledWith(
                GLComponentNames[ComponentTechnicalName.ORDERING_GRAPH],
                { refId: 0 },
                GLComponentTitles[ComponentTechnicalName.ORDERING_GRAPH],
            );
        });
    });

    describe('loadGLLayout()', async () => {
        let defaultLayout: LayoutConfig;
        beforeEach(async () => {
            defaultLayout = {
                root: {
                    type: 'row',
                    content: [
                        {
                            type: 'component',
                            title: GLComponentTitles[ComponentTechnicalName.ORDERING_GRAPH],
                            componentType: GLComponentNames[ComponentTechnicalName.ORDERING_GRAPH],
                        },
                    ],
                },
            };
        });

        it('clears the golden layout instance and loads the new layout into the instance', async () => {
            await adapter.vm.loadGLLayout(defaultLayout);

            expect(virtualLayoutInstance.clear).toHaveBeenCalled();
            expect(virtualLayoutInstance.loadLayout).toHaveBeenCalledWith(defaultLayout);
        });

        it('throws an error if the layout config does not contain a root element', async () => {
            await expect(adapter.vm.loadGLLayout({
                root: undefined,
            })).rejects.toThrow('GoldenLayout: Root element not found');
        });

        it('loads a previously saved layout config transformed from resolved form', async () => {
            await adapter.vm.addGLComponent(ComponentTechnicalName.ORDERING_GRAPH, 'some-title');
            const layout = await adapter.vm.getLayoutConfig();

            await adapter.vm.loadGLLayout(layout);

            expect(virtualLayoutInstance.loadLayout).toHaveBeenCalledWith(LayoutConfig.fromResolved(layout));
        });

        it.each([
            ['refIds are given', [{ refId: 15 }, { refId: 42 }], [{ refId: 15 }, { refId: 42 }]],
            ['refIds are not given', [{}, undefined], [{ refId: 0 }, { refId: 1 }]],
        ])('loads a complex layout config with refIds for the components when %s', async (
            _: string,
            givenComponentStates: ({ refId?: number } | undefined)[],
            expectedComponentStates: { refId: number }[],
        ) => {
            defaultLayout = {
                root: {
                    type: 'row',
                    content: [
                        {
                            type: 'component',
                            title: GLComponentTitles[ComponentTechnicalName.ORDERING_GRAPH],
                            componentType: GLComponentNames[ComponentTechnicalName.ORDERING_GRAPH],
                            componentState: givenComponentStates[0],
                        },
                        {
                            type: 'column',
                            content: [ {
                                type: 'component',
                                title: GLComponentTitles[ComponentTechnicalName.SIMULATION],
                                componentType: GLComponentNames[ComponentTechnicalName.SIMULATION],
                                componentState: givenComponentStates[1],
                            }],
                        },
                    ],
                },
            };

            await adapter.vm.loadGLLayout(defaultLayout);

            expect(virtualLayoutInstance.loadLayout).toHaveBeenCalledWith({
                root: {
                    type: 'row',
                    content: [
                        {
                            type: 'component',
                            title: GLComponentTitles[ComponentTechnicalName.ORDERING_GRAPH],
                            componentType: GLComponentNames[ComponentTechnicalName.ORDERING_GRAPH],
                            componentState: expectedComponentStates[0],
                        },
                        {
                            type: 'column',
                            content: [ {
                                type: 'component',
                                title: GLComponentTitles[ComponentTechnicalName.SIMULATION],
                                componentType: GLComponentNames[ComponentTechnicalName.SIMULATION],
                                componentState: expectedComponentStates[1],
                            }],
                        },
                    ],
                },
            });
        });
    });

    it('allows getting the layout config via the golden layout instance', async () => {
        virtualLayoutInstance.saveLayout.mockImplementationOnce(() => ({ some: 'layout' }));

        const layoutConfig = adapter.vm.getLayoutConfig();

        expect(virtualLayoutInstance.saveLayout).toHaveBeenCalled();
        expect(layoutConfig).toStrictEqual({ some: 'layout' });
    });

    it('handles virtual recting required events on containers by setting the pos and size on their component', async () => {
        await adapter.vm.addGLComponent(ComponentTechnicalName.ORDERING_GRAPH);
        await vi.dynamicImportSettled();

        const usedContainer = virtualLayoutInstance.bindComponentEvent.mock.lastCall[0];
        // exposed functions via the vue expose compiler macro are only accessible when accessed via the refs through
        // the component, not via the ref option of the findComponent method of vue/test-utils
        const glComponent = (adapter.vm.$refs['glc_0'] as typeof GoldenLayoutComponent[])[0];
        vi.spyOn(glComponent, 'setPosAndSize');

        virtualLayoutInstance.beforeVirtualRectingEvent();
        usedContainer.virtualRectingRequiredEvent(
            usedContainer,
            10,
            20,
        );

        const glRootBoundingRect = (adapter.find({ ref: 'GLRoot' }).element as HTMLElement).getBoundingClientRect();
        const containerBoundingRect = (usedContainer.element as HTMLElement).getBoundingClientRect();
        expect(glComponent.setPosAndSize).toHaveBeenCalledWith(
            containerBoundingRect.left - glRootBoundingRect.left,
            containerBoundingRect.top - glRootBoundingRect.top,
            10,
            20,
        );
    });

    it('handles virtual visibility change required events by calling its component equivalent', async () => {
        await adapter.vm.addGLComponent(ComponentTechnicalName.ORDERING_GRAPH);
        await vi.dynamicImportSettled();

        const usedContainer = virtualLayoutInstance.bindComponentEvent.mock.lastCall[0];
        // exposed functions via the vue expose compiler macro are only accessible when accessed via the refs through
        // the component, not via the ref option of the findComponent method of vue/test-utils
        const glComponent = (adapter.vm.$refs['glc_0'] as typeof GoldenLayoutComponent[])[0];
        vi.spyOn(glComponent, 'setVisibility');

        usedContainer.virtualVisibilityChangeRequiredEvent(usedContainer, true);
        expect(glComponent.setVisibility).toHaveBeenCalledWith(true);
    });

    it('handles virtual z index change required events by calling its component equivalent with the default z index', async () => {
        await adapter.vm.addGLComponent(ComponentTechnicalName.ORDERING_GRAPH);
        await vi.dynamicImportSettled();

        const usedContainer = virtualLayoutInstance.bindComponentEvent.mock.lastCall[0];
        // exposed functions via the vue expose compiler macro are only accessible when accessed via the refs through
        // the component, not via the ref option of the findComponent method of vue/test-utils
        const glComponent = (adapter.vm.$refs['glc_0'] as typeof GoldenLayoutComponent[])[0];
        vi.spyOn(glComponent, 'setZIndex');

        usedContainer.virtualZIndexChangeRequiredEvent(usedContainer, 10, 50);
        expect(glComponent.setZIndex).toHaveBeenCalledWith(50);
    });

    // This test explicitly asserts that all known components can be mounted and is automatically extended
    it.each(Object.keys(ComponentTechnicalName).map((key) => ({
        componentTechnicalName: key as ComponentTechnicalName,
        componentName: GLComponentNames[key as ComponentTechnicalName],
    })))('GL component mount test %#: $componentName', async ({ componentTechnicalName, componentName }) => {
        await adapter.vm.addGLComponent(componentTechnicalName);
        await vi.dynamicImportSettled();

        const glComponents = adapter.findAllComponents({ name: 'golden-layout-component' });
        expect(glComponents).toHaveLength(1);
        const subComponent = glComponents[0].findComponent({ name: componentName });
        expect(subComponent.exists()).toBe(true);
    });
});
