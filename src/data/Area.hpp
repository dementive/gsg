#pragma once

#include "core/templates/local_vector.h"
#include "defs/soa.hpp"

namespace CG {

struct Area {
	Singleton(Area)
	FixedSizeSOA(
		Area,5,
		String, name,
		TightLocalVector<ProvinceEntity>, provinces,
		RegionEntity, region,
		ProvinceEntity, capital,
		Color, color
	)
};

}