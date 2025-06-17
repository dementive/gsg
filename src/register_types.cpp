#include "register_types.h"

#include "core/object/class_db.h"

#include "cg/Locator.hpp"
#include "cg/MapCamera.hpp"

#include "ecs/Registry.hpp"

#include "Map.hpp"
#include "nodes/Map3D.hpp"

#ifdef TOOLS_ENABLED

#include "tools/MapEditor.hpp"

#endif

using namespace CG;

void initialize_src_module(ModuleInitializationLevel p_level) {
	if (p_level != MODULE_INITIALIZATION_LEVEL_SCENE)
		return;

	new Registry;

	GDREGISTER_RUNTIME_CLASS(Map3D)
	GDREGISTER_RUNTIME_CLASS(MapCamera)

#ifdef TOOLS_ENABLED
	GDREGISTER_CLASS(MapEditorNode)
	GDREGISTER_INTERNAL_CLASS(MapEditorSprite)
	GDREGISTER_INTERNAL_CLASS(MapEditorLabel)
	EditorPlugins::add_by_type<MapEditorPlugin>();
#endif
}

void uninitialize_src_module(ModuleInitializationLevel p_level) {
	if (p_level != MODULE_INITIALIZATION_LEVEL_SCENE)
		return;

	delete Registry::self;
	memdelete_notnull(Map::self);

#ifdef TOOLS_ENABLED
	memdelete_notnull(EditorLocators::self);
#endif
}
