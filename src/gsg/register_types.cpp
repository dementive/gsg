#include "register_types.h"

#include "cg/Locator.hpp"
#include "cg/MapCamera.hpp"

#include "gui/ClickLayer.hpp"
#include "gui/Hud.hpp"
#include "nodes/Map3D.hpp"

#ifdef TOOLS_ENABLED
#include "tools/MapEditor.hpp"
#endif

using namespace CG;

void initialize_gsg_module(ModuleInitializationLevel p_level) {
	if (p_level != MODULE_INITIALIZATION_LEVEL_SCENE)
		return;

	new ECS;

	GDREGISTER_RUNTIME_CLASS(Map3D)
	GDREGISTER_RUNTIME_CLASS(MapCamera)

	GDREGISTER_RUNTIME_CLASS(DataBind)
	GDREGISTER_RUNTIME_CLASS(Hud)
	GDREGISTER_CLASS(ClickLayer)

#ifdef TOOLS_ENABLED
	GDREGISTER_CLASS(MapEditorNode)
	GDREGISTER_INTERNAL_CLASS(MapEditorSprite)
	GDREGISTER_INTERNAL_CLASS(MapEditorLabel)
	EditorPlugins::add_by_type<MapEditorPlugin>();
#endif
}

void uninitialize_gsg_module(ModuleInitializationLevel p_level) {
	if (p_level != MODULE_INITIALIZATION_LEVEL_SCENE)
		return;

#ifdef TOOLS_ENABLED
	memdelete_notnull(EditorLocators::self);
#endif

	delete ECS::self;
}
