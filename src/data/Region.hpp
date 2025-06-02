#pragma once

#include "templates/Vec.hpp"
#include "defs/soa.hpp"

namespace CG {

struct Region {
	Singleton(Region)
	FixedSizeSOA(
		Region,4,
		String, name,
		TightVec<AreaEntity>, areas,
		ProvinceEntity, capital,
		Color, color
	)
};

}