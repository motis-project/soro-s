function defaultSeparation(a2, b) {
  return a2.parent === b.parent ? 1 : 2;
}
function meanX(children) {
  return children.reduce(meanXReduce, 0) / children.length;
}
function meanXReduce(x, c2) {
  return x + c2.x;
}
function maxY(children) {
  return 1 + children.reduce(maxYReduce, 0);
}
function maxYReduce(y, c2) {
  return Math.max(y, c2.y);
}
function leafLeft(node) {
  var children;
  while (children = node.children)
    node = children[0];
  return node;
}
function leafRight(node) {
  var children;
  while (children = node.children)
    node = children[children.length - 1];
  return node;
}
function cluster() {
  var separation = defaultSeparation, dx = 1, dy = 1, nodeSize = false;
  function cluster2(root) {
    var previousNode, x = 0;
    root.eachAfter(function(node) {
      var children = node.children;
      if (children) {
        node.x = meanX(children);
        node.y = maxY(children);
      } else {
        node.x = previousNode ? x += separation(node, previousNode) : 0;
        node.y = 0;
        previousNode = node;
      }
    });
    var left = leafLeft(root), right = leafRight(root), x0 = left.x - separation(left, right) / 2, x1 = right.x + separation(right, left) / 2;
    return root.eachAfter(nodeSize ? function(node) {
      node.x = (node.x - root.x) * dx;
      node.y = (root.y - node.y) * dy;
    } : function(node) {
      node.x = (node.x - x0) / (x1 - x0) * dx;
      node.y = (1 - (root.y ? node.y / root.y : 1)) * dy;
    });
  }
  cluster2.separation = function(x) {
    return arguments.length ? (separation = x, cluster2) : separation;
  };
  cluster2.size = function(x) {
    return arguments.length ? (nodeSize = false, dx = +x[0], dy = +x[1], cluster2) : nodeSize ? null : [dx, dy];
  };
  cluster2.nodeSize = function(x) {
    return arguments.length ? (nodeSize = true, dx = +x[0], dy = +x[1], cluster2) : nodeSize ? [dx, dy] : null;
  };
  return cluster2;
}
function count(node) {
  var sum = 0, children = node.children, i = children && children.length;
  if (!i)
    sum = 1;
  else
    while (--i >= 0)
      sum += children[i].value;
  node.value = sum;
}
function node_count() {
  return this.eachAfter(count);
}
function node_each(callback, that) {
  let index2 = -1;
  for (const node of this) {
    callback.call(that, node, ++index2, this);
  }
  return this;
}
function node_eachBefore(callback, that) {
  var node = this, nodes = [node], children, i, index2 = -1;
  while (node = nodes.pop()) {
    callback.call(that, node, ++index2, this);
    if (children = node.children) {
      for (i = children.length - 1; i >= 0; --i) {
        nodes.push(children[i]);
      }
    }
  }
  return this;
}
function node_eachAfter(callback, that) {
  var node = this, nodes = [node], next = [], children, i, n, index2 = -1;
  while (node = nodes.pop()) {
    next.push(node);
    if (children = node.children) {
      for (i = 0, n = children.length; i < n; ++i) {
        nodes.push(children[i]);
      }
    }
  }
  while (node = next.pop()) {
    callback.call(that, node, ++index2, this);
  }
  return this;
}
function node_find(callback, that) {
  let index2 = -1;
  for (const node of this) {
    if (callback.call(that, node, ++index2, this)) {
      return node;
    }
  }
}
function node_sum(value) {
  return this.eachAfter(function(node) {
    var sum = +value(node.data) || 0, children = node.children, i = children && children.length;
    while (--i >= 0)
      sum += children[i].value;
    node.value = sum;
  });
}
function node_sort(compare) {
  return this.eachBefore(function(node) {
    if (node.children) {
      node.children.sort(compare);
    }
  });
}
function node_path(end) {
  var start = this, ancestor = leastCommonAncestor(start, end), nodes = [start];
  while (start !== ancestor) {
    start = start.parent;
    nodes.push(start);
  }
  var k = nodes.length;
  while (end !== ancestor) {
    nodes.splice(k, 0, end);
    end = end.parent;
  }
  return nodes;
}
function leastCommonAncestor(a2, b) {
  if (a2 === b)
    return a2;
  var aNodes = a2.ancestors(), bNodes = b.ancestors(), c2 = null;
  a2 = aNodes.pop();
  b = bNodes.pop();
  while (a2 === b) {
    c2 = a2;
    a2 = aNodes.pop();
    b = bNodes.pop();
  }
  return c2;
}
function node_ancestors() {
  var node = this, nodes = [node];
  while (node = node.parent) {
    nodes.push(node);
  }
  return nodes;
}
function node_descendants() {
  return Array.from(this);
}
function node_leaves() {
  var leaves = [];
  this.eachBefore(function(node) {
    if (!node.children) {
      leaves.push(node);
    }
  });
  return leaves;
}
function node_links() {
  var root = this, links = [];
  root.each(function(node) {
    if (node !== root) {
      links.push({source: node.parent, target: node});
    }
  });
  return links;
}
function* node_iterator() {
  var node = this, current, next = [node], children, i, n;
  do {
    current = next.reverse(), next = [];
    while (node = current.pop()) {
      yield node;
      if (children = node.children) {
        for (i = 0, n = children.length; i < n; ++i) {
          next.push(children[i]);
        }
      }
    }
  } while (next.length);
}
function hierarchy(data, children) {
  if (data instanceof Map) {
    data = [void 0, data];
    if (children === void 0)
      children = mapChildren;
  } else if (children === void 0) {
    children = objectChildren;
  }
  var root = new Node(data), node, nodes = [root], child, childs, i, n;
  while (node = nodes.pop()) {
    if ((childs = children(node.data)) && (n = (childs = Array.from(childs)).length)) {
      node.children = childs;
      for (i = n - 1; i >= 0; --i) {
        nodes.push(child = childs[i] = new Node(childs[i]));
        child.parent = node;
        child.depth = node.depth + 1;
      }
    }
  }
  return root.eachBefore(computeHeight);
}
function node_copy() {
  return hierarchy(this).eachBefore(copyData);
}
function objectChildren(d) {
  return d.children;
}
function mapChildren(d) {
  return Array.isArray(d) ? d[1] : null;
}
function copyData(node) {
  if (node.data.value !== void 0)
    node.value = node.data.value;
  node.data = node.data.data;
}
function computeHeight(node) {
  var height = 0;
  do
    node.height = height;
  while ((node = node.parent) && node.height < ++height);
}
function Node(data) {
  this.data = data;
  this.depth = this.height = 0;
  this.parent = null;
}
Node.prototype = hierarchy.prototype = {
  constructor: Node,
  count: node_count,
  each: node_each,
  eachAfter: node_eachAfter,
  eachBefore: node_eachBefore,
  find: node_find,
  sum: node_sum,
  sort: node_sort,
  path: node_path,
  ancestors: node_ancestors,
  descendants: node_descendants,
  leaves: node_leaves,
  links: node_links,
  copy: node_copy,
  [Symbol.iterator]: node_iterator
};
function optional(f) {
  return f == null ? null : required(f);
}
function required(f) {
  if (typeof f !== "function")
    throw new Error();
  return f;
}
function constantZero() {
  return 0;
}
function constant(x) {
  return function() {
    return x;
  };
}
const a = 1664525;
const c = 1013904223;
const m = 4294967296;
function lcg() {
  let s = 1;
  return () => (s = (a * s + c) % m) / m;
}
function array(x) {
  return typeof x === "object" && "length" in x ? x : Array.from(x);
}
function shuffle(array2, random) {
  let m2 = array2.length, t, i;
  while (m2) {
    i = random() * m2-- | 0;
    t = array2[m2];
    array2[m2] = array2[i];
    array2[i] = t;
  }
  return array2;
}
function enclose(circles) {
  return packEncloseRandom(circles, lcg());
}
function packEncloseRandom(circles, random) {
  var i = 0, n = (circles = shuffle(Array.from(circles), random)).length, B = [], p, e;
  while (i < n) {
    p = circles[i];
    if (e && enclosesWeak(e, p))
      ++i;
    else
      e = encloseBasis(B = extendBasis(B, p)), i = 0;
  }
  return e;
}
function extendBasis(B, p) {
  var i, j;
  if (enclosesWeakAll(p, B))
    return [p];
  for (i = 0; i < B.length; ++i) {
    if (enclosesNot(p, B[i]) && enclosesWeakAll(encloseBasis2(B[i], p), B)) {
      return [B[i], p];
    }
  }
  for (i = 0; i < B.length - 1; ++i) {
    for (j = i + 1; j < B.length; ++j) {
      if (enclosesNot(encloseBasis2(B[i], B[j]), p) && enclosesNot(encloseBasis2(B[i], p), B[j]) && enclosesNot(encloseBasis2(B[j], p), B[i]) && enclosesWeakAll(encloseBasis3(B[i], B[j], p), B)) {
        return [B[i], B[j], p];
      }
    }
  }
  throw new Error();
}
function enclosesNot(a2, b) {
  var dr = a2.r - b.r, dx = b.x - a2.x, dy = b.y - a2.y;
  return dr < 0 || dr * dr < dx * dx + dy * dy;
}
function enclosesWeak(a2, b) {
  var dr = a2.r - b.r + Math.max(a2.r, b.r, 1) * 1e-9, dx = b.x - a2.x, dy = b.y - a2.y;
  return dr > 0 && dr * dr > dx * dx + dy * dy;
}
function enclosesWeakAll(a2, B) {
  for (var i = 0; i < B.length; ++i) {
    if (!enclosesWeak(a2, B[i])) {
      return false;
    }
  }
  return true;
}
function encloseBasis(B) {
  switch (B.length) {
    case 1:
      return encloseBasis1(B[0]);
    case 2:
      return encloseBasis2(B[0], B[1]);
    case 3:
      return encloseBasis3(B[0], B[1], B[2]);
  }
}
function encloseBasis1(a2) {
  return {
    x: a2.x,
    y: a2.y,
    r: a2.r
  };
}
function encloseBasis2(a2, b) {
  var x1 = a2.x, y1 = a2.y, r1 = a2.r, x2 = b.x, y2 = b.y, r2 = b.r, x21 = x2 - x1, y21 = y2 - y1, r21 = r2 - r1, l = Math.sqrt(x21 * x21 + y21 * y21);
  return {
    x: (x1 + x2 + x21 / l * r21) / 2,
    y: (y1 + y2 + y21 / l * r21) / 2,
    r: (l + r1 + r2) / 2
  };
}
function encloseBasis3(a2, b, c2) {
  var x1 = a2.x, y1 = a2.y, r1 = a2.r, x2 = b.x, y2 = b.y, r2 = b.r, x3 = c2.x, y3 = c2.y, r3 = c2.r, a22 = x1 - x2, a3 = x1 - x3, b2 = y1 - y2, b3 = y1 - y3, c22 = r2 - r1, c3 = r3 - r1, d1 = x1 * x1 + y1 * y1 - r1 * r1, d2 = d1 - x2 * x2 - y2 * y2 + r2 * r2, d3 = d1 - x3 * x3 - y3 * y3 + r3 * r3, ab = a3 * b2 - a22 * b3, xa = (b2 * d3 - b3 * d2) / (ab * 2) - x1, xb = (b3 * c22 - b2 * c3) / ab, ya = (a3 * d2 - a22 * d3) / (ab * 2) - y1, yb = (a22 * c3 - a3 * c22) / ab, A = xb * xb + yb * yb - 1, B = 2 * (r1 + xa * xb + ya * yb), C = xa * xa + ya * ya - r1 * r1, r = -(Math.abs(A) > 1e-6 ? (B + Math.sqrt(B * B - 4 * A * C)) / (2 * A) : C / B);
  return {
    x: x1 + xa + xb * r,
    y: y1 + ya + yb * r,
    r
  };
}
function place(b, a2, c2) {
  var dx = b.x - a2.x, x, a22, dy = b.y - a2.y, y, b2, d2 = dx * dx + dy * dy;
  if (d2) {
    a22 = a2.r + c2.r, a22 *= a22;
    b2 = b.r + c2.r, b2 *= b2;
    if (a22 > b2) {
      x = (d2 + b2 - a22) / (2 * d2);
      y = Math.sqrt(Math.max(0, b2 / d2 - x * x));
      c2.x = b.x - x * dx - y * dy;
      c2.y = b.y - x * dy + y * dx;
    } else {
      x = (d2 + a22 - b2) / (2 * d2);
      y = Math.sqrt(Math.max(0, a22 / d2 - x * x));
      c2.x = a2.x + x * dx - y * dy;
      c2.y = a2.y + x * dy + y * dx;
    }
  } else {
    c2.x = a2.x + c2.r;
    c2.y = a2.y;
  }
}
function intersects(a2, b) {
  var dr = a2.r + b.r - 1e-6, dx = b.x - a2.x, dy = b.y - a2.y;
  return dr > 0 && dr * dr > dx * dx + dy * dy;
}
function score(node) {
  var a2 = node._, b = node.next._, ab = a2.r + b.r, dx = (a2.x * b.r + b.x * a2.r) / ab, dy = (a2.y * b.r + b.y * a2.r) / ab;
  return dx * dx + dy * dy;
}
function Node$1(circle) {
  this._ = circle;
  this.next = null;
  this.previous = null;
}
function packSiblingsRandom(circles, random) {
  if (!(n = (circles = array(circles)).length))
    return 0;
  var a2, b, c2, n, aa, ca, i, j, k, sj, sk;
  a2 = circles[0], a2.x = 0, a2.y = 0;
  if (!(n > 1))
    return a2.r;
  b = circles[1], a2.x = -b.r, b.x = a2.r, b.y = 0;
  if (!(n > 2))
    return a2.r + b.r;
  place(b, a2, c2 = circles[2]);
  a2 = new Node$1(a2), b = new Node$1(b), c2 = new Node$1(c2);
  a2.next = c2.previous = b;
  b.next = a2.previous = c2;
  c2.next = b.previous = a2;
  pack:
    for (i = 3; i < n; ++i) {
      place(a2._, b._, c2 = circles[i]), c2 = new Node$1(c2);
      j = b.next, k = a2.previous, sj = b._.r, sk = a2._.r;
      do {
        if (sj <= sk) {
          if (intersects(j._, c2._)) {
            b = j, a2.next = b, b.previous = a2, --i;
            continue pack;
          }
          sj += j._.r, j = j.next;
        } else {
          if (intersects(k._, c2._)) {
            a2 = k, a2.next = b, b.previous = a2, --i;
            continue pack;
          }
          sk += k._.r, k = k.previous;
        }
      } while (j !== k.next);
      c2.previous = a2, c2.next = b, a2.next = b.previous = b = c2;
      aa = score(a2);
      while ((c2 = c2.next) !== b) {
        if ((ca = score(c2)) < aa) {
          a2 = c2, aa = ca;
        }
      }
      b = a2.next;
    }
  a2 = [b._], c2 = b;
  while ((c2 = c2.next) !== b)
    a2.push(c2._);
  c2 = packEncloseRandom(a2, random);
  for (i = 0; i < n; ++i)
    a2 = circles[i], a2.x -= c2.x, a2.y -= c2.y;
  return c2.r;
}
function siblings(circles) {
  packSiblingsRandom(circles, lcg());
  return circles;
}
function defaultRadius(d) {
  return Math.sqrt(d.value);
}
function index() {
  var radius = null, dx = 1, dy = 1, padding = constantZero;
  function pack(root) {
    const random = lcg();
    root.x = dx / 2, root.y = dy / 2;
    if (radius) {
      root.eachBefore(radiusLeaf(radius)).eachAfter(packChildrenRandom(padding, 0.5, random)).eachBefore(translateChild(1));
    } else {
      root.eachBefore(radiusLeaf(defaultRadius)).eachAfter(packChildrenRandom(constantZero, 1, random)).eachAfter(packChildrenRandom(padding, root.r / Math.min(dx, dy), random)).eachBefore(translateChild(Math.min(dx, dy) / (2 * root.r)));
    }
    return root;
  }
  pack.radius = function(x) {
    return arguments.length ? (radius = optional(x), pack) : radius;
  };
  pack.size = function(x) {
    return arguments.length ? (dx = +x[0], dy = +x[1], pack) : [dx, dy];
  };
  pack.padding = function(x) {
    return arguments.length ? (padding = typeof x === "function" ? x : constant(+x), pack) : padding;
  };
  return pack;
}
function radiusLeaf(radius) {
  return function(node) {
    if (!node.children) {
      node.r = Math.max(0, +radius(node) || 0);
    }
  };
}
function packChildrenRandom(padding, k, random) {
  return function(node) {
    if (children = node.children) {
      var children, i, n = children.length, r = padding(node) * k || 0, e;
      if (r)
        for (i = 0; i < n; ++i)
          children[i].r += r;
      e = packSiblingsRandom(children, random);
      if (r)
        for (i = 0; i < n; ++i)
          children[i].r -= r;
      node.r = e + r;
    }
  };
}
function translateChild(k) {
  return function(node) {
    var parent = node.parent;
    node.r *= k;
    if (parent) {
      node.x = parent.x + k * node.x;
      node.y = parent.y + k * node.y;
    }
  };
}
function roundNode(node) {
  node.x0 = Math.round(node.x0);
  node.y0 = Math.round(node.y0);
  node.x1 = Math.round(node.x1);
  node.y1 = Math.round(node.y1);
}
function treemapDice(parent, x0, y0, x1, y1) {
  var nodes = parent.children, node, i = -1, n = nodes.length, k = parent.value && (x1 - x0) / parent.value;
  while (++i < n) {
    node = nodes[i], node.y0 = y0, node.y1 = y1;
    node.x0 = x0, node.x1 = x0 += node.value * k;
  }
}
function partition() {
  var dx = 1, dy = 1, padding = 0, round = false;
  function partition2(root) {
    var n = root.height + 1;
    root.x0 = root.y0 = padding;
    root.x1 = dx;
    root.y1 = dy / n;
    root.eachBefore(positionNode(dy, n));
    if (round)
      root.eachBefore(roundNode);
    return root;
  }
  function positionNode(dy2, n) {
    return function(node) {
      if (node.children) {
        treemapDice(node, node.x0, dy2 * (node.depth + 1) / n, node.x1, dy2 * (node.depth + 2) / n);
      }
      var x0 = node.x0, y0 = node.y0, x1 = node.x1 - padding, y1 = node.y1 - padding;
      if (x1 < x0)
        x0 = x1 = (x0 + x1) / 2;
      if (y1 < y0)
        y0 = y1 = (y0 + y1) / 2;
      node.x0 = x0;
      node.y0 = y0;
      node.x1 = x1;
      node.y1 = y1;
    };
  }
  partition2.round = function(x) {
    return arguments.length ? (round = !!x, partition2) : round;
  };
  partition2.size = function(x) {
    return arguments.length ? (dx = +x[0], dy = +x[1], partition2) : [dx, dy];
  };
  partition2.padding = function(x) {
    return arguments.length ? (padding = +x, partition2) : padding;
  };
  return partition2;
}
var preroot = {depth: -1}, ambiguous = {}, imputed = {};
function defaultId(d) {
  return d.id;
}
function defaultParentId(d) {
  return d.parentId;
}
function stratify() {
  var id = defaultId, parentId = defaultParentId, path;
  function stratify2(data) {
    var nodes = Array.from(data), currentId = id, currentParentId = parentId, n, d, i, root, parent, node, nodeId, nodeKey, nodeByKey = new Map();
    if (path != null) {
      const I = nodes.map((d2, i2) => normalize(path(d2, i2, data)));
      const P = I.map(parentof);
      const S = new Set(I).add("");
      for (const i2 of P) {
        if (!S.has(i2)) {
          S.add(i2);
          I.push(i2);
          P.push(parentof(i2));
          nodes.push(imputed);
        }
      }
      currentId = (_, i2) => I[i2];
      currentParentId = (_, i2) => P[i2];
    }
    for (i = 0, n = nodes.length; i < n; ++i) {
      d = nodes[i], node = nodes[i] = new Node(d);
      if ((nodeId = currentId(d, i, data)) != null && (nodeId += "")) {
        nodeKey = node.id = nodeId;
        nodeByKey.set(nodeKey, nodeByKey.has(nodeKey) ? ambiguous : node);
      }
      if ((nodeId = currentParentId(d, i, data)) != null && (nodeId += "")) {
        node.parent = nodeId;
      }
    }
    for (i = 0; i < n; ++i) {
      node = nodes[i];
      if (nodeId = node.parent) {
        parent = nodeByKey.get(nodeId);
        if (!parent)
          throw new Error("missing: " + nodeId);
        if (parent === ambiguous)
          throw new Error("ambiguous: " + nodeId);
        if (parent.children)
          parent.children.push(node);
        else
          parent.children = [node];
        node.parent = parent;
      } else {
        if (root)
          throw new Error("multiple roots");
        root = node;
      }
    }
    if (!root)
      throw new Error("no root");
    if (path != null) {
      while (root.data === imputed && root.children.length === 1) {
        root = root.children[0], --n;
      }
      for (let i2 = nodes.length - 1; i2 >= 0; --i2) {
        node = nodes[i2];
        if (node.data !== imputed)
          break;
        node.data = null;
      }
    }
    root.parent = preroot;
    root.eachBefore(function(node2) {
      node2.depth = node2.parent.depth + 1;
      --n;
    }).eachBefore(computeHeight);
    root.parent = null;
    if (n > 0)
      throw new Error("cycle");
    return root;
  }
  stratify2.id = function(x) {
    return arguments.length ? (id = optional(x), stratify2) : id;
  };
  stratify2.parentId = function(x) {
    return arguments.length ? (parentId = optional(x), stratify2) : parentId;
  };
  stratify2.path = function(x) {
    return arguments.length ? (path = optional(x), stratify2) : path;
  };
  return stratify2;
}
function normalize(path) {
  path = `${path}`;
  let i = path.length;
  if (slash(path, i - 1) && !slash(path, i - 2))
    path = path.slice(0, -1);
  return path[0] === "/" ? path : `/${path}`;
}
function parentof(path) {
  let i = path.length;
  if (i < 2)
    return "";
  while (--i > 1)
    if (slash(path, i))
      break;
  return path.slice(0, i);
}
function slash(path, i) {
  if (path[i] === "/") {
    let k = 0;
    while (i > 0 && path[--i] === "\\")
      ++k;
    if ((k & 1) === 0)
      return true;
  }
  return false;
}
function defaultSeparation$1(a2, b) {
  return a2.parent === b.parent ? 1 : 2;
}
function nextLeft(v) {
  var children = v.children;
  return children ? children[0] : v.t;
}
function nextRight(v) {
  var children = v.children;
  return children ? children[children.length - 1] : v.t;
}
function moveSubtree(wm, wp, shift) {
  var change = shift / (wp.i - wm.i);
  wp.c -= change;
  wp.s += shift;
  wm.c += change;
  wp.z += shift;
  wp.m += shift;
}
function executeShifts(v) {
  var shift = 0, change = 0, children = v.children, i = children.length, w;
  while (--i >= 0) {
    w = children[i];
    w.z += shift;
    w.m += shift;
    shift += w.s + (change += w.c);
  }
}
function nextAncestor(vim, v, ancestor) {
  return vim.a.parent === v.parent ? vim.a : ancestor;
}
function TreeNode(node, i) {
  this._ = node;
  this.parent = null;
  this.children = null;
  this.A = null;
  this.a = this;
  this.z = 0;
  this.m = 0;
  this.c = 0;
  this.s = 0;
  this.t = null;
  this.i = i;
}
TreeNode.prototype = Object.create(Node.prototype);
function treeRoot(root) {
  var tree2 = new TreeNode(root, 0), node, nodes = [tree2], child, children, i, n;
  while (node = nodes.pop()) {
    if (children = node._.children) {
      node.children = new Array(n = children.length);
      for (i = n - 1; i >= 0; --i) {
        nodes.push(child = node.children[i] = new TreeNode(children[i], i));
        child.parent = node;
      }
    }
  }
  (tree2.parent = new TreeNode(null, 0)).children = [tree2];
  return tree2;
}
function tree() {
  var separation = defaultSeparation$1, dx = 1, dy = 1, nodeSize = null;
  function tree2(root) {
    var t = treeRoot(root);
    t.eachAfter(firstWalk), t.parent.m = -t.z;
    t.eachBefore(secondWalk);
    if (nodeSize)
      root.eachBefore(sizeNode);
    else {
      var left = root, right = root, bottom = root;
      root.eachBefore(function(node) {
        if (node.x < left.x)
          left = node;
        if (node.x > right.x)
          right = node;
        if (node.depth > bottom.depth)
          bottom = node;
      });
      var s = left === right ? 1 : separation(left, right) / 2, tx = s - left.x, kx = dx / (right.x + s + tx), ky = dy / (bottom.depth || 1);
      root.eachBefore(function(node) {
        node.x = (node.x + tx) * kx;
        node.y = node.depth * ky;
      });
    }
    return root;
  }
  function firstWalk(v) {
    var children = v.children, siblings2 = v.parent.children, w = v.i ? siblings2[v.i - 1] : null;
    if (children) {
      executeShifts(v);
      var midpoint = (children[0].z + children[children.length - 1].z) / 2;
      if (w) {
        v.z = w.z + separation(v._, w._);
        v.m = v.z - midpoint;
      } else {
        v.z = midpoint;
      }
    } else if (w) {
      v.z = w.z + separation(v._, w._);
    }
    v.parent.A = apportion(v, w, v.parent.A || siblings2[0]);
  }
  function secondWalk(v) {
    v._.x = v.z + v.parent.m;
    v.m += v.parent.m;
  }
  function apportion(v, w, ancestor) {
    if (w) {
      var vip = v, vop = v, vim = w, vom = vip.parent.children[0], sip = vip.m, sop = vop.m, sim = vim.m, som = vom.m, shift;
      while (vim = nextRight(vim), vip = nextLeft(vip), vim && vip) {
        vom = nextLeft(vom);
        vop = nextRight(vop);
        vop.a = v;
        shift = vim.z + sim - vip.z - sip + separation(vim._, vip._);
        if (shift > 0) {
          moveSubtree(nextAncestor(vim, v, ancestor), v, shift);
          sip += shift;
          sop += shift;
        }
        sim += vim.m;
        sip += vip.m;
        som += vom.m;
        sop += vop.m;
      }
      if (vim && !nextRight(vop)) {
        vop.t = vim;
        vop.m += sim - sop;
      }
      if (vip && !nextLeft(vom)) {
        vom.t = vip;
        vom.m += sip - som;
        ancestor = v;
      }
    }
    return ancestor;
  }
  function sizeNode(node) {
    node.x *= dx;
    node.y = node.depth * dy;
  }
  tree2.separation = function(x) {
    return arguments.length ? (separation = x, tree2) : separation;
  };
  tree2.size = function(x) {
    return arguments.length ? (nodeSize = false, dx = +x[0], dy = +x[1], tree2) : nodeSize ? null : [dx, dy];
  };
  tree2.nodeSize = function(x) {
    return arguments.length ? (nodeSize = true, dx = +x[0], dy = +x[1], tree2) : nodeSize ? [dx, dy] : null;
  };
  return tree2;
}
function treemapSlice(parent, x0, y0, x1, y1) {
  var nodes = parent.children, node, i = -1, n = nodes.length, k = parent.value && (y1 - y0) / parent.value;
  while (++i < n) {
    node = nodes[i], node.x0 = x0, node.x1 = x1;
    node.y0 = y0, node.y1 = y0 += node.value * k;
  }
}
var phi = (1 + Math.sqrt(5)) / 2;
function squarifyRatio(ratio, parent, x0, y0, x1, y1) {
  var rows = [], nodes = parent.children, row, nodeValue, i0 = 0, i1 = 0, n = nodes.length, dx, dy, value = parent.value, sumValue, minValue, maxValue, newRatio, minRatio, alpha, beta;
  while (i0 < n) {
    dx = x1 - x0, dy = y1 - y0;
    do
      sumValue = nodes[i1++].value;
    while (!sumValue && i1 < n);
    minValue = maxValue = sumValue;
    alpha = Math.max(dy / dx, dx / dy) / (value * ratio);
    beta = sumValue * sumValue * alpha;
    minRatio = Math.max(maxValue / beta, beta / minValue);
    for (; i1 < n; ++i1) {
      sumValue += nodeValue = nodes[i1].value;
      if (nodeValue < minValue)
        minValue = nodeValue;
      if (nodeValue > maxValue)
        maxValue = nodeValue;
      beta = sumValue * sumValue * alpha;
      newRatio = Math.max(maxValue / beta, beta / minValue);
      if (newRatio > minRatio) {
        sumValue -= nodeValue;
        break;
      }
      minRatio = newRatio;
    }
    rows.push(row = {value: sumValue, dice: dx < dy, children: nodes.slice(i0, i1)});
    if (row.dice)
      treemapDice(row, x0, y0, x1, value ? y0 += dy * sumValue / value : y1);
    else
      treemapSlice(row, x0, y0, value ? x0 += dx * sumValue / value : x1, y1);
    value -= sumValue, i0 = i1;
  }
  return rows;
}
var squarify = function custom(ratio) {
  function squarify2(parent, x0, y0, x1, y1) {
    squarifyRatio(ratio, parent, x0, y0, x1, y1);
  }
  squarify2.ratio = function(x) {
    return custom((x = +x) > 1 ? x : 1);
  };
  return squarify2;
}(phi);
function index$1() {
  var tile = squarify, round = false, dx = 1, dy = 1, paddingStack = [0], paddingInner = constantZero, paddingTop = constantZero, paddingRight = constantZero, paddingBottom = constantZero, paddingLeft = constantZero;
  function treemap(root) {
    root.x0 = root.y0 = 0;
    root.x1 = dx;
    root.y1 = dy;
    root.eachBefore(positionNode);
    paddingStack = [0];
    if (round)
      root.eachBefore(roundNode);
    return root;
  }
  function positionNode(node) {
    var p = paddingStack[node.depth], x0 = node.x0 + p, y0 = node.y0 + p, x1 = node.x1 - p, y1 = node.y1 - p;
    if (x1 < x0)
      x0 = x1 = (x0 + x1) / 2;
    if (y1 < y0)
      y0 = y1 = (y0 + y1) / 2;
    node.x0 = x0;
    node.y0 = y0;
    node.x1 = x1;
    node.y1 = y1;
    if (node.children) {
      p = paddingStack[node.depth + 1] = paddingInner(node) / 2;
      x0 += paddingLeft(node) - p;
      y0 += paddingTop(node) - p;
      x1 -= paddingRight(node) - p;
      y1 -= paddingBottom(node) - p;
      if (x1 < x0)
        x0 = x1 = (x0 + x1) / 2;
      if (y1 < y0)
        y0 = y1 = (y0 + y1) / 2;
      tile(node, x0, y0, x1, y1);
    }
  }
  treemap.round = function(x) {
    return arguments.length ? (round = !!x, treemap) : round;
  };
  treemap.size = function(x) {
    return arguments.length ? (dx = +x[0], dy = +x[1], treemap) : [dx, dy];
  };
  treemap.tile = function(x) {
    return arguments.length ? (tile = required(x), treemap) : tile;
  };
  treemap.padding = function(x) {
    return arguments.length ? treemap.paddingInner(x).paddingOuter(x) : treemap.paddingInner();
  };
  treemap.paddingInner = function(x) {
    return arguments.length ? (paddingInner = typeof x === "function" ? x : constant(+x), treemap) : paddingInner;
  };
  treemap.paddingOuter = function(x) {
    return arguments.length ? treemap.paddingTop(x).paddingRight(x).paddingBottom(x).paddingLeft(x) : treemap.paddingTop();
  };
  treemap.paddingTop = function(x) {
    return arguments.length ? (paddingTop = typeof x === "function" ? x : constant(+x), treemap) : paddingTop;
  };
  treemap.paddingRight = function(x) {
    return arguments.length ? (paddingRight = typeof x === "function" ? x : constant(+x), treemap) : paddingRight;
  };
  treemap.paddingBottom = function(x) {
    return arguments.length ? (paddingBottom = typeof x === "function" ? x : constant(+x), treemap) : paddingBottom;
  };
  treemap.paddingLeft = function(x) {
    return arguments.length ? (paddingLeft = typeof x === "function" ? x : constant(+x), treemap) : paddingLeft;
  };
  return treemap;
}
function binary(parent, x0, y0, x1, y1) {
  var nodes = parent.children, i, n = nodes.length, sum, sums = new Array(n + 1);
  for (sums[0] = sum = i = 0; i < n; ++i) {
    sums[i + 1] = sum += nodes[i].value;
  }
  partition2(0, n, parent.value, x0, y0, x1, y1);
  function partition2(i2, j, value, x02, y02, x12, y12) {
    if (i2 >= j - 1) {
      var node = nodes[i2];
      node.x0 = x02, node.y0 = y02;
      node.x1 = x12, node.y1 = y12;
      return;
    }
    var valueOffset = sums[i2], valueTarget = value / 2 + valueOffset, k = i2 + 1, hi = j - 1;
    while (k < hi) {
      var mid = k + hi >>> 1;
      if (sums[mid] < valueTarget)
        k = mid + 1;
      else
        hi = mid;
    }
    if (valueTarget - sums[k - 1] < sums[k] - valueTarget && i2 + 1 < k)
      --k;
    var valueLeft = sums[k] - valueOffset, valueRight = value - valueLeft;
    if (x12 - x02 > y12 - y02) {
      var xk = value ? (x02 * valueRight + x12 * valueLeft) / value : x12;
      partition2(i2, k, valueLeft, x02, y02, xk, y12);
      partition2(k, j, valueRight, xk, y02, x12, y12);
    } else {
      var yk = value ? (y02 * valueRight + y12 * valueLeft) / value : y12;
      partition2(i2, k, valueLeft, x02, y02, x12, yk);
      partition2(k, j, valueRight, x02, yk, x12, y12);
    }
  }
}
function sliceDice(parent, x0, y0, x1, y1) {
  (parent.depth & 1 ? treemapSlice : treemapDice)(parent, x0, y0, x1, y1);
}
var resquarify = function custom2(ratio) {
  function resquarify2(parent, x0, y0, x1, y1) {
    if ((rows = parent._squarify) && rows.ratio === ratio) {
      var rows, row, nodes, i, j = -1, n, m2 = rows.length, value = parent.value;
      while (++j < m2) {
        row = rows[j], nodes = row.children;
        for (i = row.value = 0, n = nodes.length; i < n; ++i)
          row.value += nodes[i].value;
        if (row.dice)
          treemapDice(row, x0, y0, x1, value ? y0 += (y1 - y0) * row.value / value : y1);
        else
          treemapSlice(row, x0, y0, value ? x0 += (x1 - x0) * row.value / value : x1, y1);
        value -= row.value;
      }
    } else {
      parent._squarify = rows = squarifyRatio(ratio, parent, x0, y0, x1, y1);
      rows.ratio = ratio;
    }
  }
  resquarify2.ratio = function(x) {
    return custom2((x = +x) > 1 ? x : 1);
  };
  return resquarify2;
}(phi);
export {Node, cluster, hierarchy, index as pack, enclose as packEnclose, siblings as packSiblings, partition, stratify, tree, index$1 as treemap, binary as treemapBinary, treemapDice, resquarify as treemapResquarify, treemapSlice, sliceDice as treemapSliceDice, squarify as treemapSquarify};
export default null;
