#pragma once

#include "core/string/ustring.h"

#include "Entity.hpp"
#include "Vec.hpp"

namespace CG {

#define MAKE_SAME(m_class, m_type, m_name)                                                                                                                                                   \
	m_type m_name;                                                                                                                                                                           \
	m_class(const m_type &p_##m_name) : m_name(p_##m_name) {}                                                                                                                                \
	operator m_type &() { return m_name; }                                                                                                                                                   \
	operator const m_type &() const { return m_name; }

struct Name : String {};
struct Centroid : Vector2 {};
struct Orientation {
	MAKE_SAME(Orientation, float, value)
};

// Provinces in an area
struct AreaProvinces : TightVec<ProvinceEntity> {};

// Areas in a region
struct RegionAreas : TightVec<AreaEntity> {};

// Country owned provinces
struct OwnedProvinces : Vec<ProvinceEntity> {};

struct Owner {
	MAKE_SAME(Owner, CountryEntity, entity)
};

struct Capital {
	MAKE_SAME(Capital, ProvinceEntity, entity)
};

struct AreaComponent {
	MAKE_SAME(AreaComponent, AreaEntity, entity)
};

struct RegionComponent {
	MAKE_SAME(RegionComponent, RegionEntity, entity)
};

// Province Adjacency/Border components
struct AdjacencyTo {
	MAKE_SAME(AdjacencyTo, ProvinceEntity, entity)
};

struct AdjacencyFrom {
	MAKE_SAME(AdjacencyFrom, ProvinceEntity, entity)
};

struct CrossingLocator {
	MAKE_SAME(CrossingLocator, Vector4, locator)
};

struct ProvinceBorderMeshRID : RID {};

// Each province's adjacencies and borders
struct ProvinceAdjacencies : TightVec<ProvinceAdjacencyEntity> {};
struct ProvinceBorders : TightVec<ProvinceBorderEntity> {};

#undef MAKE_SAME

} // namespace CG
