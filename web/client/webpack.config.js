const path = require('path');

module.exports = {
  watch:true,
  entry: './components/ordering_graph/sigma_graph.ts',
  devtool: 'inline-source-map',
  module: {
    rules:[
      {
        test: /\.tsx?$/,
        use: [{
          loader: 'expose-loader',
          options: {
           exposes: ['myNameSpace'],
          },
         }, {
           loader: 'ts-loader'
         }],
        exclude: /node_modules/,
      },
    ],
  },
  resolve: {
    extensions: ['.tsx', '.ts', '.js'],
  },
  output: {
    filename: 'main.js',
    path: path.resolve(__dirname, '/..', 'Users', 'toebn', 'source', 'repos', 'soro-s', 'build', 'msvc-release', 'server_resources', 'components', 'ordering_graph'),
  },
};