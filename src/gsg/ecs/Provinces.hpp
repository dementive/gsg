#pragma once

#include <cstdint>

#include "ecs_entity.hpp"
#include "ecs_tags.hpp"

namespace CG {

enum class ProvinceAdjacencyType : uint8_t {
	Water = 0, // both provinces are water
	Land = 1, // both provinces are land
	Coastal = 2, // one province is water and the other is land
	Impassable = 3, // both provinces are land but movement between them is impossible
	Crossing = 4, // both provinces are land but the adjacency crosses over a water province
};

enum class ProvinceBorderType : uint8_t {
	Country = 0, // border between 2 countries
	Area = 1, // border between 2 areas
	Province = 2, // border between 2 land provinces in the same area
	Impassable = 3, // border between anything and an impassable province
	Water = 4, // border between 2 water provinces
	Coastal = 5, // border between a water and land province
	PROVINCE_BORDER_TYPE_MAX = 6
};

inline bool is_navigable_water_province(const Entity p_entity) { return p_entity.has<OceanProvinceTag>() or p_entity.has<RiverProvinceTag>(); }

inline bool is_impassable_province(const Entity p_entity) { return p_entity.has<ImpassableProvinceTag>() or p_entity.has<UninhabitableProvinceTag>(); }

} // namespace CG