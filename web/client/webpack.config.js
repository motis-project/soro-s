const path = require('path');

module.exports = {
  watch: true,
  entry: './components/ordering_graph/index.ts',
  module: {
    rules: [
      {
        test: /\.tsx?$/,
        use: 'ts-loader',
        exclude: /node_modules/,
      },
    ],
  },
  resolve: {
    extensions: ['.tsx', '.ts', '.js'],
  },
  output: {
    filename: 'bundle.js',
    path: path.resolve(__dirname, 'components', 'ordering_graph'),
    library: {
      name: 'webpackSigmaGraph',
      type: 'umd',
    },
  },
  optimization: {
    chunkIds: 'total-size',
  },
};
