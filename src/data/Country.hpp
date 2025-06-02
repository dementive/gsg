#pragma once

#include "templates/Vec.hpp"
#include "core/templates/a_hash_map.h"
#include "defs/soa.hpp"

namespace CG {

struct Country {
	Singleton(Country)
	MutableSOA(
		Country,4,
		String, name,
		Vec<ProvinceEntity>, owned_provinces,
		ProvinceEntity, capital,
		Color, color
	)
};

}