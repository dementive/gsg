#pragma once

#ifdef TOOLS_ENABLED

#include "scene/3d/node_3d.h"
#include "editor/plugins/editor_plugin.h"

class MeshInstance3D;
class VBoxContainer;
class ItemList;
class VFlowContainer;
class HFlowContainer;
class Sprite3D;

template <typename T> class Ref;
class InputEvent;

namespace CG {

class MapEditorNode : public Node3D {
	GDCLASS(MapEditorNode, Node3D)

protected:
	static void _bind_methods() {}
	void _notification(int p_what);

public:
	MeshInstance3D *map_mesh{};
	static inline bool has_loaded_map = false;

	void load_map();
};

class MapEditor : public Control {
	GDCLASS(MapEditor, Control)

private:
	HashMap<int, Sprite3D *> edited_unit_nodes;

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

	void on_3d_viewport_gui_input(const Ref<InputEvent> &p_event);

protected:
	static void _bind_methods() {}
	void _notification(int p_what);

public:
	void hide();
	void show();

	bool is_province_selection_enabled() const;

	void on_map_province_selected(int p_province_entity);
	void on_map_province_deselected(int p_province_entity);
	void deselect_all_map_provinces();

	void on_province_selected(int p_province_entity);
	void on_province_deselected(int p_province_entity);

	MapEditor();
	~MapEditor() override;

	// Stuff that can't happen in the constructor because the map data hasn't been loaded yet.
	void on_map_loaded();
};

class MapEditorPlugin : public EditorPlugin {
	GDCLASS(MapEditorPlugin, EditorPlugin);

private:
	MapEditor *map_editor{};
	PackedColorArray selected_areas; // TODO - make this a FixedVector<Color, 10> when updating to 4.5. Right now it will proabbly causes bugs with more than 10 selections.

public:
	static inline MapEditorNode *map_editor_node{};
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
};

} // namespace CG

#endif // TOOLS_ENABLED