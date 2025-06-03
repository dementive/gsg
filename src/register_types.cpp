#include "register_types.h"

#include "core/string/print_string.h"
#include "nodes/Map3D.hpp"
#include "templates/SingletonAllocator.hpp"
#include "data/Province.hpp"
#include "data/Area.hpp"
#include "data/Region.hpp"
#include "data/Country.hpp"
#include "data/Locator.hpp"

#include "defs/Entity.hpp"

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

static void update(Registry &registry) {
    auto view = registry.view<const Name>();

    for(auto [entity, name]: view.each()) {
    	print_line(vformat("Entity: %d name: %s", static_cast<int>(entity), name));
    	//registry.destroy(entity);
    	// view.get<Name>(entity);
    }

    auto provinces_view = registry.view<EntityTag::Province>();
    for(const EntityType::Province entity: provinces_view) {
    	print_line(vformat("Territory Entity: %d name: %s", static_cast<int>(entity), registry.get<Name>(entity)));
    	//registry.destroy(entity);
    	// view.get<Name>(entity);
    }
}

void initialize_src_module(ModuleInitializationLevel p_level) {
	if (p_level != MODULE_INITIALIZATION_LEVEL_SCENE)
		return;


	singleton_allocator.init();

	Registry *registry = new Registry;

	// create provinces
	for(int i = 0; i < 10; ++i) {
		const EntityType::Province entity = registry->create_entity<EntityTag::Province>();
		registry->emplace<Name>(entity, "Ohio");
	}

	// create nations
	for(int i = 0; i < 10; ++i) {
		const EntityType::Country entity = registry->create_entity<EntityTag::Country>();
		registry->emplace<Name>(entity, "Rome");
	}

	update(*registry);

	delete registry;

	GDREGISTER_RUNTIME_CLASS(Map3D)
}

void uninitialize_src_module(ModuleInitializationLevel p_level) {
	if (p_level != MODULE_INITIALIZATION_LEVEL_SCENE)
		return;

	singleton_allocator.free();
}
