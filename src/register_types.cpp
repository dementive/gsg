#include <algorithm>

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
ProvinceLocator
> singleton_allocator;

SingletonAllocator<
// Game data
Registry
> entt_singleton_allocator;
}
// clang-format on

void initialize_src_module(ModuleInitializationLevel p_level) {
	if (p_level != MODULE_INITIALIZATION_LEVEL_SCENE)
		return;


	singleton_allocator.init();
	entt_singleton_allocator.init();

	GDREGISTER_RUNTIME_CLASS(Map3D)
}

void uninitialize_src_module(ModuleInitializationLevel p_level) {
	if (p_level != MODULE_INITIALIZATION_LEVEL_SCENE)
		return;

	singleton_allocator.free();
	entt_singleton_allocator.free();
}
