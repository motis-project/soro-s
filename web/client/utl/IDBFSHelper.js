import { FS } from "../soro-client.js";

export function sync_fs(direction, callback) {
  FS.syncfs(direction, err => {
    if (err) {
      console.error(err);
      return;
    }

    callback();
  });
}

export function load_from_persistent(callback) {
  if (callback) {
    sync_fs(true, callback);
  } else {
    sync_fs(true, function () {
    });
  }
}

export function save_to_persistent(callback) {
  if (callback) {
    sync_fs(false, callback);
  } else {
    sync_fs(false, function () {
    });
  }
}

function syncFs(direction, callback) {
  FS.syncfs(direction, err => {
    if (err) {
      console.error(err);
      return;
    }

    callback();
  });
}

export function loadFromPersistent(callback) {
  if (callback) {
    syncFs(true, callback);
  } else {
    syncFs(true, function () {
    });
  }
}

export function saveToPersistent(callback = () => {
}) {
  if (callback) {
    syncFs(false, callback);
  } else {
    syncFs(false, function () {
    });
  }
}

export function exists(path) {
  return FS.analyzePath(path).exists;
}

export function timetableFileExists(timetableFilename) {
  const fp = '/idbfs/fpl/' + timetableFilename;

  if (FS.analyzePath(fp).exists) {
    return fp;
  }

  return undefined;
}

export function infrastructureFileExists(infrastructureFilename) {
  const fp = '/idbfs/iss/' + infrastructureFilename;

  if (FS.analyzePath(fp).exists) {
    return fp;
  }

  return undefined;
}

export function getInfrastructureFilenames() {
  return FS.readdir('/idbfs/iss/');
}

export function getTimetableFilenames() {
  return FS.readdir('/idbfs/fpl/');
}

export function getCacheFilenames() {
  return FS.readdir('/idbfs/cache/');
}

export function getStationCoordPath() {
  return '/idbfs/msc/btrs_geo.csv';
}

export function saveFileToIDBFS(filename, arrayBuffer) {
  const contents = new Uint8Array(arrayBuffer);

  let filePath = '/idbfs/';
  let type = filename.split('.').pop();

  if (type === 'iss' || type === 'xml') {
    filePath += 'iss/';
  } else if (type === 'fpl') {
    filePath += 'fpl/';
  } else {
    filePath += 'msc/';
  }

  filePath += filename;
  let fileStream = FS.open(filePath, 'w+');
  FS.write(fileStream, contents, 0, contents.length, 0);
  FS.close(fileStream);

  saveToPersistent();

  return filePath;
}

const dirs = ['iss/', 'fpl/', 'msc/', 'cache/'];

export function createFolders() {
  for (const dir of dirs) {
    if (!exists('/idbfs/' + dir)) {
      FS.mkdir('/idbfs/' + dir);
    }
  }
}

export function deleteAllFiles() {
  for (const dir of dirs) {
    const basePath = '/idbfs/' + dir;
    for (const fileName in FS.lookupPath(basePath).node.contents) {
      FS.unlink(basePath + fileName);
    }
  }

  saveToPersistent();
}

