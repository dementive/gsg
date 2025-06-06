#pragma once

#include "entt/core/hashed_string.hpp"
using namespace entt::literals;

namespace CG {

template <uint32_t Value> using entity_tag = std::integral_constant<decltype(Value), Value>;

// Tags for every entity type
// By default entt doesn't have a great way to iterate over every entity of a certain type, all entities are always the same type unless they come from a different registry.
// Adding a unique tag to every entity allows getting all components with that tag to get all entites of a certain type.
using ProvinceTag = entity_tag<"Province"_hs>;
using CountryTag = entity_tag<"Country"_hs>;
using RegionTag = entity_tag<"Region"_hs>;
using AreaTag = entity_tag<"Area"_hs>;

template <typename T>
concept IsEntityTag = std::is_same_v<T, ProvinceTag> || std::is_same_v<T, CountryTag> || std::is_same_v<T, RegionTag> || std::is_same_v<T, AreaTag>;

// Province type tags
using LandProvinceTag = entity_tag<"LandProvince"_hs>;
using OceanProvinceTag = entity_tag<"OceanProvince"_hs>;
using LakeProvinceTag = entity_tag<"LakeProvince"_hs>;
using RiverProvinceTag = entity_tag<"RiverProvince"_hs>;
using ImpassableProvinceTag = entity_tag<"ImpassableProvince"_hs>;
using UninhabitableProvinceTag = entity_tag<"UninhabitableProvince"_hs>;

} // namespace CG