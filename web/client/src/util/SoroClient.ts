import HttpClient from '@/util/HttpClient';
import { LngLatBounds } from 'maplibre-gl';

type InfrastructureList = { infrastructures: string[] };
type BoundingBox = { boundingBox: LngLatBounds };
type Coordinate = { lon: number; lat: number };

export type Station = {
  id: number;
  ds100: string;
  stationRoutes: StationRoute[];
  interlockingRouteIds: number[];
};
type StationList = { stations: string[] };

export type StationRoute = {
  id: number;
  name: string;
  fromElement: number;
  toElement: number;

  // only available if StationRoute is fetched directly
  ds100?: string;
  path?: Coordinate[];
};

export type InterlockingRoute = {
  id: number;

  // only available if InterlockingRoute is fetched directly
  path?: Coordinate[];
};

type EOTD = {
  signal: boolean;
};

type Slope = {
  rising: number;
  falling: number;
};

type Halt = {
  name: string;
  passenger: boolean;
  idExtern: string;
  idOp: string;
};

enum PointOfActivation {
  Here,
  LastSignal
}

type SpeedLimit = {
  limit: number;
  pointOfActivation: PointOfActivation;
};

type Switch = {
  name: string;
};

type Element = EOTD | Slope | Halt | SpeedLimit | Switch;

export enum SearchResultType {
  Station = 'station',
  Element = 'element',
  StationRoute = 'stationRoute',
  InterlockingRoute = 'interlockingRoute'
}

export type SearchResult = {
  type: SearchResultType;
  id: number;
  name: string;
  boundingBox: BoundingBox;

  // empty when type !== Element
  elementType: string;
};

type Infrastructure = {
  boundingBox: () => Promise<BoundingBox>;
  stations: () => Promise<StationList>;
  station: (id: number) => Promise<Station>;
  interlockingRoute: (id: number) => Promise<InterlockingRoute>;
  stationRoute: (id: number) => Promise<StationRoute>;
  element: (id: number) => Promise<Element>;
  search: (query: string) => Promise<SearchResult[]>;
};

class SoroClient extends HttpClient {
  constructor(baseURL: string, headers: RequestInit = {}) {
    super(baseURL, headers);
  }

  infrastructures(): Promise<InfrastructureList> {
    return this.get<InfrastructureList>('/infrastructures');
  }

  infrastructure(name: string): Infrastructure {
    return {
      boundingBox: () => this.boundingBox(name),
      stations: () => this.stations(name),
      station: (id: number) => this.station(name, id),
      stationRoute: (id: number) => this.stationRoute(name, id),
      interlockingRoute: (id: number) => this.interlockingRoute(name, id),
      element: (id: number) => this.element(name, id),
      search: (query: string) => this.search(name, query)
    };
  }

  boundingBox(infrastructureName: string): Promise<BoundingBox> {
    return this.get('/infrastructure/' + infrastructureName + '/bounding_box');
  }

  stations(infrastructureName: string): Promise<StationList> {
    return this.get('/infrastructure/' + infrastructureName + '/stations');
  }

  station(infrastructureName: string, stationId: number): Promise<Station> {
    return this.get(
      '/infrastructure/' + infrastructureName + '/station/' + stationId
    );
  }

  stationRoute(
    infrastructureName: string,
    stationRouteId: number
  ): Promise<StationRoute> {
    return this.get(
      '/infrastructure/' +
        infrastructureName +
        '/station_route/' +
        stationRouteId
    );
  }

  interlockingRoute(
    infrastructureName: string,
    interlockingRouteId: number
  ): Promise<InterlockingRoute> {
    return this.get(
      '/infrastructure/' +
        infrastructureName +
        '/interlocking_route/' +
        interlockingRouteId
    );
  }

  element(infrastructureName: string, elementId: number): Promise<Element> {
    return this.get(
      '/infrastructure/' + infrastructureName + '/element/' + elementId
    );
  }

  search(infrastructureName: string, query: string): Promise<SearchResult[]> {
    return this.get(
      '/infrastructure/' + infrastructureName + '/search/' + query
    );
  }
}

export default SoroClient;
