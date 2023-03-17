import { mountWithDefaults } from '@/test-utils/mount-with-defaults';
import SoroSelect from './soro-select.vue';
import { VueWrapper } from '@vue/test-utils';
import { ComponentPublicInstance } from 'vue';
import { VSelect, VListItem } from 'vuetify/components';

describe('soro-select', async () => {
    let soroSelect: VueWrapper<ComponentPublicInstance<any>>;

    beforeEach(async () => {
        soroSelect = await mountWithDefaults(SoroSelect, {
            props: { label: 'some-label' },
        });
    });

    it('displays the label and the options with the inner select', async () => {
        soroSelect = await mountWithDefaults(SoroSelect, {
            props: {
                label: 'some-label',
                options: [
                    'option-1',
                    'option-2',
                ],
            },
        });

        const innerSelect = soroSelect.getComponent<typeof VSelect>( { name: 'v-select' });

        expect(innerSelect.props('label')).toBe('some-label');
        expect(innerSelect.props('items')).toStrictEqual([
            'option-1',
            'option-2',
        ]);
    });

    it('displays a selection clear item that is prepended to all others and emits a \'select\' event on click', async () => {
        soroSelect = await mountWithDefaults(SoroSelect, {
            props: {
                label: 'some-label',
                options: ['option-1'],
            },
        });

        const listItems = soroSelect.findAllComponents<typeof VListItem>({ name: 'v-list-item' });
        listItems[0].vm.$emit('click', new MouseEvent('left'));

        expect(listItems).toHaveLength(2);
        expect(listItems[0].text()).toBe('Clear selection');
        expect(listItems[1].text()).not.toBe('Clear selection');
        expect(soroSelect.emitted('select')).toStrictEqual([[null]]);
    });

    it('emits a \'select\' event when the inner select emits a model update', async () => {
        const innerSelect = soroSelect.findComponent<typeof VSelect>({ name: 'v-select' });
        innerSelect.vm.$emit('update:modelValue', 'new-value');

        expect(soroSelect.emitted('select')).toStrictEqual([['new-value']]);
    });

    it('resets the current value when the value given in the props changes', async () => {
        await soroSelect.setData({ currentValue: 'some-outdated-value' });

        await soroSelect.setProps({ value: 'some-new-value' });

        expect(soroSelect.vm.currentValue).toBe('some-new-value');
    });
});
