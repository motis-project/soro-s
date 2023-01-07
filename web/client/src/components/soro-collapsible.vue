<template>
    <div class="wrap-collapsible">
        <label
            class="collapsible-toggle"
        >
            <input
                class="collapsible-toggle hidden"
                type="checkbox"
                @input="toggleContent"
            >
            {{ label }}
        </label>
        <div
            ref="innerContent"
            class="collapsible-content"
        >
            <div class="content-inner">
                <slot />
            </div>
        </div>
    </div>
</template>

<script lang="ts">
import { defineComponent } from 'vue';

export default defineComponent({
    name: 'SoroCollapsible',

    props: {
        label: {
            type: String,
            required: true
        },
    },

    methods: {
        toggleContent() {
            (this.$refs.innerContent as HTMLDivElement).classList.toggle('show-content');
        },
    },
});
</script>

<style scoped>
input[type="checkbox"].hidden {
    opacity: 0;
}

.wrap-collapsible {
    display: inline;
}

.collapsible-toggle {
    display: flex;
    flex-direction: row;
    max-height: 50%;
    font-weight: 550;
    font-family: var(--main-font-family);
    font-size: 16px;
    text-align: left;
    box-shadow: 0 10px 20px rgb(0 0 0 / 19%), 0 6px 6px rgb(0 0 0 / 23%);
    padding: 1em;
    color: var(--text-color);
    background: var(--dialog-color);
    cursor: pointer;
    border-radius: var(--border-radius);
}

.collapsible-toggle:hover {
    background-color: var(--element-color);
}

.collapsible-content {
    position: relative;
    max-height: 0;
    overflow-y: auto;
    overflow-x: hidden;
    box-shadow: 0 20px 10px rgb(0 0 0 / 19%), 0 8px 6px rgb(0 0 0 / 23%);
}

.collapsible-content.show-content {
    max-height: 100%;
}

.content-inner {
    display: flex;
    flex-direction: column;
    background: var(--dialog-color);
    border-bottom: 1px solid var(--dialog-color);
    border-bottom-left-radius: var(--border-radius);
    border-bottom-right-radius: var(--border-radius);
    padding: 1em;
}

.collapsible-toggle:checked + .collapsible-toggle + .collapsible-content {
    max-height: 65%;
}

.collapsible-toggle:checked + .collapsible-toggle::after {
    content: "\e5ce";
}

.collapsible-toggle:checked + .collapsible-toggle {
    border-bottom-right-radius: 0;
    border-bottom-left-radius: 0;
}
</style>