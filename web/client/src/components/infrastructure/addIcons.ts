import { elementTypes } from './elementTypes.js';
import { Map } from 'maplibre-gl';
import { transformUrl } from '@/api/api-client';

export const iconUrl = transformUrl('/icons/');
export const iconExtension = '.png';

export function addIcons(map: Map) {
    return Promise.all(elementTypes.map((elementType) => {
        const iconName = 'icon-' + elementType;

        return new Promise<void>((resolve: () => void, reject) => {
            map.loadImage(iconUrl + elementType + iconExtension, (error, image) => {
                if (error || !image) {
                    reject(error ?? new Error('Image was not loaded but error was not thrown'));

                    return;
                }

                if (!map.hasImage(iconName)) {
                    map.addImage(iconName, image);
                }

                resolve();
            });
        });
    }));
}
