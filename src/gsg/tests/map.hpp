#pragma once
#ifdef TOOLS_ENABLED

#include "SFT.hpp"
#include "flecs/ecs.hpp"

#include "cg/Map.hpp"
#include "ecs/components.hpp"

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
		"get_name", ECS::self->find("c::JP1").name() == "JP1",
		"player_name", ECS::self->get<Player>().value.name() == "OBSERVER_NATION"
	)
}

test(map) {
	test_province();
	test_country();

	const CountryEntity owner = ECS::self->get_target(ECS::self->find("p::2"), Relation::Owner);
	Map::self->set_player(owner);

	named_tests(
		"Map", 
		"set_player", ECS::self->get<Player>().value.name() == "JP1"
	)
}

}

#endif