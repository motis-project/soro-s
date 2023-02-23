module.exports = {
    root: true,
    env: {
        browser: true,
        es2021: true
    },
    extends: [
        'eslint:recommended',
        'plugin:vue/vue3-essential',
        'plugin:vue/vue3-recommended',
        '@vue/eslint-config-typescript',
        '@vue/eslint-config-typescript/recommended',
    ],
    overrides: [],
    parser: 'vue-eslint-parser',
    parserOptions: {
        parser: '@typescript-eslint/parser',
        ecmaVersion: 'latest',
        sourceType: 'module'
    },
    plugins: [
        'vue',
        '@typescript-eslint'
    ],
    rules: {
        'indent': [
            'error',
            4
        ],
        'curly': [
            'error',
            'all'
        ],
        'brace-style': [
            'error',
            '1tbs'
        ],
        'function-paren-newline': [
            'error',
            'multiline'
        ],
        'linebreak-style': [
            'error',
            'unix'
        ],
        'quotes': [
            'error',
            'single'
        ],
        'semi': [
            'error',
            'always'
        ],
        'comma-dangle': [
            'error',
            'always-multiline',
        ],
        'object-curly-spacing': [
            'error',
            'always',
        ],
        'object-shorthand': [
            'error',
            'properties',
            { avoidQuotes: true }
        ],
        'vue/html-indent': [
            'error',
            4,
        ],
        '@typescript-eslint/no-explicit-any': 0,
        '@typescript-eslint/ban-ts-comment': 0,
        '@typescript-eslint/no-unused-vars': ['error'],
        'no-console': ['error', { allow: ['warn', 'error'] }],
        'object-curly-newline': ['error', { multiline: true, consistent: true }],
        'object-property-newline': ['error'],
    }
};
