#include "register_types.h"

#include "cg/MapCamera.hpp"

#include "ecs/Registry.hpp"

#include "templates/SingletonAllocator.hpp"

#include "nodes/Map3D.hpp"

using namespace CG;

// clang-format off
namespace {
SingletonAllocator<
// Game data
Registry
> entt_singleton_allocator;
}
// clang-format on

void initialize_src_module(ModuleInitializationLevel p_level) {
	if (p_level != MODULE_INITIALIZATION_LEVEL_SCENE)
		return;

	entt_singleton_allocator.init();
	GDREGISTER_RUNTIME_CLASS(Map3D)
	GDREGISTER_RUNTIME_CLASS(MapCamera)
}

void uninitialize_src_module(ModuleInitializationLevel p_level) {
	if (p_level != MODULE_INITIALIZATION_LEVEL_SCENE)
		return;

	entt_singleton_allocator.free();
}
