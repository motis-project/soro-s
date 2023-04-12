<template>
    <div style="position: absolute; height: 100%;">
        <div
            ref="GLRoot"
            style="position: absolute; width: 100%; height: 100%;"
        />
        <!-- Root dom for Golden-Layout manager -->
        <div style="position: absolute; width: 100%; height: 100%;">
            <golden-layout-component
                v-for="pair in AllComponents"
                :key="pair[0]"
                :ref="GlcKeyPrefix + pair[0]"
            >
                <component
                    :is="pair[1].glc"
                    :golden-layout-key="GlcKeyPrefix + pair[0]"
                    :container="pair[1].container"
                />
            </golden-layout-component>
        </div>
    </div>
</template>

<script setup lang="ts">
import {
    onMounted,
    ref,
    markRaw,
    readonly,
    defineAsyncComponent,
    nextTick,
    getCurrentInstance,
} from 'vue';
import {
    ComponentContainer,
    Json,
    LayoutConfig,
    RowOrColumnItemConfig,
    StackItemConfig,
    ComponentItemConfig,
    ResolvedComponentItemConfig,
    LogicalZIndex,
    VirtualLayout,
    ResolvedLayoutConfig,
} from 'golden-layout';
import GoldenLayoutComponent from './golden-layout-component.vue';
import { useStore } from 'vuex';
import { GoldenLayoutNamespace } from '@/stores/golden-layout-store';
import { ComponentTechnicalName, GLComponentNames, GLComponentTitles } from './golden-layout-constants';

/*******************
 * Data
 *******************/
const GLRoot = ref<null | HTMLElement>(null);
let GLayout: VirtualLayout;
const GlcKeyPrefix = readonly(ref('glc_'));

type mappedComponent = { refId: number; glc: typeof GoldenLayoutComponent };
const MapComponents = new Map<ComponentContainer, mappedComponent>();
const AllComponents = ref(new Map<number, { glc: any; container?: ComponentContainer }>());
const UnusedIndexes: number[] = [];
let CurIndex = 0;
let GlBoundingClientRect: DOMRect;

const instance = getCurrentInstance();
const store = useStore();

/*******************
 * Method
 *******************/
/** @internal */
const addComponent = (componentType: string, alreadyUsedIndex?: number) => {
    const glc = markRaw(defineAsyncComponent(() => import(`./components/${componentType}.vue`)));

    let index;
    if (alreadyUsedIndex) {
        index = alreadyUsedIndex;
    } else if (UnusedIndexes.length > 0) {
        index = UnusedIndexes.pop() as number;
    } else {
        while (AllComponents.value.has(CurIndex)) {
            CurIndex++; 
        }

        index = CurIndex;
    }

    AllComponents.value.set(index, { glc });

    return index;
};

const addGLComponent = async (componentTechnicalName: ComponentTechnicalName, title?: string) => {
    const index = addComponent(GLComponentNames[componentTechnicalName]);
    await nextTick(); // wait 1 tick for vue to add the dom

    GLayout.addComponent(
        GLComponentNames[componentTechnicalName],
        { refId: index },
        title ?? GLComponentTitles[componentTechnicalName],
    );
};

type ValidItemConfig = RowOrColumnItemConfig | StackItemConfig | ComponentItemConfig;
const loadGLLayout = async (layoutConfig: LayoutConfig | ResolvedLayoutConfig) => {
    GLayout.clear();
    AllComponents.value.clear();

    if (!layoutConfig.root)  {
        throw new Error('GoldenLayout: Root element not found');
    }

    const config = LayoutConfig.isResolved(layoutConfig)
        ? LayoutConfig.fromResolved(layoutConfig as ResolvedLayoutConfig)
        : layoutConfig;
    const contents = [layoutConfig.root.content] as ValidItemConfig[][];

    let index = 0;
    while (contents.length > 0) {
        const content = contents.shift() as ValidItemConfig[];

        content.forEach((itemConfig) => {
            if (itemConfig.type !== 'component') {
                contents.push(itemConfig.content);

                return;
            }

            if (!itemConfig.componentState || !(typeof itemConfig.componentState === 'object')) {
                itemConfig.componentState = {};
            }

            const usedIndex = (itemConfig.componentState as Json).refId;
            index = addComponent(itemConfig.componentType as string, usedIndex as number);
            (itemConfig.componentState as Json).refId = index;
        });
    }

    await nextTick(); // wait 1 tick for vue to add the dom

    GLayout.loadLayout(config);
};

