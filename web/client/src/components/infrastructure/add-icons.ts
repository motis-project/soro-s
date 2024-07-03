import { ElementType } from './element-types';
import { Map } from 'maplibre-gl';

export const iconUrl = '/icons/';
export const iconExtension = '.png';

export function getIconUrl(elementType: string): string {
  return iconUrl + elementType + iconExtension;
}

export function addIcons(map: Map) {
  return Promise.all(
    Object.values(ElementType).map((elementType) => {
      const iconName = 'icon-' + elementType;

      return new Promise<void>((resolve: () => void, reject) => {
        map.loadImage(getIconUrl(elementType), (error, image) => {
          if (error || !image) {
            reject(
              error ??
                new Error('Image was not loaded but error was not thrown')
            );

            return;
          }

          if (!map.hasImage(iconName)) {
            map.addImage(iconName, image);
          }

          resolve();
        });
      });
    })
  );
}
