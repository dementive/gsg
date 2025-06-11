#pragma once

#include "scene/3d/node_3d.h"
#include "editor/plugins/editor_plugin.h"

#include "Entity.hpp"

class MeshInstance3D;
class VBoxContainer;
class ItemList;
class VFlowContainer;
class HFlowContainer;

namespace CG {

#ifdef TOOLS_ENABLED

class MapEditorNode : public Node3D {
	GDCLASS(MapEditorNode, Node3D)

protected:
	static void _bind_methods() {}
	void _notification(int p_what);

public:
	MeshInstance3D *map_mesh{};
	bool has_loaded_map = false;

	void load_map();
};

class MapEditor : public Control {
	GDCLASS(MapEditor, Control)

private:
	VBoxContainer *province_inspector_dock{};
	ItemList *province_inspector_item_list{};

	VFlowContainer *sidebar_container{};
	HFlowContainer *map_object_toolbar_container{};

	Button *map_object_button{};
	Button *map_object_toolbar_province_selection_button{};

	enum class ActiveToolbar : uint8_t { None, MapObject };

	bool is_visible = true;
	bool province_selection_enabled = false;

	ActiveToolbar active_toolbar = ActiveToolbar::None;

	void _locator_button_toggled(bool p_toggled);
	void _province_selection_button_toggled(bool p_toggled);
	void province_inspector_item_list_multi_selected(int p_index, bool p_selected);
	static void add_tool_button(Button *p_button, const String &p_tooltip_text);
	void init_province_inspector_dock();
	void remove_province_inspector_dock();

protected:
	static void _bind_methods() {}
	void _notification(int p_what);

public:
	void hide();
	void show();

	bool is_province_selection_enabled() const;

	void on_map_province_selected(int p_province_entity);
	void deselect_all_map_provinces();
	void deselect_map_provinces(int p_province_entity);

	MapEditor();

	// Stuff that can't happen in the constructor because the map data hasn't been loaded yet.
	void on_map_loaded();

	~MapEditor() override;
};

class MapEditorPlugin : public EditorPlugin {
	GDCLASS(MapEditorPlugin, EditorPlugin);

private:
	MapEditor *map_editor{};
	static inline MapEditorNode *map_editor_node{};
	PackedColorArray selected_areas;

public:
	static inline MapEditorPlugin *self = nullptr;

	String get_plugin_name() const final;
	bool has_main_screen() const final;

	// Called when selecting a province in the item list selection
	void select_province(int p_province_id);
	void deselect_province(int p_province_id);

	EditorPlugin::AfterGUIInput forward_3d_gui_input(Camera3D *p_camera, const Ref<InputEvent> &p_event) final;
	bool handles(Object *p_object) const final;
	void make_visible(bool p_visible) final;

	MapEditorPlugin();
	~MapEditorPlugin() override = default;
};

#endif // TOOLS_ENABLED

} // namespace CG