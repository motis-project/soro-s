import { mountWithDefaults } from '@/test-utils/mount-with-defaults';
import { VueWrapper } from '@vue/test-utils';
import InfrastructureLegend from '@/components/infrastructure/infrastructure-legend.vue';
import { SpecialLegendControls, SpecialLegendControl } from '@/components/infrastructure/infrastructure-legend.vue';
import { ComponentPublicInstance } from 'vue';
import { VCheckbox } from 'vuetify/components';
import { ElementType, ElementTypes } from '@/components/infrastructure/element-types';

describe('infrastructure-legend', async () => {
    let infrastructureLegend: VueWrapper<ComponentPublicInstance<any>>;

    beforeEach(async () => {
        infrastructureLegend = await mountWithDefaults(InfrastructureLegend, {
            props: {
                checkedControls: [],
            },
        });
    });

    it('displays a button to expand and collapse the legend', async () => {
        const collapseButton = infrastructureLegend.findComponent({ name: 'v-btn' });

        let mapLegend = infrastructureLegend.findComponent({ ref: 'mapLegend' });
        expect(mapLegend.classes()).not.toContain('infrastructure-map-legend-collapsed');

        await collapseButton.find('button').trigger('click');
        await infrastructureLegend.vm.$nextTick();

        mapLegend = infrastructureLegend.findComponent({ ref: 'mapLegend' });
        expect(mapLegend.classes()).toContain('infrastructure-map-legend-collapsed');

        await collapseButton.find('button').trigger('click');

        mapLegend = infrastructureLegend.findComponent({ ref: 'mapLegend' });
        expect(mapLegend.classes()).not.toContain('infrastructure-map-legend-collapsed');
    });

    it('displays a reset link that emits \'reset\' upon being clicked', async () => {
        const reset = infrastructureLegend.find<HTMLDivElement>('.infrastructure-map-reset');

        await reset.find('a').trigger('click');

        expect(infrastructureLegend.emitted('reset')).toHaveLength(1);
    });

    it.each(Array.of(
        ...ElementTypes,
        ...SpecialLegendControls,
    ))('displays a checkbox for the legend control %s', async (elementType: string) => {
        const mapLegend = infrastructureLegend.findComponent({ ref: 'mapLegend' });

        const checkbox = mapLegend.findComponent<VCheckbox>({ ref: elementType });
        expect(checkbox.exists());
    });

    it('emits a \'change\' event with that legend controls element type when a checkbox is checked', async () => {
        const mapLegend = infrastructureLegend.findComponent({ ref: 'mapLegend' });

        const checkbox = mapLegend.find<HTMLInputElement>('input[value="station"]');
        checkbox.element.checked = true;
        await checkbox.trigger('input');

        const changeEvents = infrastructureLegend.emitted('change');
        expect(changeEvents).toStrictEqual([['station', true]]);
    });

    it('emits an \'unchecked\' event with that legend controls element type when a checkbox is unchecked', async () => {
        const mapLegend = infrastructureLegend.findComponent({ ref: 'mapLegend' });

        const checkbox = mapLegend.find<HTMLInputElement>('input[value="station"]');
        checkbox.element.checked = false;
        await checkbox.trigger('input');

        const changeEvents = infrastructureLegend.emitted('change');
        expect(changeEvents).toStrictEqual([['station', false]]);
    });

    it('exports special legend controls available for other modules', async () => {
        expect(SpecialLegendControl.RISING).toBe('Rising');
        expect(SpecialLegendControl.FALLING).toBe('Falling');

        expect(SpecialLegendControls).toStrictEqual([
            'Rising',
            'Falling',
        ]);
    });

    it('initially checks the checkboxes given in the props', async () => {
        await infrastructureLegend.setProps({
            checkedControls: [
                ElementType.STATION,
                ElementType.HALT,
            ],
        });
        const mapLegend = infrastructureLegend.findComponent({ ref: 'mapLegend' });

        const stationCheckbox = mapLegend.find<HTMLInputElement>('input[value="station"]');
        expect(stationCheckbox.element.checked).toBe(true);
        const haltCheckbox = mapLegend.find<HTMLInputElement>('input[value="hlt"]');
        expect(haltCheckbox.element.checked).toBe(true);
        const bumperCheckbox = mapLegend.find<HTMLInputElement>('input[value="bumper"]');
        expect(bumperCheckbox.element.checked).toBe(false);
    });
});
