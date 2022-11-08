#include "soro/server/import/import.h"

#include "utl/logging.h"

#include "tiles/db/clear_database.h"
#include "tiles/db/database_stats.h"
#include "tiles/db/feature_inserter_mt.h"
#include "tiles/db/feature_pack.h"
#include "tiles/db/pack_file.h"
#include "tiles/db/prepare_tiles.h"
#include "tiles/db/tile_database.h"
#include "tiles/osm/feature_handler.h"
#include "tiles/osm/load_osm.h"

namespace soro::server {

void import_tiles(import_settings const& settings) {
  tiles::check_profile(settings.osm_profile_.string());

  uLOG(utl::info) << "Clearing Database";
  tiles::clear_database(settings.db_path_.string(), tiles::kDefaultSize);
  tiles::clear_pack_file(settings.db_path_.string().c_str());

  lmdb::env db_env =
      tiles::make_tile_database(settings.db_path_.string().c_str(), tiles::kDefaultSize);
  tiles::tile_db_handle db_handle{db_env};
  tiles::pack_handle pack_handle{settings.db_path_.string().c_str()};

  {
    tiles::feature_inserter_mt inserter{
        tiles::dbi_handle{db_handle, db_handle.features_dbi_opener()},
        pack_handle};

    uLOG(utl::info) << "Loading features";
    load_osm(db_handle, inserter, settings.osm_path_. string(),
             settings.osm_profile_.string(),
             settings.tmp_dir_.string());
  }

  database_stats(db_handle, pack_handle);

  uLOG(utl::info) << "Packing features";
  pack_features(db_handle, pack_handle);

  uLOG(utl::info) << "Preparing tiles";
  prepare_tiles(db_handle, pack_handle, 10);

  uLOG(utl::info) << "Import done!";
}

}  // namespace soro::server