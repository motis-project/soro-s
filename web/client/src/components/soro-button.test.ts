import { mountWithDefaults } from '@/test-utils/mount-with-defaults';
import SoroButton from './soro-button.vue';
import { VueWrapper } from '@vue/test-utils';
import { ComponentPublicInstance } from 'vue';

describe('soro-button', async () => {
    let soroButton: VueWrapper<ComponentPublicInstance<any>>;

    it('displays the label on the button', async () => {
        soroButton = await mountWithDefaults(SoroButton, {
            props: { label: 'some-label' },
        });

        expect(soroButton.text()).toBe('some-label');
    });

    it('re-bubbles the \'click\' event from the inner button', async () => {
        soroButton = await mountWithDefaults(SoroButton, {
            props: { label: 'some-label' },
        });

        await soroButton.find('button').trigger('click');

        expect(soroButton.emitted('click')).toHaveLength(1);
    });
});
