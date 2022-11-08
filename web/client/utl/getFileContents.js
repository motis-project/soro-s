export async function getFileContents(filename) {
  let headers = new Headers();
  headers.append('pragma', 'no-cache');
  headers.append('cache-control', 'no-cache');

  const settings = {
    headers: headers
  };

  return fetch(filename, settings)
    .then((resp) => {
      return resp.text()
    })
    .then((data) => {
      return data
    });
}

