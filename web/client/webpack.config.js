const path = require('path');

module.exports = {
  watch: true,
  entry: './components/ordering_graph/index.ts',
  devtool: 'inline-source-map',
  module: {
    rules: [
      {
        test: /\.svg/,
        type: 'asset/inline'
      },
      {
        test: /\.(glsl|vs|fs)$/,
        use: 'ts-shader-loader',
        exclude: /node_modules/,
      },
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
};
