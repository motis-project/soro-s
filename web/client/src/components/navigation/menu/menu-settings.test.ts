import { mountWithDefaults } from '@/test-utils/mount-with-defaults';
import { VueWrapper } from '@vue/test-utils';
import MenuSettings from './menu-settings.vue';
import { DarkLightModes, SettingsNamespace, SettingsState } from '@/stores/settings-store';

describe('menu-settings', async () => {
    let menuSettings: VueWrapper<any>;
    const settingsState: SettingsState = {
        darkLightModePreference: DarkLightModes.DARK,
        theme: DarkLightModes.DARK,
        primaryColor: null,
    };
    const settingsActions = {
        setDarkLightModePreference: vi.fn(),
        setPrimaryColor: vi.fn(),
    };

    const defaults = {
        store: {
            [SettingsNamespace]: {
                state: settingsState,
                actions: settingsActions,
            },
        },
    };

    beforeEach(async () => {
        vi.clearAllMocks();
        menuSettings = await mountWithDefaults(MenuSettings, defaults);
    });

    it('displays a button toggle that allows to set the dark light mode', async () => {
        const buttonToggle = menuSettings.findComponent({ name: 'v-btn-toggle' });

        expect(buttonToggle.exists()).toBe(true);
        const buttons = buttonToggle.findAllComponents({ name: 'v-btn' });
        expect(buttons).toHaveLength(3);
        expect(buttons[0].text()).toBe('OS');
        expect(buttons[0].vm.$props.value).toBe(DarkLightModes.OS);
        expect(buttons[1].text()).toBe('Light');
        expect(buttons[1].vm.$props.value).toBe(DarkLightModes.LIGHT);
        expect(buttons[2].text()).toBe('Dark');
        expect(buttons[2].vm.$props.value).toBe(DarkLightModes.DARK);
    });

    it('updates the dark light mode preference when the button toggle emits an \'update\'', async () => {
        const buttonToggle = menuSettings.findComponent({ name: 'v-btn-toggle' });

        buttonToggle.vm.$emit('update:modelValue', 'some-preference');

        expect(settingsActions.setDarkLightModePreference).toHaveBeenCalledWith(
            expect.any(Object),
            'some-preference',
        );
    });

    it('displays a button that can open the color picker', async () => {
        const colorPickerContainer = menuSettings.findComponent('.accent-color-picker');
        const toggleButton = colorPickerContainer.findComponent({ name: 'v-btn' });

        expect(colorPickerContainer.findComponent({ name: 'v-color-picker' }).exists()).toBe(false);
        await toggleButton.trigger('click');
        expect(colorPickerContainer.findComponent({ name: 'v-color-picker' }).exists()).toBe(true);
    });

    it('updates the color selection when the color picker emits an \'update\' event', async () => {
        const colorPickerContainer = menuSettings.findComponent('.accent-color-picker');
        await colorPickerContainer.findComponent({ name: 'v-btn' }).trigger('click');

        const colorPicker = colorPickerContainer.findComponent({ name: 'v-color-picker' });
        colorPicker.vm.$emit('update:modelValue', 'some-color');

        expect(settingsActions.setPrimaryColor).toHaveBeenCalledWith(
            expect.any(Object),
            'some-color',
        );
    });
});
