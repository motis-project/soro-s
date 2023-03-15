import { mountWithDefaults } from '@/test-utils/mount-with-defaults';
import { constructTestInjectComponent } from '@/test-utils/helper-components';
import InfrastructureComponent from '@/golden-layout/components/InfrastructureComponent.vue';

describe('InfrastructureComponent', async () => {
    it('provides its golden layout key from the props to its children via injection provision', async () => {
        const glComponent = await mountWithDefaults(InfrastructureComponent, {
            stubs: {
                'infrastructure-map': constructTestInjectComponent('goldenLayoutKeyInjection'),
            },
            props: { goldenLayoutKey: 'some-golden-layout-key' },
        });

        const testInjectComponent = glComponent.findComponent({ name: 'TestInjectComponent' });
        expect(testInjectComponent.text()).toBe('some-golden-layout-key');
    });
});
