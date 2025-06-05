#pragma once

#include "Vec.hpp"
#include "core/string/ustring.h"
#include "Entity.hpp"

namespace CG {

struct Name : String {};

struct AreaProvinces : TightVec<Entity> {};
struct RegionAreas : TightVec<Entity> {};
struct OwnedProvinces : Vec<Entity> {};

struct Owner {
	CountryEntity entity;

	Owner(const CountryEntity &p_entity) : entity(p_entity) {}
	operator CountryEntity &() { return entity; }
	operator const CountryEntity &() const { return entity; }
};

struct Capital {
	ProvinceEntity entity;

	Capital(const ProvinceEntity &p_entity) : entity(p_entity) {}
	operator ProvinceEntity &() { return entity; }
	operator const ProvinceEntity &() const { return entity; }
};

struct AreaComponent {
	AreaEntity entity;

	AreaComponent(const AreaEntity &p_entity) : entity(p_entity) {}
	operator AreaEntity &() { return entity; }
	operator const AreaEntity &() const { return entity; }
};

struct RegionComponent {
	RegionEntity entity;

	RegionComponent(const RegionEntity &p_entity) : entity(p_entity) {}
	operator RegionEntity &() { return entity; }
	operator const RegionEntity &() const { return entity; }
};


}
