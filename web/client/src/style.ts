import 'material-design-icons-iconfont/dist/material-design-icons.css';
import 'vuetify/styles';
import './style/style.css';
import { ThemeDefinition } from 'vuetify';

// These are partial overrides of the light and dark themes provided by vuetify.
export const customLightTheme: ThemeDefinition = {
    colors: {
        primary: '#2196F3',
        goldenLayoutTabBackground: '#F4F4F4',
    },
    variables: {
        highEmphasisOpacity: 1,
        mediumEmphasisOpacity: 1,
    }
};

export const customDarkTheme: ThemeDefinition = {
    colors: {
        primary: '#2196F3',
    },
    variables: {
        highEmphasisOpacity: 1,
        mediumEmphasisOpacity: 1,
    }
};
