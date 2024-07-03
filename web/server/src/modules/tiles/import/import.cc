#include "soro/server/modules/tiles/import/import.h"

#include "utl/logging.h"
#include "utl/timer.h"

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
  utl::scoped_timer const timer("importing tiles");
  tiles::check_profile(settings.osm_profile_.string());

  tiles::clear_database(settings.db_path_.string(), tiles::kDefaultSize);
  tiles::clear_pack_file(settings.db_path_.string().c_str());
  timer.print("cleared database");

  auto db_env = tiles::make_tile_database(settings.db_path_.string().c_str(),
                                          tiles::kDefaultSize);
  tiles::tile_db_handle db_handle{db_env};
  tiles::pack_handle pack_handle{settings.db_path_.string().c_str()};

  {
    tiles::feature_inserter_mt inserter{
        tiles::dbi_handle{db_handle, db_handle.features_dbi_opener()},
        pack_handle};

    load_osm(db_handle, inserter, settings.osm_path_.string(),
             settings.osm_profile_.string(), settings.tmp_dir_.string());
    timer.print("loaded features");
  }

  database_stats(db_handle, pack_handle);

  uLOG(utl::info) << "Packing features";
  pack_features(db_handle, pack_handle);
  timer.print("packed features");

  uLOG(utl::info) << "Preparing tiles";
  prepare_tiles(db_handle, pack_handle, 10);
  timer.print("prepared tiles");
}

}  // namespace soro::server