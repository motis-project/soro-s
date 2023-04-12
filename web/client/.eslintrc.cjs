module.exports = {
  root: true,
  env: {
    browser: true,
    es2021: true
  },
  extends: [
    "eslint:recommended",
    "plugin:vue/vue3-essential",
    "plugin:vue/vue3-recommended",
    "@vue/eslint-config-typescript",
    "@vue/eslint-config-typescript/recommended",
    "prettier"
  ],
  overrides: [],
  parser: "vue-eslint-parser",
  parserOptions: {
    parser: "@typescript-eslint/parser",
    ecmaVersion: "latest",
    sourceType: "module"
  },
  plugins: [
    "vue",
    "@typescript-eslint",
    "prettier"
  ],
  rules: {
    "@typescript-eslint/no-explicit-any": 0,
    "@typescript-eslint/ban-ts-comment": 0,
    "@typescript-eslint/no-unused-vars": ["error"],
    "no-console": ["error", { allow: ["warn", "error"] }],
    "object-curly-newline": ["error", { multiline: true, consistent: true }],
    "object-property-newline": ["error"]
  }
};
