import { VueWrapper } from '@vue/test-utils';
import { ComponentPublicInstance } from 'vue';

import { mountWithDefaults } from '@/test/helpers/mount-with-defaults';

import SoroButton from '@/components/base/soro-button.vue';

describe('soro-button', async () => {
  const soroButton: VueWrapper<ComponentPublicInstance<any>> =
    await mountWithDefaults(SoroButton, { props: { label: 'some-label' } });

  it('displays the label on the button', async () => {
    expect(soroButton.text()).toBe('some-label');
  });

  it("re-bubbles the 'click' event from the inner button", async () => {
    await soroButton.find('button').trigger('click');

    expect(soroButton.emitted('click')).toHaveLength(1);
  });
});
