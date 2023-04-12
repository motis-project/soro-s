import { VSelect, VListItem } from 'vuetify/components';

import { mountWithDefaults } from '@/test/helpers/mount-with-defaults';
import SoroSelect from '@/components/base/soro-select.vue';

const noOption = await mountWithDefaults(SoroSelect, {
  props: {
    label: 'some-label',
    options: []
  }
});

const oneOption = await mountWithDefaults(SoroSelect, {
  props: {
    label: 'some-label',
    options: ['option-1']
  }
});

const twoOptions = await mountWithDefaults(SoroSelect, {
  props: {
    label: 'some-label',
    options: ['option-1', 'option-2']
  }
});

describe('soro-select', async () => {
  it('displays the label and the options with the inner select', async () => {
    const innerSelect = twoOptions.getComponent<typeof VSelect>({
      name: 'v-select'
    });

    expect(innerSelect.props('label')).toBe('some-label');
    expect(innerSelect.props('items')).toStrictEqual(['option-1', 'option-2']);
  });

  it("displays a selection clear item that is prepended to all others and emits a 'select' event on click", async () => {
    const listItems = oneOption.findAllComponents<typeof VListItem>({
      name: 'v-list-item'
    });
    listItems[0].vm.$emit('click', new MouseEvent('left'));

    expect(listItems).toHaveLength(2);
    expect(listItems[0].text()).toBe('Clear selection');
    expect(listItems[1].text()).not.toBe('Clear selection');
    expect(oneOption.emitted('select')).toStrictEqual([[null]]);
  });

  it("emits a 'select' event when the inner select emits a model update", async () => {
    const innerSelect = noOption.findComponent<typeof VSelect>({
      name: 'v-select'
    });
    innerSelect.vm.$emit('update:modelValue', 'new-value');

    expect(noOption.emitted('select')).toStrictEqual([['new-value']]);
  });

  it('resets the current value when the value given in the props changes', async () => {
    await noOption.setData({ currentValue: 'some-outdated-value' });

    await noOption.setProps({ value: 'some-new-value' });

    expect(noOption.vm.currentValue).toBe('some-new-value');
  });
});
