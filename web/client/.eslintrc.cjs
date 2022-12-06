module.exports = { // TODO add TS support in ESLint
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
	parserOptions: {
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
			'tab'
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
			'only-multiline',
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
			'tab',
		],
	}
};
