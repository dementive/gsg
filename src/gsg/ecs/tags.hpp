#pragma once

namespace CG {

/* Tags for every entity type */
struct ProvinceTag {};
struct CountryTag {};
struct RegionTag {};
struct AreaTag {};
struct UnitTag {};

/* Province type tags */
struct LandProvinceTag {};
struct OceanProvinceTag {};
struct LakeProvinceTag {};
struct RiverProvinceTag {};
struct ImpassableProvinceTag {};
struct UninhabitableProvinceTag {};

// Special country tags
static constexpr const char *OBSERVER_TAG = "OBSERVER_NATION";

} // namespace CG
