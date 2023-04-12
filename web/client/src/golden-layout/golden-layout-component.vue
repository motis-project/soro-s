<template>
    <div
        ref="GLComponent"
        style="position: absolute; overflow: hidden;"
    >
        <slot />
    </div>
</template>

<script setup lang="ts">
import { ref } from 'vue';

const GLComponent = ref<null | HTMLElement>(null);

const numberToPixels = (value: number) => value.toString(10) + 'px';
const setPosAndSize = (
    left: number,
    top: number,
    width: number,
    height: number,
): void => {
    const el = GLComponent.value as HTMLElement;
    el.style.left = numberToPixels(left);
    el.style.top = numberToPixels(top);
    el.style.width = numberToPixels(width);
    el.style.height = numberToPixels(height);
};

const setVisibility = (visible: boolean): void => {
    const el = GLComponent.value as HTMLElement;
    el.style.display = visible ? '' : 'none';
};

const setZIndex = (value: string): void => {
    const el = GLComponent.value as HTMLElement;
    el.style.zIndex = value;
};

defineExpose({
    setPosAndSize,
    setVisibility,
    setZIndex,
});
</script>
