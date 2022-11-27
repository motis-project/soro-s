import { elementTypes } from './elementTypes.js';

export const iconUrl = window.origin + '/icons/';
export const iconExtension = '.png';

export function addIcons(map) {
	for (const type of elementTypes) {
		const iconName = 'icon-' + type;

		map.loadImage(iconUrl + type + iconExtension, (error, image) => {
			if (error) {
				throw error;
			}

			map.addImage(iconName, image);
		});
	}
}
