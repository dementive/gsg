#pragma once

#include "defs/soa.hpp"

namespace CG {

struct Locator {
	FixedSizeSOA(
		Locator,3,
		Vector2, position,
		float, orientation,
		float, scale
	)
};

struct ProvinceLocator {
	Singleton(ProvinceLocator)
	Locator text;
	Locator unit;

	void init(int p_size) {
		text.init(p_size);
		unit.init(p_size);
	}
};

}
