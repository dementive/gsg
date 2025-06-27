#pragma once

namespace CG {

/* Relationship tags */

// Country province owner
struct ProvinceOwner {};

// Province owned by a country
struct OwnedProvince {};

// Capital province of an area, region, or country
struct Capital {};

/* Tags for every entity type */
struct ProvinceTag {};
struct CountryTag {};
struct RegionTag {};
struct AreaTag {};

/* Province type tags */
struct LandProvinceTag {};
struct OceanProvinceTag {};
struct LakeProvinceTag {};
struct RiverProvinceTag {};
struct ImpassableProvinceTag {};
struct UninhabitableProvinceTag {};

} // namespace CG
