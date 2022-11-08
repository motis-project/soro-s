import {dsvFormat, tsvParse, csvParse} from "./d3-dsv.js";
function responseBlob(response) {
  if (!response.ok)
    throw new Error(response.status + " " + response.statusText);
  return response.blob();
}
function blob(input, init) {
  return fetch(input, init).then(responseBlob);
}
function responseArrayBuffer(response) {
  if (!response.ok)
    throw new Error(response.status + " " + response.statusText);
  return response.arrayBuffer();
}
function buffer(input, init) {
  return fetch(input, init).then(responseArrayBuffer);
}
function responseText(response) {
  if (!response.ok)
    throw new Error(response.status + " " + response.statusText);
  return response.text();
}
function text(input, init) {
  return fetch(input, init).then(responseText);
}
function dsvParse(parse) {
  return function(input, init, row) {
    if (arguments.length === 2 && typeof init === "function")
      row = init, init = void 0;
    return text(input, init).then(function(response) {
      return parse(response, row);
    });
  };
}
function dsv(delimiter, input, init, row) {
  if (arguments.length === 3 && typeof init === "function")
    row = init, init = void 0;
  var format = dsvFormat(delimiter);
  return text(input, init).then(function(response) {
    return format.parse(response, row);
  });
}
var csv = dsvParse(csvParse);
var tsv = dsvParse(tsvParse);
function image(input, init) {
  return new Promise(function(resolve, reject) {
    var image2 = new Image();
    for (var key in init)
      image2[key] = init[key];
    image2.onerror = reject;
    image2.onload = function() {
      resolve(image2);
    };
    image2.src = input;
  });
}
function responseJson(response) {
  if (!response.ok)
    throw new Error(response.status + " " + response.statusText);
  if (response.status === 204 || response.status === 205)
    return;
  return response.json();
}
function json(input, init) {
  return fetch(input, init).then(responseJson);
}
function parser(type) {
  return (input, init) => text(input, init).then((text2) => new DOMParser().parseFromString(text2, type));
}
var xml = parser("application/xml");
var html = parser("text/html");
var svg = parser("image/svg+xml");
export {blob, buffer, csv, dsv, html, image, json, svg, text, tsv, xml};
export default null;
