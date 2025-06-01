#pragma once

#include "core/templates/local_vector.h"
#include "defs/soa.hpp"

namespace CG {

struct Region {
	Singleton(Region)
	FixedSizeSOA(
		Region,4,
		String, name,
		TightLocalVector<AreaEntity>, areas,
		ProvinceEntity, capital,
		Color, color
	)
};

}