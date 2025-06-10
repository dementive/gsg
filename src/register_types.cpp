#include "register_types.h"

#include "cg/MapCamera.hpp"

#include "ecs/Registry.hpp"

#include "templates/SingletonAllocator.hpp"

#include "nodes/Map3D.hpp"

#ifdef TOOLS_ENABLED

#include "tools/MapEditor.hpp"

#endif

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

#ifdef TOOLS_ENABLED
	GDREGISTER_CLASS(MapEditorNode)
	EditorPlugins::add_by_type<MapEditorPlugin>();
#endif
}

void uninitialize_src_module(ModuleInitializationLevel p_level) {
	if (p_level != MODULE_INITIALIZATION_LEVEL_SCENE)
		return;

	entt_singleton_allocator.free();
}