const getLayoutConfig = () => {
    return GLayout.saveLayout();
};

/*******************
 * Mount
 *******************/
onMounted(() => {
    const onResize = () => {
        const dom = GLRoot.value as HTMLElement;
        GLayout.setSize(dom.offsetWidth, dom.offsetHeight);
    };

    window.addEventListener('resize', onResize, { passive: true });

    const handleBeforeVirtualRectingEvent = () => {
        GlBoundingClientRect = (GLRoot.value as HTMLElement).getBoundingClientRect();
    };

    const handleContainerVirtualRectingRequiredEvent = (
        container: ComponentContainer,
        width: number,
        height: number,
    ): void => {
        const component = MapComponents.get(container) as mappedComponent;

        const containerBoundingClientRect = container.element.getBoundingClientRect();
        const left = containerBoundingClientRect.left - GlBoundingClientRect.left;
        const top = containerBoundingClientRect.top - GlBoundingClientRect.top;
        component.glc.setPosAndSize(left, top, width, height);
    };

    const handleContainerVirtualVisibilityChangeRequiredEvent = (
        container: ComponentContainer,
        visible: boolean,
    ): void => {
        const component = MapComponents.get(container) as mappedComponent;
        component.glc.setVisibility(visible);
    };

    const handleContainerVirtualZIndexChangeRequiredEvent = (
        container: ComponentContainer,
        logicalZIndex: LogicalZIndex,
        defaultZIndex: string,
    ): void => {
        const component = MapComponents.get(container) as mappedComponent;
        component.glc.setZIndex(defaultZIndex);
    };

    const bindComponentEventListener = (
        container: ComponentContainer,
        itemConfig: ResolvedComponentItemConfig,
    ): ComponentContainer.BindableComponent => {
        const refId = (itemConfig.componentState as Json).refId as number;
        const ref = GlcKeyPrefix.value + refId;
        const component = (instance?.refs[ref] as typeof GoldenLayoutComponent[])[0];

        MapComponents.set(container, {
            refId,
            glc: component, 
        });
        // @ts-ignore
        AllComponents.value.get(refId).container = container;

        container.virtualRectingRequiredEvent = handleContainerVirtualRectingRequiredEvent.bind(this);
        container.virtualVisibilityChangeRequiredEvent = handleContainerVirtualVisibilityChangeRequiredEvent.bind(this);
        container.virtualZIndexChangeRequiredEvent = handleContainerVirtualZIndexChangeRequiredEvent.bind(this);

        return {
            component,
            virtual: true,
        };
    };

    const unbindComponentEventListener = (container: ComponentContainer): void => {
        const component = MapComponents.get(container) as mappedComponent;
        MapComponents.delete(container);
        AllComponents.value.delete(component.refId);
        UnusedIndexes.push(component.refId);
    };

    GLayout = new VirtualLayout(
      GLRoot.value as HTMLElement,
      bindComponentEventListener,
      unbindComponentEventListener,
    );

    GLayout.beforeVirtualRectingEvent = handleBeforeVirtualRectingEvent;
    GLayout.on('stateChanged', () => store.dispatch(`${GoldenLayoutNamespace}/saveSettings`));
});

/*******************
 * Expose
 *******************/
defineExpose({
    addGLComponent,
    loadGLLayout,
    getLayoutConfig,
});
</script>

<style>
@import "goldenlayout-base.css";
@import "goldenlayout-mdl-theme.css";
</style>
