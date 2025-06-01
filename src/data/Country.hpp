#pragma once

#include "core/templates/local_vector.h"
#include "defs/soa.hpp"

namespace CG {

struct Country {
	Singleton(Country)
	FixedSizeSOA(
		Country,4,
		String, name,
		LocalVector<ProvinceEntity>, owned_provinces,
		ProvinceEntity, capital,
		Color, color
	)
};

}