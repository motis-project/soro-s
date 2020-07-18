importScripts("viz.js");

onmessage = function (e) {
    var result = Viz(e.data.src, {engine: 'dot', format: 'svg'});
    postMessage(result);
}
