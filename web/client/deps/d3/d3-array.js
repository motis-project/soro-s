import {InternMap, InternSet} from "./internmap.js";
export {InternMap, InternSet} from "./internmap.js";
function ascending(a, b) {
  return a == null || b == null ? NaN : a < b ? -1 : a > b ? 1 : a >= b ? 0 : NaN;
}
function descending(a, b) {
  return a == null || b == null ? NaN : b < a ? -1 : b > a ? 1 : b >= a ? 0 : NaN;
}
function bisector(f) {
  let compare1, compare2, delta;
  if (f.length !== 2) {
    compare1 = ascending;
    compare2 = (d, x) => ascending(f(d), x);
    delta = (d, x) => f(d) - x;
  } else {
    compare1 = f === ascending || f === descending ? f : zero;
    compare2 = f;
    delta = f;
  }
  function left(a, x, lo = 0, hi = a.length) {
    if (lo < hi) {
      if (compare1(x, x) !== 0)
        return hi;
      do {
        const mid = lo + hi >>> 1;
        if (compare2(a[mid], x) < 0)
          lo = mid + 1;
        else
          hi = mid;
      } while (lo < hi);
    }
    return lo;
  }
  function right(a, x, lo = 0, hi = a.length) {
    if (lo < hi) {
      if (compare1(x, x) !== 0)
        return hi;
      do {
        const mid = lo + hi >>> 1;
        if (compare2(a[mid], x) <= 0)
          lo = mid + 1;
        else
          hi = mid;
      } while (lo < hi);
    }
    return lo;
  }
  function center(a, x, lo = 0, hi = a.length) {
    const i = left(a, x, lo, hi - 1);
    return i > lo && delta(a[i - 1], x) > -delta(a[i], x) ? i - 1 : i;
  }
  return {left, center, right};
}
function zero() {
  return 0;
}
function number(x) {
  return x === null ? NaN : +x;
}
function* numbers(values, valueof) {
  if (valueof === void 0) {
    for (let value of values) {
      if (value != null && (value = +value) >= value) {
        yield value;
      }
    }
  } else {
    let index2 = -1;
    for (let value of values) {
      if ((value = valueof(value, ++index2, values)) != null && (value = +value) >= value) {
        yield value;
      }
    }
  }
}
const ascendingBisect = bisector(ascending);
const bisectRight = ascendingBisect.right;
const bisectLeft = ascendingBisect.left;
const bisectCenter = bisector(number).center;
function count(values, valueof) {
  let count2 = 0;
  if (valueof === void 0) {
    for (let value of values) {
      if (value != null && (value = +value) >= value) {
        ++count2;
      }
    }
  } else {
    let index2 = -1;
    for (let value of values) {
      if ((value = valueof(value, ++index2, values)) != null && (value = +value) >= value) {
        ++count2;
      }
    }
  }
  return count2;
}
function length(array2) {
  return array2.length | 0;
}
function empty(length2) {
  return !(length2 > 0);
}
function arrayify(values) {
  return typeof values !== "object" || "length" in values ? values : Array.from(values);
}
function reducer(reduce2) {
  return (values) => reduce2(...values);
}
function cross(...values) {
  const reduce2 = typeof values[values.length - 1] === "function" && reducer(values.pop());
  values = values.map(arrayify);
  const lengths = values.map(length);
  const j = values.length - 1;
  const index2 = new Array(j + 1).fill(0);
  const product = [];
  if (j < 0 || lengths.some(empty))
    return product;
  while (true) {
    product.push(index2.map((j2, i2) => values[i2][j2]));
    let i = j;
    while (++index2[i] === lengths[i]) {
      if (i === 0)
        return reduce2 ? product.map(reduce2) : product;
      index2[i--] = 0;
    }
  }
}
function cumsum(values, valueof) {
  var sum2 = 0, index2 = 0;
  return Float64Array.from(values, valueof === void 0 ? (v) => sum2 += +v || 0 : (v) => sum2 += +valueof(v, index2++, values) || 0);
}
function variance(values, valueof) {
  let count2 = 0;
  let delta;
  let mean2 = 0;
  let sum2 = 0;
  if (valueof === void 0) {
    for (let value of values) {
      if (value != null && (value = +value) >= value) {
        delta = value - mean2;
        mean2 += delta / ++count2;
        sum2 += delta * (value - mean2);
      }
    }
  } else {
    let index2 = -1;
    for (let value of values) {
      if ((value = valueof(value, ++index2, values)) != null && (value = +value) >= value) {
        delta = value - mean2;
        mean2 += delta / ++count2;
        sum2 += delta * (value - mean2);
      }
    }
  }
  if (count2 > 1)
    return sum2 / (count2 - 1);
}
function deviation(values, valueof) {
  const v = variance(values, valueof);
  return v ? Math.sqrt(v) : v;
}
function extent(values, valueof) {
  let min2;
  let max2;
  if (valueof === void 0) {
    for (const value of values) {
      if (value != null) {
        if (min2 === void 0) {
          if (value >= value)
            min2 = max2 = value;
        } else {
          if (min2 > value)
            min2 = value;
          if (max2 < value)
            max2 = value;
        }
      }
    }
  } else {
    let index2 = -1;
    for (let value of values) {
      if ((value = valueof(value, ++index2, values)) != null) {
        if (min2 === void 0) {
          if (value >= value)
            min2 = max2 = value;
        } else {
          if (min2 > value)
            min2 = value;
          if (max2 < value)
            max2 = value;
        }
      }
    }
  }
  return [min2, max2];
}
class Adder {
  constructor() {
    this._partials = new Float64Array(32);
    this._n = 0;
  }
  add(x) {
    const p = this._partials;
    let i = 0;
    for (let j = 0; j < this._n && j < 32; j++) {
      const y = p[j], hi = x + y, lo = Math.abs(x) < Math.abs(y) ? x - (hi - y) : y - (hi - x);
      if (lo)
        p[i++] = lo;
      x = hi;
    }
    p[i] = x;
    this._n = i + 1;
    return this;
  }
  valueOf() {
    const p = this._partials;
    let n = this._n, x, y, lo, hi = 0;
    if (n > 0) {
      hi = p[--n];
      while (n > 0) {
        x = hi;
        y = p[--n];
        hi = x + y;
        lo = y - (hi - x);
        if (lo)
          break;
      }
      if (n > 0 && (lo < 0 && p[n - 1] < 0 || lo > 0 && p[n - 1] > 0)) {
        y = lo * 2;
        x = hi + y;
        if (y == x - hi)
          hi = x;
      }
    }
    return hi;
  }
}
function fsum(values, valueof) {
  const adder = new Adder();
  if (valueof === void 0) {
    for (let value of values) {
      if (value = +value) {
        adder.add(value);
      }
    }
  } else {
    let index2 = -1;
    for (let value of values) {
      if (value = +valueof(value, ++index2, values)) {
        adder.add(value);
      }
    }
  }
  return +adder;
}
function fcumsum(values, valueof) {
  const adder = new Adder();
  let index2 = -1;
  return Float64Array.from(values, valueof === void 0 ? (v) => adder.add(+v || 0) : (v) => adder.add(+valueof(v, ++index2, values) || 0));
}
function identity(x) {
  return x;
}
function group(values, ...keys) {
  return nest(values, identity, identity, keys);
}
function groups(values, ...keys) {
  return nest(values, Array.from, identity, keys);
}
function flatten(groups2, keys) {
  for (let i = 1, n = keys.length; i < n; ++i) {
    groups2 = groups2.flatMap((g) => g.pop().map(([key, value]) => [...g, key, value]));
  }
  return groups2;
}
function flatGroup(values, ...keys) {
  return flatten(groups(values, ...keys), keys);
}
function flatRollup(values, reduce2, ...keys) {
  return flatten(rollups(values, reduce2, ...keys), keys);
}
function rollup(values, reduce2, ...keys) {
  return nest(values, identity, reduce2, keys);
}
function rollups(values, reduce2, ...keys) {
  return nest(values, Array.from, reduce2, keys);
}
function index(values, ...keys) {
  return nest(values, identity, unique, keys);
}
function indexes(values, ...keys) {
  return nest(values, Array.from, unique, keys);
}
function unique(values) {
  if (values.length !== 1)
    throw new Error("duplicate key");
  return values[0];
}
function nest(values, map2, reduce2, keys) {
  return function regroup(values2, i) {
    if (i >= keys.length)
      return reduce2(values2);
    const groups2 = new InternMap();
    const keyof = keys[i++];
    let index2 = -1;
    for (const value of values2) {
      const key = keyof(value, ++index2, values2);
      const group2 = groups2.get(key);
      if (group2)
        group2.push(value);
      else
        groups2.set(key, [value]);
    }
    for (const [key, values3] of groups2) {
      groups2.set(key, regroup(values3, i));
    }
    return map2(groups2);
  }(values, 0);
}
function permute(source, keys) {
  return Array.from(keys, (key) => source[key]);
}
function sort(values, ...F) {
  if (typeof values[Symbol.iterator] !== "function")
    throw new TypeError("values is not iterable");
  values = Array.from(values);
  let [f] = F;
  if (f && f.length !== 2 || F.length > 1) {
    const index2 = Uint32Array.from(values, (d, i) => i);
    if (F.length > 1) {
      F = F.map((f2) => values.map(f2));
      index2.sort((i, j) => {
        for (const f2 of F) {
          const c = ascendingDefined(f2[i], f2[j]);
          if (c)
            return c;
        }
      });
    } else {
      f = values.map(f);
      index2.sort((i, j) => ascendingDefined(f[i], f[j]));
    }
    return permute(values, index2);
  }
  return values.sort(compareDefined(f));
}
function compareDefined(compare = ascending) {
  if (compare === ascending)
    return ascendingDefined;
  if (typeof compare !== "function")
    throw new TypeError("compare is not a function");
  return (a, b) => {
    const x = compare(a, b);
    if (x || x === 0)
      return x;
    return (compare(b, b) === 0) - (compare(a, a) === 0);
  };
}
function ascendingDefined(a, b) {
  return (a == null || !(a >= a)) - (b == null || !(b >= b)) || (a < b ? -1 : a > b ? 1 : 0);
}
function groupSort(values, reduce2, key) {
  return (reduce2.length !== 2 ? sort(rollup(values, reduce2, key), ([ak, av], [bk, bv]) => ascending(av, bv) || ascending(ak, bk)) : sort(group(values, key), ([ak, av], [bk, bv]) => reduce2(av, bv) || ascending(ak, bk))).map(([key2]) => key2);
}
var array = Array.prototype;
var slice = array.slice;
function constant(x) {
  return () => x;
}
var e10 = Math.sqrt(50), e5 = Math.sqrt(10), e2 = Math.sqrt(2);
function ticks(start, stop, count2) {
  var reverse2, i = -1, n, ticks2, step;
  stop = +stop, start = +start, count2 = +count2;
  if (start === stop && count2 > 0)
    return [start];
  if (reverse2 = stop < start)
    n = start, start = stop, stop = n;
  if ((step = tickIncrement(start, stop, count2)) === 0 || !isFinite(step))
    return [];
  if (step > 0) {
    let r0 = Math.round(start / step), r1 = Math.round(stop / step);
    if (r0 * step < start)
      ++r0;
    if (r1 * step > stop)
      --r1;
    ticks2 = new Array(n = r1 - r0 + 1);
    while (++i < n)
      ticks2[i] = (r0 + i) * step;
  } else {
    step = -step;
    let r0 = Math.round(start * step), r1 = Math.round(stop * step);
    if (r0 / step < start)
      ++r0;
    if (r1 / step > stop)
      --r1;
    ticks2 = new Array(n = r1 - r0 + 1);
    while (++i < n)
      ticks2[i] = (r0 + i) / step;
  }
  if (reverse2)
    ticks2.reverse();
  return ticks2;
}
function tickIncrement(start, stop, count2) {
  var step = (stop - start) / Math.max(0, count2), power = Math.floor(Math.log(step) / Math.LN10), error = step / Math.pow(10, power);
  return power >= 0 ? (error >= e10 ? 10 : error >= e5 ? 5 : error >= e2 ? 2 : 1) * Math.pow(10, power) : -Math.pow(10, -power) / (error >= e10 ? 10 : error >= e5 ? 5 : error >= e2 ? 2 : 1);
}
function tickStep(start, stop, count2) {
  var step0 = Math.abs(stop - start) / Math.max(0, count2), step1 = Math.pow(10, Math.floor(Math.log(step0) / Math.LN10)), error = step0 / step1;
  if (error >= e10)
    step1 *= 10;
  else if (error >= e5)
    step1 *= 5;
  else if (error >= e2)
    step1 *= 2;
  return stop < start ? -step1 : step1;
}
function nice(start, stop, count2) {
  let prestep;
  while (true) {
    const step = tickIncrement(start, stop, count2);
    if (step === prestep || step === 0 || !isFinite(step)) {
      return [start, stop];
    } else if (step > 0) {
      start = Math.floor(start / step) * step;
      stop = Math.ceil(stop / step) * step;
    } else if (step < 0) {
      start = Math.ceil(start * step) / step;
      stop = Math.floor(stop * step) / step;
    }
    prestep = step;
  }
}
function thresholdSturges(values) {
  return Math.ceil(Math.log(count(values)) / Math.LN2) + 1;
}
function bin() {
  var value = identity, domain = extent, threshold = thresholdSturges;
  function histogram(data) {
    if (!Array.isArray(data))
      data = Array.from(data);
    var i, n = data.length, x, step, values = new Array(n);
    for (i = 0; i < n; ++i) {
      values[i] = value(data[i], i, data);
    }
    var xz = domain(values), x0 = xz[0], x1 = xz[1], tz = threshold(values, x0, x1);
    if (!Array.isArray(tz)) {
      const max2 = x1, tn = +tz;
      if (domain === extent)
        [x0, x1] = nice(x0, x1, tn);
      tz = ticks(x0, x1, tn);
      if (tz[0] <= x0)
        step = tickIncrement(x0, x1, tn);
      if (tz[tz.length - 1] >= x1) {
        if (max2 >= x1 && domain === extent) {
          const step2 = tickIncrement(x0, x1, tn);
          if (isFinite(step2)) {
            if (step2 > 0) {
              x1 = (Math.floor(x1 / step2) + 1) * step2;
            } else if (step2 < 0) {
              x1 = (Math.ceil(x1 * -step2) + 1) / -step2;
            }
          }
        } else {
          tz.pop();
        }
      }
    }
    var m = tz.length;
    while (tz[0] <= x0)
      tz.shift(), --m;
    while (tz[m - 1] > x1)
      tz.pop(), --m;
    var bins = new Array(m + 1), bin2;
    for (i = 0; i <= m; ++i) {
      bin2 = bins[i] = [];
      bin2.x0 = i > 0 ? tz[i - 1] : x0;
      bin2.x1 = i < m ? tz[i] : x1;
    }
    if (isFinite(step)) {
      if (step > 0) {
        for (i = 0; i < n; ++i) {
          if ((x = values[i]) != null && x0 <= x && x <= x1) {
            bins[Math.min(m, Math.floor((x - x0) / step))].push(data[i]);
          }
        }
      } else if (step < 0) {
        for (i = 0; i < n; ++i) {
          if ((x = values[i]) != null && x0 <= x && x <= x1) {
            const j = Math.floor((x0 - x) * step);
            bins[Math.min(m, j + (tz[j] <= x))].push(data[i]);
          }
        }
      }
    } else {
      for (i = 0; i < n; ++i) {
        if ((x = values[i]) != null && x0 <= x && x <= x1) {
          bins[bisectRight(tz, x, 0, m)].push(data[i]);
        }
      }
    }
    return bins;
  }
  histogram.value = function(_) {
    return arguments.length ? (value = typeof _ === "function" ? _ : constant(_), histogram) : value;
  };
  histogram.domain = function(_) {
    return arguments.length ? (domain = typeof _ === "function" ? _ : constant([_[0], _[1]]), histogram) : domain;
  };
  histogram.thresholds = function(_) {
    return arguments.length ? (threshold = typeof _ === "function" ? _ : Array.isArray(_) ? constant(slice.call(_)) : constant(_), histogram) : threshold;
  };
  return histogram;
}
function max(values, valueof) {
  let max2;
  if (valueof === void 0) {
    for (const value of values) {
      if (value != null && (max2 < value || max2 === void 0 && value >= value)) {
        max2 = value;
      }
    }
  } else {
    let index2 = -1;
    for (let value of values) {
      if ((value = valueof(value, ++index2, values)) != null && (max2 < value || max2 === void 0 && value >= value)) {
        max2 = value;
      }
    }
  }
  return max2;
}
function min(values, valueof) {
  let min2;
  if (valueof === void 0) {
    for (const value of values) {
      if (value != null && (min2 > value || min2 === void 0 && value >= value)) {
        min2 = value;
      }
    }
  } else {
    let index2 = -1;
    for (let value of values) {
      if ((value = valueof(value, ++index2, values)) != null && (min2 > value || min2 === void 0 && value >= value)) {
        min2 = value;
      }
    }
  }
  return min2;
}
function quickselect(array2, k, left = 0, right = array2.length - 1, compare) {
  compare = compare === void 0 ? ascendingDefined : compareDefined(compare);
  while (right > left) {
    if (right - left > 600) {
      const n = right - left + 1;
      const m = k - left + 1;
      const z = Math.log(n);
      const s = 0.5 * Math.exp(2 * z / 3);
      const sd = 0.5 * Math.sqrt(z * s * (n - s) / n) * (m - n / 2 < 0 ? -1 : 1);
      const newLeft = Math.max(left, Math.floor(k - m * s / n + sd));
      const newRight = Math.min(right, Math.floor(k + (n - m) * s / n + sd));
      quickselect(array2, k, newLeft, newRight, compare);
    }
    const t = array2[k];
    let i = left;
    let j = right;
    swap(array2, left, k);
    if (compare(array2[right], t) > 0)
      swap(array2, left, right);
    while (i < j) {
      swap(array2, i, j), ++i, --j;
      while (compare(array2[i], t) < 0)
        ++i;
      while (compare(array2[j], t) > 0)
        --j;
    }
    if (compare(array2[left], t) === 0)
      swap(array2, left, j);
    else
      ++j, swap(array2, j, right);
    if (j <= k)
      left = j + 1;
    if (k <= j)
      right = j - 1;
  }
  return array2;
}
function swap(array2, i, j) {
  const t = array2[i];
  array2[i] = array2[j];
  array2[j] = t;
}
function quantile(values, p, valueof) {
  values = Float64Array.from(numbers(values, valueof));
  if (!(n = values.length))
    return;
  if ((p = +p) <= 0 || n < 2)
    return min(values);
  if (p >= 1)
    return max(values);
  var n, i = (n - 1) * p, i0 = Math.floor(i), value0 = max(quickselect(values, i0).subarray(0, i0 + 1)), value1 = min(values.subarray(i0 + 1));
  return value0 + (value1 - value0) * (i - i0);
}
function quantileSorted(values, p, valueof = number) {
  if (!(n = values.length))
    return;
  if ((p = +p) <= 0 || n < 2)
    return +valueof(values[0], 0, values);
  if (p >= 1)
    return +valueof(values[n - 1], n - 1, values);
  var n, i = (n - 1) * p, i0 = Math.floor(i), value0 = +valueof(values[i0], i0, values), value1 = +valueof(values[i0 + 1], i0 + 1, values);
  return value0 + (value1 - value0) * (i - i0);
}
function thresholdFreedmanDiaconis(values, min2, max2) {
  return Math.ceil((max2 - min2) / (2 * (quantile(values, 0.75) - quantile(values, 0.25)) * Math.pow(count(values), -1 / 3)));
}
function thresholdScott(values, min2, max2) {
  return Math.ceil((max2 - min2) * Math.cbrt(count(values)) / (3.49 * deviation(values)));
}
function maxIndex(values, valueof) {
  let max2;
  let maxIndex2 = -1;
  let index2 = -1;
  if (valueof === void 0) {
    for (const value of values) {
      ++index2;
      if (value != null && (max2 < value || max2 === void 0 && value >= value)) {
        max2 = value, maxIndex2 = index2;
      }
    }
  } else {
    for (let value of values) {
      if ((value = valueof(value, ++index2, values)) != null && (max2 < value || max2 === void 0 && value >= value)) {
        max2 = value, maxIndex2 = index2;
      }
    }
  }
  return maxIndex2;
}
function mean(values, valueof) {
  let count2 = 0;
  let sum2 = 0;
  if (valueof === void 0) {
    for (let value of values) {
      if (value != null && (value = +value) >= value) {
        ++count2, sum2 += value;
      }
    }
  } else {
    let index2 = -1;
    for (let value of values) {
      if ((value = valueof(value, ++index2, values)) != null && (value = +value) >= value) {
        ++count2, sum2 += value;
      }
    }
  }
  if (count2)
    return sum2 / count2;
}
function median(values, valueof) {
  return quantile(values, 0.5, valueof);
}
function* flatten$1(arrays) {
  for (const array2 of arrays) {
    yield* array2;
  }
}
function merge(arrays) {
  return Array.from(flatten$1(arrays));
}
function minIndex(values, valueof) {
  let min2;
  let minIndex2 = -1;
  let index2 = -1;
  if (valueof === void 0) {
    for (const value of values) {
      ++index2;
      if (value != null && (min2 > value || min2 === void 0 && value >= value)) {
        min2 = value, minIndex2 = index2;
      }
    }
  } else {
    for (let value of values) {
      if ((value = valueof(value, ++index2, values)) != null && (min2 > value || min2 === void 0 && value >= value)) {
        min2 = value, minIndex2 = index2;
      }
    }
  }
  return minIndex2;
}
function mode(values, valueof) {
  const counts = new InternMap();
  if (valueof === void 0) {
    for (let value of values) {
      if (value != null && value >= value) {
        counts.set(value, (counts.get(value) || 0) + 1);
      }
    }
  } else {
    let index2 = -1;
    for (let value of values) {
      if ((value = valueof(value, ++index2, values)) != null && value >= value) {
        counts.set(value, (counts.get(value) || 0) + 1);
      }
    }
  }
  let modeValue;
  let modeCount = 0;
  for (const [value, count2] of counts) {
    if (count2 > modeCount) {
      modeCount = count2;
      modeValue = value;
    }
  }
  return modeValue;
}
function pairs(values, pairof = pair) {
  const pairs2 = [];
  let previous;
  let first = false;
  for (const value of values) {
    if (first)
      pairs2.push(pairof(previous, value));
    previous = value;
    first = true;
  }
  return pairs2;
}
function pair(a, b) {
  return [a, b];
}
function range(start, stop, step) {
  start = +start, stop = +stop, step = (n = arguments.length) < 2 ? (stop = start, start = 0, 1) : n < 3 ? 1 : +step;
  var i = -1, n = Math.max(0, Math.ceil((stop - start) / step)) | 0, range2 = new Array(n);
  while (++i < n) {
    range2[i] = start + i * step;
  }
  return range2;
}
function rank(values, valueof = ascending) {
  if (typeof values[Symbol.iterator] !== "function")
    throw new TypeError("values is not iterable");
  let V = Array.from(values);
  const R = new Float64Array(V.length);
  if (valueof.length !== 2)
    V = V.map(valueof), valueof = ascending;
  const compareIndex = (i, j) => valueof(V[i], V[j]);
  let k, r;
  Uint32Array.from(V, (_, i) => i).sort(valueof === ascending ? (i, j) => ascendingDefined(V[i], V[j]) : compareDefined(compareIndex)).forEach((j, i) => {
    const c = compareIndex(j, k === void 0 ? j : k);
    if (c >= 0) {
      if (k === void 0 || c > 0)
        k = j, r = i;
      R[j] = r;
    } else {
      R[j] = NaN;
    }
  });
  return R;
}
function least(values, compare = ascending) {
  let min2;
  let defined = false;
  if (compare.length === 1) {
    let minValue;
    for (const element of values) {
      const value = compare(element);
      if (defined ? ascending(value, minValue) < 0 : ascending(value, value) === 0) {
        min2 = element;
        minValue = value;
        defined = true;
      }
    }
  } else {
    for (const value of values) {
      if (defined ? compare(value, min2) < 0 : compare(value, value) === 0) {
        min2 = value;
        defined = true;
      }
    }
  }
  return min2;
}
function leastIndex(values, compare = ascending) {
  if (compare.length === 1)
    return minIndex(values, compare);
  let minValue;
  let min2 = -1;
  let index2 = -1;
  for (const value of values) {
    ++index2;
    if (min2 < 0 ? compare(value, value) === 0 : compare(value, minValue) < 0) {
      minValue = value;
      min2 = index2;
    }
  }
  return min2;
}
function greatest(values, compare = ascending) {
  let max2;
  let defined = false;
  if (compare.length === 1) {
    let maxValue;
    for (const element of values) {
      const value = compare(element);
      if (defined ? ascending(value, maxValue) > 0 : ascending(value, value) === 0) {
        max2 = element;
        maxValue = value;
        defined = true;
      }
    }
  } else {
    for (const value of values) {
      if (defined ? compare(value, max2) > 0 : compare(value, value) === 0) {
        max2 = value;
        defined = true;
      }
    }
  }
  return max2;
}
function greatestIndex(values, compare = ascending) {
  if (compare.length === 1)
    return maxIndex(values, compare);
  let maxValue;
  let max2 = -1;
  let index2 = -1;
  for (const value of values) {
    ++index2;
    if (max2 < 0 ? compare(value, value) === 0 : compare(value, maxValue) > 0) {
      maxValue = value;
      max2 = index2;
    }
  }
  return max2;
}
function scan(values, compare) {
  const index2 = leastIndex(values, compare);
  return index2 < 0 ? void 0 : index2;
}
var shuffle = shuffler(Math.random);
function shuffler(random) {
  return function shuffle2(array2, i0 = 0, i1 = array2.length) {
    let m = i1 - (i0 = +i0);
    while (m) {
      const i = random() * m-- | 0, t = array2[m + i0];
      array2[m + i0] = array2[i + i0];
      array2[i + i0] = t;
    }
    return array2;
  };
}
function sum(values, valueof) {
  let sum2 = 0;
  if (valueof === void 0) {
    for (let value of values) {
      if (value = +value) {
        sum2 += value;
      }
    }
  } else {
    let index2 = -1;
    for (let value of values) {
      if (value = +valueof(value, ++index2, values)) {
        sum2 += value;
      }
    }
  }
  return sum2;
}
function transpose(matrix) {
  if (!(n = matrix.length))
    return [];
  for (var i = -1, m = min(matrix, length$1), transpose2 = new Array(m); ++i < m; ) {
    for (var j = -1, n, row = transpose2[i] = new Array(n); ++j < n; ) {
      row[j] = matrix[j][i];
    }
  }
  return transpose2;
}
function length$1(d) {
  return d.length;
}
function zip() {
  return transpose(arguments);
}
function every(values, test) {
  if (typeof test !== "function")
    throw new TypeError("test is not a function");
  let index2 = -1;
  for (const value of values) {
    if (!test(value, ++index2, values)) {
      return false;
    }
  }
  return true;
}
function some(values, test) {
  if (typeof test !== "function")
    throw new TypeError("test is not a function");
  let index2 = -1;
  for (const value of values) {
    if (test(value, ++index2, values)) {
      return true;
    }
  }
  return false;
}
function filter(values, test) {
  if (typeof test !== "function")
    throw new TypeError("test is not a function");
  const array2 = [];
  let index2 = -1;
  for (const value of values) {
    if (test(value, ++index2, values)) {
      array2.push(value);
    }
  }
  return array2;
}
function map(values, mapper) {
  if (typeof values[Symbol.iterator] !== "function")
    throw new TypeError("values is not iterable");
  if (typeof mapper !== "function")
    throw new TypeError("mapper is not a function");
  return Array.from(values, (value, index2) => mapper(value, index2, values));
}
function reduce(values, reducer2, value) {
  if (typeof reducer2 !== "function")
    throw new TypeError("reducer is not a function");
  const iterator = values[Symbol.iterator]();
  let done, next, index2 = -1;
  if (arguments.length < 3) {
    ({done, value} = iterator.next());
    if (done)
      return;
    ++index2;
  }
  while ({done, value: next} = iterator.next(), !done) {
    value = reducer2(value, next, ++index2, values);
  }
  return value;
}
function reverse(values) {
  if (typeof values[Symbol.iterator] !== "function")
    throw new TypeError("values is not iterable");
  return Array.from(values).reverse();
}
function difference(values, ...others) {
  values = new InternSet(values);
  for (const other of others) {
    for (const value of other) {
      values.delete(value);
    }
  }
  return values;
}
function disjoint(values, other) {
  const iterator = other[Symbol.iterator](), set2 = new InternSet();
  for (const v of values) {
    if (set2.has(v))
      return false;
    let value, done;
    while ({value, done} = iterator.next()) {
      if (done)
        break;
      if (Object.is(v, value))
        return false;
      set2.add(value);
    }
  }
  return true;
}
function intersection(values, ...others) {
  values = new InternSet(values);
  others = others.map(set);
  out:
    for (const value of values) {
      for (const other of others) {
        if (!other.has(value)) {
          values.delete(value);
          continue out;
        }
      }
    }
  return values;
}
function set(values) {
  return values instanceof InternSet ? values : new InternSet(values);
}
function superset(values, other) {
  const iterator = values[Symbol.iterator](), set2 = new Set();
  for (const o of other) {
    const io = intern(o);
    if (set2.has(io))
      continue;
    let value, done;
    while ({value, done} = iterator.next()) {
      if (done)
        return false;
      const ivalue = intern(value);
      set2.add(ivalue);
      if (Object.is(io, ivalue))
        break;
    }
  }
  return true;
}
function intern(value) {
  return value !== null && typeof value === "object" ? value.valueOf() : value;
}
function subset(values, other) {
  return superset(other, values);
}
function union(...others) {
  const set2 = new InternSet();
  for (const other of others) {
    for (const o of other) {
      set2.add(o);
    }
  }
  return set2;
}
export {Adder, ascending, bin, bisectRight as bisect, bisectCenter, bisectLeft, bisectRight, bisector, count, cross, cumsum, descending, deviation, difference, disjoint, every, extent, fcumsum, filter, flatGroup, flatRollup, fsum, greatest, greatestIndex, group, groupSort, groups, bin as histogram, index, indexes, intersection, least, leastIndex, map, max, maxIndex, mean, median, merge, min, minIndex, mode, nice, pairs, permute, quantile, quantileSorted, quickselect, range, rank, reduce, reverse, rollup, rollups, scan, shuffle, shuffler, some, sort, subset, sum, superset, thresholdFreedmanDiaconis, thresholdScott, thresholdSturges, tickIncrement, tickStep, ticks, transpose, union, variance, zip};
export default null;
