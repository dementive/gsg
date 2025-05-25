#pragma once

#include "core/templates/local_vector.h"
#include "core/variant/variant.h"
#include "defs/soa.hpp"

namespace CG {

enum class ProvinceType : uint8_t {
	Land=0,
	Ocean=1,
	Lake=2,
	River=3, // navigable river
	Impassable=4, // colored in some map modes
	Uninhabitable=5 // never colored
};

enum class ProvinceAdjacencyType : uint8_t {
	Water=0, // both provinces are water
	Land=1, // both provinces are land
	Coastal=2, // one province is water and the other is land
	Impassable=3, // both provinces are land but movement between them is impossible
	Crossing=4, // both provinces are land but the adjacency crosses over a water province
};

enum class ProvinceBorderType : uint8_t {
	Country=0, // border between 2 countries
	Area=1, // border between 2 areas
	Province=2, // border between 2 land provinces in the same area
	Impassable=3, // border between anything and an impassable province
	Water=4, // border between 2 water provinces
	Coastal=5, // border between a water and land province
	PROVINCE_BORDER_TYPE_MAX=6
};

inline ProvinceType province_type_string_to_enum(const String &p_string) {
	if (p_string == "land") {
		return ProvinceType::Land;
	} else if (p_string == "ocean") {
		return ProvinceType::Ocean;
	} else if (p_string == "river") {
		return ProvinceType::River;
	} else if (p_string == "impassable") {
		return ProvinceType::Impassable;
	} else if (p_string == "uninhabitable") {
		return ProvinceType::Uninhabitable;
	}

	return ProvinceType::Land;
}

inline bool is_impassable_province(ProvinceType p_type) {
	return true ? p_type == ProvinceType::Impassable or p_type == ProvinceType::Uninhabitable : false;
}

inline bool is_navigable_water_province(ProvinceType p_type) {
	return true ? p_type == ProvinceType::Ocean or p_type == ProvinceType::River : false;
}

struct Provinces {
	FixedSizeSOA(8,
		LocalVector<ProvinceType>, type,
		String, name, // loc key
		Vector2, centroid, // mean point in province
		float, orientation, // angle in radians
		Entity, owner, // id of owning country
		Entity, area, // area id
		LocalVector<Entity>, adjacencies, // ProvinceAdjacency ids
		LocalVector<Entity>, borders // ProvinceBorder ids
	)

	void initialize(Entity p_size) {
		init(p_size);

		Entity *ptr = owner.ptr();
		for (Entity i = 0; i < p_size; ++i) {
			ptr[i] = ENTITY_MAX;
		}
	}
};

}