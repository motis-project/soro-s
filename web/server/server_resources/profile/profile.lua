function process_node(node)
  if node:has_any_tag("place") then
    if node:has_tag("place", "city") then
      node:set_approved_min(5)
    elseif node:has_tag("place", "town") or
           node:has_tag("place", "borough") then
      node:set_approved_min(9)
    elseif node:has_tag("place", "suburb") or
           node:has_tag("place", "village") then
      node:set_approved_min(11)
    end

    node:set_target_layer("cities")
    node:add_tag_as_string("place")
    node:add_tag_as_string("name")
    node:add_tag_as_integer("population")
  end

  if node:has_any_tag("type") then
    node:set_approved_min(5)
    if node:has_any_tag("subtype") then
        node:set_target_layer(node:get_tag("type") .. "-" .. node:get_tag("subtype"))
        node:add_tag_as_integer("id")
        if (node:has_tag("direction", "rising")) then
            node:add_bool("rising", true)
        end
        if (node:has_tag("direction", "falling")) then
            node:add_bool("rising", false)
        end
    else
        node:set_target_layer(node:get_tag("type"))
        if node:has_tag("type", "station") then
            node:add_tag_as_integer("id")
            node:add_tag_as_string("name")
        end
    end
  end

end

function process_way(way)
  if way:has_any_tag("type") then
    way:set_approved_min(5)
    way:set_target_layer(way:get_tag("type") .. "-" .. "line-layer")
  end

--   if way:has_any_tag("highway") then
--     way:set_target_layer("road")
--     way:add_tag_as_string("highway")
--     way:add_tag_as_string("name")
--
--     if way:has_tag("highway", "motorway") or
--        way:has_tag("highway", "trunk") then
--       way:set_approved_min(5)
--       way:add_tag_as_string("ref")
--
--     elseif way:has_tag("highway", "motorway_link") or
--            way:has_tag("highway", "trunk_link") or
--            way:has_tag("highway", "primary") or
--            way:has_tag("highway", "secondary") or
--            way:has_tag("highway", "tertiary") or
--            way:has_tag("highway", "aeroway") then
--       way:set_approved_min(9)
--       way:add_tag_as_string("ref")
--
--     elseif way:has_tag("highway", "residential") or
--            way:has_tag("highway", "living_street") or
--            way:has_tag("highway", "primary_link") or
--            way:has_tag("highway", "secondary_link") or
--            way:has_tag("highway", "tertiary_link") or
--            way:has_tag("highway", "unclassified") or
--            way:has_tag("highway", "service") or
--            way:has_tag("highway", "footway") or
--            way:has_tag("highway", "track") or
--            way:has_tag("highway", "steps") or
--            way:has_tag("highway", "cycleway") or
--            way:has_tag("highway", "path") then
--       way:set_approved_min(12)
--     end
--
--   elseif way:has_any_tag("railway", "rail", "subway", "tram") then
--     if way:has_tag("railway", "disused") or
--        way:has_tag("railway", "abandoned") then
--       way:set_target_layer("rail")
--       way:set_approved_min(14)
--       way:add_string("rail", "old")
--
--     elseif way:has_tag("usage", "industrial") or
--             way:has_tag("usage", "military") or
--             way:has_tag("usage", "test") or
--             way:has_tag("usage", "tourism") or
--             way:has_tag("service", "yard") or
--             way:has_tag("service", "spur") or
--             way:has_tag("railway", "miniature") or
--             way:has_tag("railway:preserved", "yes") then
--       way:set_target_layer("rail")
--       way:set_approved_min(14)
--       way:add_string("rail", "detail")
--
--     elseif way:has_tag("railway", "subway") or
--            way:has_tag("railway", "tram") then
--       way:set_target_layer("rail")
--       way:set_approved_min(10)
--       way:add_string("rail", "secondary")
--     elseif way:has_tag("bridge","yes") then
--       way:set_target_layer("rail")
--       way:set_approved_min(5)
--       way:add_string("rail", "bridges")
--       way:add_tag_as_string("color")
--     elseif way:has_tag("tunnel","yes") then
--       way:set_target_layer("rail")
--       way:set_approved_min(5)
--       way:add_string("rail", "underground")
--       way:add_tag_as_string("color")
--     else
--       way:set_target_layer("rail")
--       way:set_approved_min(5)
--       way:add_string("rail", "primary")
--       way:add_tag_as_string("color")
--     end
--
--   elseif way:has_any_tag("waterway") then
--     way:set_target_layer("waterway")
--
--     if way:has_tag("waterway", "river") or
--        way:has_tag("waterway", "canal") then
--        way:set_approved_min(8)
--     elseif way:has_tag("waterway", "stream") then
--       way:set_approved_min(13)
--     elseif way:has_tag("waterway", "ditch") or
--            way:has_tag("waterway", "drain") then
--       way:set_approved_min(15)
--     end
--   -- elseif way:has_tag("boundary", "administrative") and
--   --        not way:has_tag("maritime", "yes") then
--   --   if way:has_tag("admin_level", "2") then
--   --     way:set_target_layer("border")
--   --     way:set_approved_full()
--   --     way:add_tag_as_integer("admin_level")
--   --   elseif way:has_tag("admin_level", "4") then
--   --     way:set_target_layer("border")
--   --     way:set_approved_full()
--   --     way:add_tag_as_integer("admin_level")
--   --   end
--   end
end

