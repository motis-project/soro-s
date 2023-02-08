## SORO-S Webclient

### Tools:
- Node.js (preferably install via [nvm](https://github.com/nvm-sh/nvm))

### Setup:

Have nodejs installed with npm available globally (enabled by default). Execute the following in this directory:

```shell
npm install
```

### Development

```shell
npm run dev
```
You can access the client with hmr now on the port shown in the console (usually [5173](http://localhost:5173)).

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
