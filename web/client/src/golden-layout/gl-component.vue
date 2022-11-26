<template>
  <div ref="GLComponent" style="position: absolute; overflow: hidden">
    <slot ref="innerComponent"></slot>
  </div>
</template>

<script setup lang="ts">
import {ref} from "vue";
import { ComponentContainer, JsonValue } from "golden-layout";

const GLComponent = ref<null | HTMLElement>(null);
const innerComponent = ref<null | HTMLElement>(null);

const numberToPixels = (value: number): string => {
  return value.toString(10) + "px";
};

interface glInnerComponent {
  setContainer: Function,
  setComponentState: Function,
}

const setPosAndSize = (
    left: number,
    top: number,
    width: number,
    height: number
): void => {
  if (GLComponent.value) {
    const el = GLComponent.value as HTMLElement;
    el.style.left = numberToPixels(left);
    el.style.top = numberToPixels(top);
    el.style.width = numberToPixels(width);
    el.style.height = numberToPixels(height);
  }
};

const setVisibility = (visible: boolean): void => {
  if (GLComponent.value) {
    const el = GLComponent.value as HTMLElement;
    if (visible) {
      el.style.display = "";
    } else {
      el.style.display = "none";
    }
  }
};

const setZIndex = (value: string): void => {
  if (GLComponent.value) {
    const el = GLComponent.value as HTMLElement;
    el.style.zIndex = value;
  }
};

const setContainer = (container: ComponentContainer) => {
  if (innerComponent.value) {
    const innerElement = innerComponent.value as unknown as glInnerComponent;
    innerElement.setContainer(container);
  }
}
const setComponentState = (componentState: JsonValue) => {
  if (innerComponent.value) {
    const innerElement = innerComponent.value as unknown as glInnerComponent;
    innerElement.setComponentState(componentState);
  }
}

defineExpose({
  setPosAndSize,
  setVisibility,
  setZIndex,
  setContainer,
  setComponentState,
});
</script>