function process_area(area)
  if area:has_any_tag("building") then
    area:set_target_layer("building")
    area:set_approved_min_by_area(14, 1e8,
                                  12, 1e10,
                                  10, -1)

  elseif area:has_any_tag("landuse", "residential", "retail", "industrial", "commercial") then
    area:set_target_layer("landuse")
    area:add_tag_as_string("landuse")
    area:set_approved_min_by_area(14, 1e8,
                                  10, 1e10,
                                   8, -1)

  elseif area:has_any_tag("landuse", "quarry", "farmyard", "railway") then
    area:set_target_layer("landuse")
    area:add_string("landuse", "industrial")
    area:set_approved_min_by_area(14, 1e8,
                                  10, 1e10,
                                   8, -1)

  elseif area:has_tag("leisure", "sports_centre") or
         area:has_tag("amenity", "hospital") or
         area:has_tag("amenity", "police") or
         area:has_tag("amenity", "fire_station") or
         area:has_tag("amenity", "kindergarten") or
         area:has_tag("amenity", "school") or
         area:has_tag("amenity", "place_of_worship") or
         area:has_tag("amenity", "university") then
    area:set_target_layer("landuse")
    area:add_string("landuse", "complex")
    area:set_approved_min_by_area(14, 1e8,
                                  10, 1e10,
                                   8, -1)

  elseif area:has_tag("landuse", "forest") or
         area:has_tag("natural", "wood") or
         area:has_tag("natural", "oarchard") or
         area:has_tag("natural", "scrub") then
    area:set_target_layer("landuse")
    area:add_string("landuse", "nature_heavy")
    area:set_approved_min_by_area(14, 1e8,
                                  10, 1e10,
                                   8, -1)

  elseif area:has_tag("landuse", "farmland") or
         area:has_tag("landuse", "vineyard") or
         area:has_tag("landuse", "plant_nursery") or
         area:has_tag("landuse", "meadow") or
         area:has_tag("natural", "grassland") or
         area:has_tag("landuse", "grass") then
    area:set_target_layer("landuse")
    area:add_string("landuse", "nature_light")
    area:set_approved_min_by_area(14, 1e8,
                                  10, 1e10,
                                   8, -1)

  elseif area:has_tag("leisure", "park") or
         area:has_tag("leisure", "garden") or
         area:has_tag("leisure", "playground") or
         area:has_tag("leisure", "stadium") or
         area:has_tag("landuse", "recreation_ground") or
         area:has_tag("landuse", "greenhouse_horticulture") or
         area:has_tag("landuse", "allotments") then
    area:set_target_layer("landuse")
    area:add_string("landuse", "park")
    area:set_approved_min_by_area(14, 1e8,
                                  10, 1e10,
                                   8, -1)

  elseif area:has_tag("landuse", "cemetery") then
    area:set_target_layer("landuse")
    area:add_string("landuse", "cemetery")
    area:set_approved_min_by_area(14, 1e8,
                                  10, 1e10,
                                   8, -1)


  elseif area:has_tag("landuse", "brownfield") or
         area:has_tag("landuse", "greenfield") or
         area:has_tag("landuse", "construction") then
    area:set_target_layer("construction")
    area:set_approved_min_by_area(14, 1e8,
                                  10, 1e10,
                                   8, -1)

  elseif area:has_tag("natural", "water") or
         area:has_tag("waterway", "riverbank") or
         area:has_tag("waterway", "basin") or
         area:has_tag("waterway", "pond") or
         area:has_tag("leisure", "swimming_pool") then
    area:set_target_layer("water")
    area:set_approved_min_by_area(12, 1e6,
                                  10, 1e4,
                                   0, -1)

  elseif area:has_tag("natural", "beach") then
    area:set_target_layer("landuse")
    area:add_string("landuse", "beach")
    area:set_approved_min_by_area(14, 1e8,
                                  10, 1e10,
                                   8, -1)

  elseif area:has_tag("highway", "pedestrian") or
         area:has_tag("highway", "service") or
         area:has_tag("amenity", "parking") then
    area:set_target_layer("pedestrian")
    area:set_approved_min_by_area(12, 1e8,
                                  10, 1e10,
                                  8, -1)

  elseif area:has_tag("leisure", "pitch") then
    area:set_target_layer("sport")
    area:set_approved_min_by_area(14, 1e8,
                                  12, 1e10,
                                  8, -1)
  end
end
