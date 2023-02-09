import { ElementType, ElementTypes } from './elementTypes';
import { StyleSpecification } from 'maplibre-gl';
import { transformUrl } from '@/api/api-client';
import { ThemeDefinition } from 'vuetify';

export const createInfrastructureMapStyle = ({ currentTheme, activatedElements }: { currentTheme: ThemeDefinition, activatedElements: typeof ElementTypes }) => {
    const style: StyleSpecification = {
        version: 8,
        sources: {
            'osm': {
                'type': 'vector',
                'tiles': ['/tiles/{z}/{x}/{y}.mvt'],
                'maxzoom': 20
            },
            'station-routes': {
                'type': 'geojson',
                'data': {
                    type: 'FeatureCollection',
                    features: [],
                }
            },
            'signal-station-routes': {
                'type': 'geojson',
                'data': {
                    type: 'FeatureCollection',
                    features: [],
                }
            }
        },
        'glyphs': transformUrl('/font/{fontstack}/{range}.pbf'),
        'layers': [
            {
                'id': 'background',
                'type': 'background',
                'paint': {
                    'background-color': currentTheme.colors?.background,
                }
            },
            {
                'id': 'yards',
                'type': 'line',
                'source': 'osm',
                'source-layer': 'rail',
                'filter': ['==', 'rail', 'detail'],
                'paint': {
                    'line-color': '#ccc',
                    'line-width': 2.0
                }
            },
            {
                'id': 'rail',
                'type': 'line',
                'source': 'osm',
                'source-layer': 'rail',
                'filter': ['==', 'rail', 'primary'],
                'paint': {
                    'line-color': [
                        'case',
                        ['has', 'color'], ['get', 'color'],
                        '#444'
                    ],
                    'line-width': 2.0
                }
            },
            {
                'id': 'station-route-layer',
                'type': 'line',
                'source': 'station-routes',
                'layout': {
                    'line-join': 'round',
                    'line-cap': 'round'
                },
                'paint': {
                    'line-color': '#ff0000',
                    'line-width': 6
                }
            },
            {
                'id': 'station-route-element-layer',
                'type': 'circle',
                'source': 'station-routes',
                'minzoom': 16,
                'maxzoom': 24,
                'paint': {
                    'circle-color': '#ff0000',
                    'circle-radius': ['interpolate', ['linear'], ['zoom'], 10, 1, 20, 19]
                }
            },
            {
                'id': 'signal-station-route-layer',
                'type': 'line',
                'source': 'signal-station-routes',
                'layout': {
                    'line-join': 'round',
                    'line-cap': 'round'
                },
                'paint': {
                    'line-color': '#ff0000',
                    'line-width': 6
                }
            },
            {
                'id': 'signal-station-route-element-layer',
                'type': 'circle',
                'source': 'signal-station-routes',
                'minzoom': 16,
                'maxzoom': 24,
                'paint': {
                    'circle-color': '#ff0000',
                    'circle-radius': ['interpolate', ['linear'], ['zoom'], 10, 1, 20, 19]
                }
            },
        ]
    };

    ElementTypes.forEach(type => {
        if (type === ElementType.STATION) {
            style.layers.push({
                'id': type + '-layer',
                'source': 'osm',
                'source-layer': type,
                'type': 'symbol',
                'minzoom': 5,
                'maxzoom': 24,
                'paint': {
                    'icon-color': '#ffffff',
                    'text-halo-width': 1,
                    'text-halo-color': '#ffffff',
                },
                'layout': {
                    'visibility': (activatedElements.includes(type) ? 'visible' : 'none'),
                    'text-field': ['get', 'name'],
                    'text-anchor': 'top',
                    'text-offset': [0, 1],
                    'text-font': ['Noto Sans Display Bold'],
                    'icon-image': 'icon-' + type,
                    'icon-size': ['interpolate', ['linear'], ['zoom'], 10, 0.8, 20, 1.0]
                }
            });
        } else {
            // gives us the small black dots for icon stand-ins
            style.layers.push({
                'id': 'circle-' + type + '-layer',
                'source': 'osm',
                'source-layer': type,
                'type': 'circle',
                'minzoom': 13,
                'maxzoom': 24,
                'paint': {
                    'circle-radius': 3,
                    'circle-color': '#000000',
                    'circle-stroke-width': 1,
                    'circle-stroke-color': '#FFFFFF'
                },
                'layout': {
                    'visibility': (activatedElements.includes(type) ? 'visible' : 'none'),
                },
            });

            // gives us the elements as icons
            style.layers.push({
                'id': type + '-layer',
                'source': 'osm',
                'source-layer': type,
                'type': 'symbol',
                'minzoom': type === ElementType.HALT ? 10 : 15,
                'maxzoom': 24,
                'paint': {
                    'icon-color': '#ffffff',
                    'text-color': currentTheme.colors?.['on-surface'],
                },
                'layout': {
                    'visibility': (activatedElements.includes(type) ? 'visible' : 'none'),
                    'text-field': ['get', type === ElementType.HALT ? 'name' : 'id'],
                    'text-anchor': 'top',
                    'text-offset': [0, 1],
                    'text-font': ['Noto Sans Display Regular'],
                    'icon-image': 'icon-' + type,
                    'icon-size': ['interpolate', ['linear'], ['zoom'], 10, 0.2, 20, 0.4]
                },
                // "filter": ['==', 'direction', 'rising']
            });
        }
    });

    return style;
};
