export function* iterate(emscriptenList) {
    for (let i = 0; i < emscriptenList.size(); i++) {
        yield emscriptenList.get(i);
    }
}

function* iterateDist1D(dist1D) {
    const timeGranularity = 6;
    for (let i = 0; i < dist1D.dpd.size(); i++) {
        yield [dist1D.first.t + i * timeGranularity, 0, dist1D.dpd.get(i)];
    }
}

function* iterateDist2D(dist2D) {
    const granularity = dist2D.get_granularity();
    const timeGranularity = granularity.first.t;
    const speedGranularity = granularity.second.km_h;

    for (let i = 0; i < dist2D.dpd.size(); i++) {
        const innerDPD = dist2D.dpd.get(i);
        for (let j = 0; j < innerDPD.dpd.size(); j++) {
            yield [dist2D.first.t + i * timeGranularity, innerDPD.first.km_h + j * speedGranularity, innerDPD.dpd.get(j)];
        }
    }
}

export function* iterateDist(dist) {
    if (dist.constructor.name === 'TimeSpeedDPD') {
        yield* iterateDist2D(dist);
    } else {
        yield* iterateDist1D(dist);
    }
}