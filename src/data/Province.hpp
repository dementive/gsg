#pragma once

#include "templates/Vec.hpp"
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

// Crossing over water between land provinces.
struct ProvinceCrossing {
	Singleton(ProvinceCrossing)
	FixedSizeSOA(
		ProvinceCrossing, 2,
		Entity, adjacency_entity,
		Vector4, crossing_locator // xy = start coord zw = end coord, read from crossings.txt
	)
};

struct ProvinceAdjacency {
	Singleton(ProvinceAdjacency)
	FixedSizeSOA(
		ProvinceAdjacency, 4,
		ProvinceAdjacencyType, type,
		ProvinceEntity, to,
		ProvinceEntity, from,
		ProvinceCrossingEntity, crossing
	)
};

struct ProvinceBorder {
	Singleton(ProvinceBorder)
	FixedSizeSOA(
		ProvinceBorder, 4,
		ProvinceBorderType, type,
		ProvinceEntity, to,
		ProvinceEntity, from,
		RID, rid
	)
};

struct Province {
	Singleton(Province)
	FixedSizeSOA(
		Province, 8,
		ProvinceType, type,
		String, name, // loc key
		Vector2, centroid, // mean point in province
		float, orientation, // angle in radians
		CountryEntity, owner, // id of owning country
		AreaEntity, area, // area id
		TightVec<ProvinceAdjacencyEntity>, adjacencies, // ProvinceAdjacency ids
		TightVec<ProvinceBorderEntity>, borders // ProvinceBorder ids
	)

	void initialize(Entity p_size) {
		init(p_size);

		CountryEntity *ptr = owner.ptr();
		for (Entity i = 0; i < p_size; ++i) {
			ptr[i] = ENTITY_MAX;
		}
	}

	ProvinceEntity size() const {
		return type.size();
	}
};

inline bool province_has_owner(ProvinceEntity p_entity) {
	return Province::self->get_owner(p_entity) == ENTITY_MAX ? true : false;
}

}