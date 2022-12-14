import { elementTypes } from './elementTypes.js';
import { Map } from 'maplibre-gl';

export const iconUrl = window.origin + '/icons/';
export const iconExtension = '.png';

export function addIcons(map: Map) {
	for (const type of elementTypes) {
		const iconName = 'icon-' + type;

		map.loadImage(iconUrl + type + iconExtension, (error, image) => {
			if (error || !image) {
				throw error ?? new Error('Image was not loaded but error was not thrown');
			}

			map.addImage(iconName, image);
		});
	}
}
