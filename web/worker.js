importScripts("viz.js");

onmessage = function (e) {
    var result = Viz(e.data.src, 'svg');
    postMessage(result);
}
