import { mountWithDefaults } from '@/test-utils/mount-with-defaults';
import { ComponentPublicInstance } from 'vue';
import { VueWrapper } from '@vue/test-utils';
import GoldenLayoutComponent from '@/golden-layout/golden-layout-component.vue';

describe('golden-layout-adapter', async () => {
    let glComponent: VueWrapper<ComponentPublicInstance<typeof GoldenLayoutComponent>>;

    beforeEach(async () => {
        glComponent = await mountWithDefaults(GoldenLayoutComponent);
        vi.clearAllMocks();
    });

    it('exposes a setter for position and size of its root dom element', async () => {
        glComponent.vm.setPosAndSize(10, 20, 30, 40);

        const componentDiv = glComponent.find({ ref: 'GLComponent' });
        expect((componentDiv.element as HTMLElement).style.left).toBe('10px');
        expect((componentDiv.element as HTMLElement).style.top).toBe('20px');
        expect((componentDiv.element as HTMLElement).style.width).toBe('30px');
        expect((componentDiv.element as HTMLElement).style.height).toBe('40px');
    });

    it.each([
        [true, ''],
        [false, 'none'],
    ])(
        'can set the visibility of its root dom via an exposed setter to %s',
        async (visibility: boolean, expectedDisplay: string) => {
            glComponent.vm.setVisibility(visibility);

            const componentDiv = glComponent.find({ ref: 'GLComponent' });
            expect((componentDiv.element as HTMLElement).style.display).toBe(expectedDisplay);
        },
    );

    it('exposes a setter for the z index of its root dom', async () => {
        glComponent.vm.setZIndex(true);

        const componentDiv = glComponent.find({ ref: 'GLComponent' });
        expect((componentDiv.element as HTMLElement).style.display).toBe('');
    });
});
