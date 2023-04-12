import { defineComponent, inject } from 'vue';

export const TestInjectComponentRef = 'injectionTestComponent';
export const constructTestInjectComponent = (injectionKey: string) => defineComponent({
    name: 'TestInjectComponent',
    setup: () => ({
        value: inject(injectionKey),
        injectionRefString: TestInjectComponentRef,
    }),
    template: '<span :ref="injectionRefString">{{ value }}</span>',
});

export default {
    TestInjectComponentRef,
    constructTestInjectComponent,
};
