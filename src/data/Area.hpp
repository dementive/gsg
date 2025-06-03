#pragma once

#include "defs/soa.hpp"
#include "templates/Vec.hpp"

namespace CG {

struct Area {
	SINGLETON(Area)
	FixedSizeSOA(
		Area,5,
		String, name,
		TightVec<ProvinceEntity>, provinces,
		RegionEntity, region,
		ProvinceEntity, capital,
		Color, color
	)
};

}