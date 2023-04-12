import { devMode } from '@/util/devMode';

class HttpClient {
  private _baseURL: string;
  private _headers: Headers;

  constructor(baseUrl: string, options: RequestInit = {}) {
    this._baseURL = baseUrl || '';
    this._headers = <Headers>options.headers || {};
  }

  async _fetchJSON<Type>(
    endpoint: string,
    options: RequestInit = {}
  ): Promise<Type> {
    const res = await fetch(this._baseURL + endpoint, {
      ...options,
      headers: this._headers
    });

    if (!res.ok || res.status === 204) {
      throw new Error(res.statusText);
    }

    if (!devMode()) {
      return res.json();
    }

    // debug code, when in dev mode
    const copy = res.clone();
    try {
      return await res.json();
    } catch (e) {
      copy
        .clone()
        .text()
        .then((text) =>
          console.error('error in http client:', e, '\nwith text\n', text)
        );
      throw e;
    }
    // debug code end
  }

  setHeader(name: string, value: string) {
    this._headers.set(name, value);
    return this;
  }

  getHeader(name: string) {
    return this._headers.get(name);
  }

  setBasicAuth(username: string, password: string) {
    this._headers.set(
      'Authorization',
      'Basic ' + btoa(username) + ':' + btoa(password)
    );
    return this;
  }

  setBearerAuth(token: string) {
    this._headers.set('Authorization', 'Bearer ' + token);
    return this;
  }

  get<Type>(endpoint: string, options: RequestInit = {}) {
    return this._fetchJSON<Type>(endpoint, {
      ...options,
      method: 'GET'
    });
  }

  post(endpoint: string, body: any, options: RequestInit = {}) {
    return this._fetchJSON(endpoint, {
      ...options,
      body: body ? JSON.stringify(body) : undefined,
      method: 'POST'
    });
  }

  put(endpoint: string, body: any, options: RequestInit = {}) {
    return this._fetchJSON(endpoint, {
      ...options,
      body: body ? JSON.stringify(body) : undefined,
      method: 'PUT'
    });
  }

  delete(endpoint: string, options: RequestInit = {}) {
    return this._fetchJSON(endpoint, {
      ...options,
      method: 'DELETE'
    });
  }
}

export default HttpClient;
