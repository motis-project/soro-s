importScripts("http://viz-js.com/bower_components/viz.js/viz.js");

onmessage = function (e) {
    var result = Viz(e.data.src, {engine: 'dot', format: 'svg'});
    postMessage(result);
}
