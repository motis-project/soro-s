# SORO-S Webclient

### Tools:
- Node.js (preferably install via [nvm](https://github.com/nvm-sh/nvm))

### Setup:

Have nodejs installed with npm available globally (enabled by default). Execute the following in this directory:

```shell
npm install
```

### Building the web interface for production
 
If you never ran ninja before, run it now in your `build/{clang,gcc}-release` directory.
```shell
ninja 
```

Then simply execute
```shell
ninja soro-client
```
This target has also been added to the `soro-server-client` target for convenience.

## Extending the Webclient

For a guide on how to configure `golden-layout`, please refer to its corresponding [README](src/golden-layout/README.md).

### Languages
You may need to re-familiarize yourself with the following languages/tools:
* [Vue](https://vuejs.org/)
* [Typescript](https://www.typescriptlang.org/)
* [Vuex](https://vuex.vuejs.org/)

You will also need a [node.js](https://nodejs.org/) installation (preferably with `nvm` as stated at the top of this file)
and work execute `npm install`

### Development
Run the following in this directory:
```shell
npm run dev
```
You can access the client with hot module reload now on the port shown in the console (usually [5173](http://localhost:5173)).
For a more detailed reference on how to hot module reload works and general vite options, please refer to [vite guide](https://vitejs.dev/guide/).

#### Testing
Run the following in this directory:
```shell
npm run test
```

This will execute all tests for the Vue components and the Vuex stores. There are a few more scripts given, such as
opening the test UI or computing coverage. For more details, please refer to the `scripts` section in the [package.json](package.json).

For more details on the used test API, please visit https://vitest.dev/ and https://test-utils.vuejs.org/.
