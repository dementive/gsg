#pragma once

#include "ecs/Provinces.hpp"
#include "flecs/ecs.hpp"
#include "components.hpp"

namespace CG {
	class MapUnit;
	struct UnitLocator;
	struct TextLocator;
	class LandAStar;
	class NavalAStar;
}

using namespace CG;

namespace ecs {

struct gsg {
    gsg(flecs::world& world) {
        world.module<gsg>();
 
 		// Register variants
 		world.component<LocKey>();
        world.component<Color>();
        world.component<AABB>();

 		// Register components
 		world.component<CrossingLocator>();
 		world.component<ProvinceBorderMeshRID>();
 		world.component<UnitLocator>();
 		world.component<TextLocator>();
 		world.component<ProvinceAdjacencyType>();
 		world.component<ProvinceBorderType>();
 		world.component<Player>();
 		world.component<Ptr<MapUnit>>();
 		world.component<Ptr<LandAStar>>();
 		world.component<Ptr<NavalAStar>>();

 		// Register tag components
 		world.component<AreaTag>();
 		world.component<CountryTag>();
 		world.component<RegionTag>();
 		world.component<ProvinceTag>();
 		world.component<UnitTag>();

 		world.component<LandProvinceTag>();
 		world.component<OceanProvinceTag>();
 		world.component<RiverProvinceTag>();
 		world.component<LakeProvinceTag>();
 		world.component<ImpassableProvinceTag>();
 		world.component<UninhabitableProvinceTag>();
 		world.component<InFogOfWar>();
 		world.component<Discovered>();

 		world.component<Selected>();

 		// Top level scope entities. Adding each entity types to these as children allows using ecs.lookup("p::1") syntax to lookup entities.
 		// These entities hold no data they just act as namespaces inside of flecs.
 		ECS::self->register_scopes();

 		// Create all relationship entities
 		ECS::self->register_relations();
    }
};

inline void import() {
	ECS::self->import<ecs::gsg>();
}

}
