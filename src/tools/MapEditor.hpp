#pragma once

#include "core/object/callable_method_pointer.h"
#include "core/string/print_string.h"

#include "scene/gui/box_container.h"
#include "scene/gui/button.h"
#include "scene/gui/control.h"
#include "scene/gui/flow_container.h"
#include "scene/gui/label.h"
#include "scene/gui/separator.h"
#include "editor/editor_dock_manager.h"
#include "editor/editor_main_screen.h"
#include "editor/editor_node.h"
#include "editor/plugins/editor_plugin.h"
#include "editor/plugins/node_3d_editor_plugin.h"

#include "singleton.hpp"

namespace CG {

#ifdef TOOLS_ENABLED

class MapEditorNode : public Node3D {
	GDCLASS(MapEditorNode, Node3D)

protected:
	static void _bind_methods() {}
};

class MapEditor : public Control {
	GDCLASS(MapEditor, Control)

private:
	VBoxContainer *province_inspector_dock{};

	VFlowContainer *sidebar_container{};
	HFlowContainer *map_object_toolbar_container{};

	Button *map_object_button{};
	Button *map_object_toolbar_province_selection_button{};

	enum class ActiveToolbar : uint8_t { None, MapObject };

	ActiveToolbar active_toolbar = ActiveToolbar::None;

	void _locator_button_toggled(bool p_toggled) {
		print_line("Pressed locator button.");
		if (!p_toggled) {
			active_toolbar = ActiveToolbar::None;
			map_object_toolbar_container->hide();
			remove_province_inspector_dock();
			return;
		}

		active_toolbar = ActiveToolbar::MapObject;
		map_object_toolbar_container->show();
		init_province_inspector_dock();
	}

	void _province_selection_button_toggled(bool p_toggled) { print_line("Pressed province selection button."); }

	static void add_tool_button(Button *p_button, const String &p_tooltip_text) {
		p_button->set_flat(true);
		p_button->set_tooltip_text(p_tooltip_text);
		p_button->set_toggle_mode(true);
		p_button->set_h_size_flags(SIZE_SHRINK_END);
	}

	void init_province_inspector_dock() {
		EditorDockManager::get_singleton()->add_dock(province_inspector_dock, "Province Inspector", EditorDockManager::DOCK_SLOT_RIGHT_UL, nullptr, "GridMinimap");
		EditorDockManager::get_singleton()->focus_dock(province_inspector_dock);
	}

	void remove_province_inspector_dock() { EditorDockManager::get_singleton()->remove_dock(province_inspector_dock); }

protected:
	static void _bind_methods() {}
	void _notification(int p_what) {
		switch (p_what) {
			case NOTIFICATION_THEME_CHANGED: {
				map_object_button->set_button_icon(get_editor_theme_icon(SNAME("EditorPosition")));
				map_object_toolbar_province_selection_button->set_button_icon(get_editor_theme_icon(SNAME("EditPivot")));
			} break;
		}
	}

public:
	void hide() {
		sidebar_container->hide();
		map_object_toolbar_container->hide();
		if (active_toolbar == ActiveToolbar::MapObject)
			remove_province_inspector_dock();
	}
	void show() {
		sidebar_container->show();

		if (active_toolbar == ActiveToolbar::MapObject) {
			map_object_toolbar_container->show();
			init_province_inspector_dock();
		}
	}

	MapEditor() : province_inspector_dock(memnew(VBoxContainer())), sidebar_container(memnew(VFlowContainer())), map_object_toolbar_container(memnew(HFlowContainer())) {
		sidebar_container->set_custom_minimum_size(Vector2(20, 0));

		// Setup left panel toolbar
		map_object_button = memnew(Button());
		map_object_button->connect("toggled", callable_mp(this, &MapEditor::_locator_button_toggled));
		add_tool_button(map_object_button, "Map Objects");
		sidebar_container->add_child(map_object_button);

		Node3DEditor::get_singleton()->add_control_to_left_panel(sidebar_container);

		// Setup object selection toolbar
		map_object_toolbar_province_selection_button = memnew(Button());
		add_tool_button(map_object_toolbar_province_selection_button, "Province Selection");
		map_object_toolbar_province_selection_button->connect("toggled", callable_mp(this, &MapEditor::_province_selection_button_toggled));
		map_object_toolbar_container->add_child(map_object_toolbar_province_selection_button);

		map_object_toolbar_container->set_custom_minimum_size(Vector2(20, 0));
		Node3DEditor::get_singleton()->add_control_to_menu_panel(map_object_toolbar_container);

		// Setup province inspector dock
		Label *bullshit = memnew(Label());
		bullshit->set_text("Province Label");

		province_inspector_dock->add_child(bullshit);
		province_inspector_dock->add_child(memnew(HSeparator));
	}

	~MapEditor() override { memdelete_notnull(province_inspector_dock); }
};

class MapEditor3D : public Node3DEditor {
	GDCLASS(MapEditor3D, Node3DEditor);
	SINGLETON(MapEditor3D)
};

class MapEditorPlugin : public EditorPlugin {
	GDCLASS(MapEditorPlugin, EditorPlugin);

	MapEditor *map_editor = nullptr;

public:
	String get_plugin_name() const final { return "MapEditor"; }
	bool has_main_screen() const final { return false; }

	bool handles(Object *p_object) const final {
		if (Object::cast_to<MapEditorNode>(p_object) != nullptr)
			return true;

		return false;
	};

	void make_visible(bool p_visible) final {
		if (p_visible)
			map_editor->show();
		else
			map_editor->hide();
	};

	MapEditorPlugin() : map_editor(memnew(MapEditor)) {
		EditorNode::get_singleton()->get_gui_base()->add_child(map_editor);
		map_editor->hide();
	};
	~MapEditorPlugin() override = default;
};

#endif // TOOLS_ENABLED

} // namespace CG