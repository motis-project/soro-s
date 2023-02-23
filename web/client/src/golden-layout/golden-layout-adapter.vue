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

/*******************
 * Data
 *******************/
const GLRoot = ref<null | HTMLElement>(null);
let GLayout: VirtualLayout;
const GlcKeyPrefix = readonly(ref('glc_'));

const MapComponents = new Map<ComponentContainer, { refId: number; glc: typeof GoldenLayoutComponent }>();
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
    const glc = markRaw(defineAsyncComponent(() => import(`../components/golden-layout-components/${componentType}.vue`)));

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

const addGLComponent = async (componentType: string, title: string) => {
    if (componentType.length == 0) {
        throw new Error('addGLComponent: Component\'s type is empty');
    }

    const index = addComponent(componentType);

    await nextTick(); // wait 1 tick for vue to add the dom

    GLayout.addComponent(componentType, { refId: index }, title);
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
        const content = contents.shift() ?? [];

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
    if (GLRoot.value == null) {
        throw new Error('Golden Layout can\'t find the root DOM!');
    }

    const onResize = () => {
        const dom = GLRoot.value;
        const width = dom ? dom.offsetWidth : 0;
        const height = dom ? dom.offsetHeight : 0;
        GLayout.setSize(width, height);
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
        const component = MapComponents.get(container);
        if (!component || !component?.glc) {
            throw new Error('handleContainerVirtualRectingRequiredEvent: Component not found');
        }

        const containerBoundingClientRect =
        container.element.getBoundingClientRect();
        const left =
        containerBoundingClientRect.left - GlBoundingClientRect.left;
        const top = containerBoundingClientRect.top - GlBoundingClientRect.top;
        component.glc.setPosAndSize(left, top, width, height);
    };

    const handleContainerVirtualVisibilityChangeRequiredEvent = (
        container: ComponentContainer,
        visible: boolean,
    ): void => {
        const component = MapComponents.get(container);
        if (!component || !component?.glc) {
            throw new Error('handleContainerVirtualVisibilityChangeRequiredEvent: Component not found');
        }

        component.glc.setVisibility(visible);
    };

    const handleContainerVirtualZIndexChangeRequiredEvent = (
        container: ComponentContainer,
        logicalZIndex: LogicalZIndex,
        defaultZIndex: string,
    ): void => {
        const component = MapComponents.get(container);
        if (!component || !component?.glc) {
            throw new Error('handleContainerVirtualZIndexChangeRequiredEvent: Component not found');
        }

        component.glc.setZIndex(defaultZIndex);
    };

    const bindComponentEventListener = (
        container: ComponentContainer,
        itemConfig: ResolvedComponentItemConfig,
    ): ComponentContainer.BindableComponent => {
        let refId = -1;
        if (itemConfig && itemConfig.componentState) {
            refId = (itemConfig.componentState as Json).refId as number;
        } else {
            throw new Error('bindComponentEventListener: component\'s ref id is required');
        }

        const ref = GlcKeyPrefix.value + refId;
        // @ts-ignore
        const component = instance?.refs[ref][0] as typeof GoldenLayoutComponent;

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
        const component = MapComponents.get(container);
        if (!component || !component?.glc) {
            throw new Error('handleUnbindComponentEvent: Component not found');
        }

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
