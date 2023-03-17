import { transformUrl } from '@/api/api-client';
import { iconUrl, iconExtension, addIcons } from '@/components/infrastructure/add-icons';
import { ElementTypes } from '@/components/infrastructure/element-types';

vi.mock('./elementTypes', () => ({
    ElementTypes: [
        'some-first-element-type',
        'some-second-element-type',
        'some-third-element-type',
    ],
}));

describe('add-icons', async () => {
    const map: any = {
        loadImage: vi.fn(),
        hasImage: vi.fn(),
        addImage: vi.fn(),
    };

    beforeEach(async () => {
        map.loadImage.mockImplementation((url: string, callback: (error: string, image: any) => void) => callback('', {}));
        map.hasImage.mockImplementation(() => false);
        map.addImage.mockImplementation(() => ({}));

        vi.clearAllMocks();
    });

    it('uses the url transform to generate the icons url', async () => {
        expect(iconUrl).toBe(transformUrl('/icons/'));
    });

    describe('addIcons()', async () => {
        it.each(ElementTypes)('returns a promise that loads the icon for %s into the map', async (elementType: string) => {
            map.loadImage.mockImplementation((url: string, callback: (error: string, image: any) => void) => callback(
                '',
                {
                    some: 'image',
                    foo: 'bar',
                },
            ));
            map.hasImage.mockImplementation(() => false);

            await addIcons(map);

            expect(map.loadImage).toHaveBeenCalledWith(
                iconUrl + elementType + iconExtension,
                expect.any(Function),
            );
            expect(map.addImage).toHaveBeenCalledWith(
                `icon-${elementType}`,
                {
                    some: 'image',
                    foo: 'bar',
                },
            );
        });

        it('does not load an icon into the map when the map already has that icon', async () => {
            map.hasImage.mockImplementation(() => true);

            await addIcons(map);

            expect(map.addImage).not.toHaveBeenCalled();
        });

        it('rejects the promise with the given error when an error is given to the callback from \'loadImage\'', async () => {
            map.loadImage.mockImplementation((url: string, callback: (error: string, image: any) => void) => callback(
                'some-dubious-error',
                { some: 'error-image' },
            ));

            await expect(addIcons(map)).rejects.toThrow('some-dubious-error');
        });

        it('rejects the promise with default error when no error is given to the callback from \'loadImage\' but no image was loaded', async () => {
            map.loadImage.mockImplementation((url: string, callback: (error: string | undefined, image: any) => void) => callback(
                undefined,
                undefined,
            ));

            await expect(addIcons(map)).rejects.toThrow('Image was not loaded but error was not thrown');
        });
    });
});
