#include "register_types.h"

#include "core/string/print_string.h"
#include "nodes/Map3D.hpp"
#include "templates/SingletonAllocator.hpp"
#include "data/Province.hpp"
#include "data/Area.hpp"
#include "data/Region.hpp"
#include "data/Country.hpp"
#include "data/Locator.hpp"

#include <entt/entity/registry.hpp>

using namespace CG;

// clang-format off
namespace {
SingletonAllocator<
// Game data
ProvinceCrossing,
ProvinceAdjacency,
ProvinceBorder,
Province,
Country,
Area,
Region,
ProvinceLocator
> singleton_allocator;
}
// clang-format on

struct Name : String {};

using TerritoryEntity = entt::entity;

using NationEntity = entt::entity;

static void update(entt::registry &registry) {
    auto view = registry.view<const Name>();

    for(auto [entity, name]: view.each()) {
    	print_line(vformat("Entity: %d name: %s", static_cast<int>(entity), name));
    	//registry.destroy(entity);
    	// view.get<Name>(entity);
    }
}

// Stores the Entity ids of every entity type and wraps entt::registry.
struct EntityRegistry {
	Vec<TerritoryEntity> territories;
	entt::registry registry;
};

void initialize_src_module(ModuleInitializationLevel p_level) {
	if (p_level != MODULE_INITIALIZATION_LEVEL_SCENE)
		return;

	singleton_allocator.init();

	EntityRegistry entity_registry;
	entt::registry registry;
	entity_registry.territories.resize(10);

	// create provinces
	for(int i = 0; i < 10; ++i) {
		const TerritoryEntity entity = registry.create();
		registry.emplace<Name>(entity, "Ohio");
		entity_registry.territories[i] = entity;
	}

	// create nations
	for(int i = 0; i < 10; ++i) {
		const NationEntity entity = registry.create();
		registry.emplace<Name>(entity, "Rome");
	}

	for (const TerritoryEntity entity: entity_registry.territories) {
		print_line(vformat("Territory Entity: %d name: %s", static_cast<int>(entity), registry.get<Name>(entity)));
	}

	update(registry);

	GDREGISTER_RUNTIME_CLASS(Map3D)
}

void uninitialize_src_module(ModuleInitializationLevel p_level) {
	if (p_level != MODULE_INITIALIZATION_LEVEL_SCENE)
		return;

	singleton_allocator.free();
}
