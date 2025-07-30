#pragma once
#ifdef TOOLS_ENABLED

#include "SFT.hpp"
#include "flecs/ecs.hpp"

namespace CG {

test(province) {
	named_tests(
		"Province", 
		"get_name", ECS::self->find("p::1").name() == "1"
	)
}

test(country) {
	named_tests(
		"Country", 
		"get_name", ECS::self->find("c::JP1").name() == "JP1"
	)
}

test(map) {
	test_province();
	test_country();
}

}

#endif