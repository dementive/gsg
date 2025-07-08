#ifdef TOOLS_ENABLED

#include "MapEditor.hpp"

#include "core/input/input_event.h"
#include "core/io/resource_loader.h"
#include "core/os/memory.h"

#include "scene/3d/mesh_instance_3d.h"
#include "scene/gui/box_container.h"
#include "scene/gui/button.h"
#include "scene/gui/control.h"
#include "scene/gui/flow_container.h"
#include "scene/gui/item_list.h"
#include "scene/gui/scroll_container.h"
#include "scene/gui/separator.h"
#include "editor/editor_dock_manager.h"
#include "editor/editor_inspector.h"
#include "editor/editor_interface.h"
#include "editor/editor_main_screen.h"
#include "editor/editor_node.h"
#include "editor/editor_settings.h"
#include "editor/plugins/node_3d_editor_plugin.h"

#include "cg/Locator.hpp"
#include "cg/Map.hpp"
#include "cg/MapUtils.hpp"

#include "ecs/components.hpp"

using namespace CG;

/* MapEditorNode */

void MapEditorNode::load_map() {
	if (has_loaded_map)
		return;

	Map::self = memnew(Map);
	Map::self->load_map_editor(this);

	map_mesh = Object::cast_to<MeshInstance3D>(get_node(NodePath("%MapMesh")));
	memnew(EditorLocators());

	has_loaded_map = true;
}

void MapEditorNode::_notification(int p_what) {
	switch (p_what) {
		case NOTIFICATION_EXIT_TREE: {
			memdelete_notnull(Map::self);
			MapEditorNode::has_loaded_map = false;
			MapEditorPlugin::self->map_editor->clear_item_lists();
		} break;
	}
}

/* MapEditorSprite */

void MapEditorSprite::init(LocatorType p_locator_type) { locator_type = p_locator_type; }

void MapEditorSprite::_notification(int p_what) {
	switch (p_what) {
		case NOTIFICATION_TRANSFORM_CHANGED: {
			if (EditorLocators::self != nullptr)
				EditorLocators::self->set_dirty(locator_type);
		} break;
	}
}

/* MapEditorLabel */

void MapEditorLabel::init(LocatorType p_locator_type) { locator_type = p_locator_type; }

void MapEditorLabel::_notification(int p_what) {
	switch (p_what) {
		case NOTIFICATION_TRANSFORM_CHANGED: {
			if (EditorLocators::self != nullptr)
				EditorLocators::self->set_dirty(locator_type);
		} break;
	}
}

/* MapEditorPlugin */

void MapEditor::clear_item_lists() {
	node_item_list->clear();
	province_inspector_item_list->clear();
}

void MapEditor::_locator_button_toggled(bool p_toggled) {
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

void MapEditor::_province_selection_button_toggled(bool p_toggled) { province_selection_enabled = p_toggled; }

void MapEditor::province_inspector_item_list_multi_selected(int p_index, bool p_selected) {
	if (p_selected) {
		MapEditorPlugin::self->select_province(p_index + 1);
		on_province_selected(p_index + 1);
	} else {
		MapEditorPlugin::self->deselect_province(p_index + 1);
		on_province_deselected(p_index + 1);
	}
}

void MapEditor::province_inspector_item_list_node_selected(int p_index) {
	Object *obj = node_item_list->get_item_metadata(p_index);
	if (obj == nullptr)
		return;

	Node3D *node = Object::cast_to<Node3D>(obj);
	if (node == nullptr)
		return;

	EditorInterface::get_singleton()->edit_node(node);
	map_object_toolbar_province_selection_button->set_pressed(false);
}

void MapEditor::create_unit_locator(int p_province_entity) {
	if (!EditorLocators::self->has_locator(LocatorType::Unit, p_province_entity))
		return;

	const Locator locator = EditorLocators::self->get_locator(LocatorType::Unit, p_province_entity);
	MapEditorSprite *sprite = memnew(MapEditorSprite());
	sprite->set_texture(ResourceLoader::load("res://gfx/icon.svg"));
	sprite->set_alpha_cut_mode(Sprite3D::AlphaCutMode::ALPHA_CUT_DISCARD);
	sprite->set_draw_flag(Sprite3D::DrawFlags::FLAG_DOUBLE_SIDED, false);
	sprite->set_draw_flag(Sprite3D::DrawFlags::FLAG_SHADED, true);

	MapEditorPlugin::map_editor_node->add_child(sprite, false, InternalMode::INTERNAL_MODE_BACK);
	sprite->set_owner(MapEditorPlugin::map_editor_node);

	sprite->set_rotation(Vector3(unit_x_rotation, locator.orientation, 0));
	sprite->set_scale(Vector3(locator.scale, locator.scale, locator.scale));
	sprite->set_global_position(Vector3(locator.position.x, unit_map_layer, locator.position.y));

	edited_unit_nodes[p_province_entity] = sprite;
	const int item_index = node_item_list->add_item(String(uitos(p_province_entity) + ":    Unit"));
	node_item_list->set_item_metadata(item_index, sprite);
}

void MapEditor::create_text_locator(int p_province_entity) {
	if (!EditorLocators::self->has_locator(LocatorType::Text, p_province_entity))
		return;

	const Locator locator = EditorLocators::self->get_locator(LocatorType::Text, p_province_entity);
	MapEditorLabel *label = memnew(MapEditorLabel());

	MapEditorPlugin::map_editor_node->add_child(label, false, InternalMode::INTERNAL_MODE_BACK);
	label->set_owner(MapEditorPlugin::map_editor_node);

	const ProvinceEntity entity = ECS::self->scope_lookup(Scope::Province, uitos(p_province_entity));
	label->set_text(tr(entity.get<LocKey>()));
	label->set_draw_flag(Label3D::FLAG_DOUBLE_SIDED, false);
	label->set_modulate(Color(0, 0, 0, 0.93));
	label->set_outline_modulate(Color(0, 0, 0, 0));
	label->set_vertical_alignment(VerticalAlignment::VERTICAL_ALIGNMENT_TOP); // must be same as MapLabel

	label->set_rotation(Vector3(-1.570796, locator.orientation, 0));
	label->set_scale(Vector3(locator.scale, locator.scale, locator.scale));
	label->set_global_position(Vector3(locator.position.x, label_map_layer, locator.position.y));

	edited_label_nodes[p_province_entity] = label;
	const int item_index = node_item_list->add_item(String(uitos(p_province_entity) + ":    Text"));
	node_item_list->set_item_metadata(item_index, label);
}

void MapEditor::on_province_selected(int p_province_entity) {
	// Load the locators for this province and place nodes on the map in their positions so they can be manipulated.
	create_unit_locator(p_province_entity);
	create_text_locator(p_province_entity);
}

void MapEditor::_on_province_deselected(int p_province_entity, LocatorType p_locator_type) {
	Node3D *node{};
	switch (p_locator_type) {
		case LocatorType::Unit: {
			node = edited_unit_nodes[p_province_entity];
		} break;
		case LocatorType::Text: {
			node = edited_label_nodes[p_province_entity];
		} break;
	}

	if (node != nullptr) {
		const Locator new_locator{
			.position = Vector2(node->get_global_position().x, node->get_global_position().z),
			.orientation = node->get_rotation().y,
			.scale = node->get_scale().x,
		};
		const Locator &old_locator = EditorLocators::self->get_locator(p_locator_type, p_province_entity);

		if (new_locator != old_locator) {
			EditorLocators::self->set_locator(p_locator_type, p_province_entity, new_locator);
			EditorLocators::self->save(p_locator_type);
		}

		for (int i = 0; i < node_item_list->get_item_count(); ++i) {
			Object *obj = node_item_list->get_item_metadata(i);
			if (obj == node)
				node_item_list->remove_item(i);
		}

		Node *owner = node->get_owner();
		if (owner != nullptr) {
			EditorInterface::get_singleton()->edit_node(MapEditorPlugin::map_editor_node);
			owner->remove_child(node);
		}
		memdelete(node);
	}

	switch (p_locator_type) {
		case LocatorType::Unit: {
			edited_unit_nodes.erase(p_province_entity);
		} break;
		case LocatorType::Text: {
			edited_label_nodes.erase(p_province_entity);
		} break;
	}
}

void MapEditor::on_province_deselected(int p_province_entity) {
	_on_province_deselected(p_province_entity, LocatorType::Unit);
	_on_province_deselected(p_province_entity, LocatorType::Text);
}

void MapEditor::add_tool_button(Button *p_button, const String &p_tooltip_text) {
	p_button->set_flat(true);
	p_button->set_tooltip_text(p_tooltip_text);
	p_button->set_toggle_mode(true);
	p_button->set_h_size_flags(SIZE_SHRINK_END);
}

void MapEditor::init_province_inspector_dock() {
	EditorDockManager::get_singleton()->add_dock(province_inspector_dock, "Province Inspector", EditorDockManager::DOCK_SLOT_RIGHT_UL, nullptr, "GridMinimap");
	EditorDockManager::get_singleton()->focus_dock(province_inspector_dock);
}

void MapEditor::remove_province_inspector_dock() { EditorDockManager::get_singleton()->remove_dock(province_inspector_dock); }

void MapEditor::_notification(int p_what) {
	switch (p_what) {
		case NOTIFICATION_THEME_CHANGED: {
			map_object_button->set_button_icon(get_editor_theme_icon("EditorPosition"));
			map_object_toolbar_province_selection_button->set_button_icon(get_editor_theme_icon("EditPivot"));
		} break;
		case NOTIFICATION_EDITOR_POST_SAVE: {
			EditorLocators::self->save_all();
		} break;
	}
}

void MapEditor::hide() {
	sidebar_container->hide();
	is_visible = false;

	map_object_toolbar_container->hide();
	if (active_toolbar == ActiveToolbar::MapObject)
		remove_province_inspector_dock();
}

void MapEditor::show() {
	sidebar_container->show();

	if (!is_visible and active_toolbar == ActiveToolbar::MapObject) {
		map_object_toolbar_container->show();
		init_province_inspector_dock();
	}

	is_visible = true;
}

bool MapEditor::is_province_selection_enabled() const { return province_selection_enabled; }

void MapEditor::on_map_province_selected(const int p_province_entity) {
	province_inspector_item_list->select(p_province_entity - 1, false);
	on_province_selected(p_province_entity);
}

void MapEditor::on_map_province_deselected(int p_province_entity) {
	province_inspector_item_list->deselect(p_province_entity - 1);
	on_province_deselected(p_province_entity);
}

void MapEditor::deselect_all_map_provinces() {
	for (const int i : province_inspector_item_list->get_selected_items())
		on_map_province_deselected(province_inspector_item_list->get_item_text(i).to_int());
	province_inspector_item_list->deselect_all();
}

void MapEditor::on_3d_viewport_gui_input(const Ref<InputEvent> &p_event) {
	const auto tool_mode = Node3DEditor::get_singleton()->get_tool_mode();
	if (is_province_selection_enabled() and
			(tool_mode == Node3DEditor::TOOL_MODE_SELECT or tool_mode == Node3DEditor::TOOL_MODE_MOVE or tool_mode == Node3DEditor::TOOL_MODE_SCALE or
					tool_mode == Node3DEditor::TOOL_MODE_ROTATE)) {
		map_object_toolbar_province_selection_button->set_pressed(false);
	}
}

MapEditor::MapEditor() :
		province_inspector_dock(memnew(VBoxContainer)),
		province_inspector_item_list(memnew(ItemList)),
		node_item_list(memnew(ItemList)),
		sidebar_container(memnew(VFlowContainer)),
		map_object_toolbar_container(memnew(HFlowContainer)) {
	// Setup left panel toolbar
	map_object_button = memnew(Button());
	map_object_button->connect("toggled", callable_mp(this, &MapEditor::_locator_button_toggled));
	add_tool_button(map_object_button, "Map Objects");

	sidebar_container->set_custom_minimum_size(Vector2(20, 0));
	sidebar_container->add_child(map_object_button);

	Node3DEditor::get_singleton()->add_control_to_left_panel(sidebar_container);

	// Hack to detect when a toolbar button is pressed.
	// Node3DEditor doens't expose any way to add new toolbar buttons in a way that keeps only 1 selected when editing.
	// but can connect to it's gui_input signal, check the tool_mode, and then deselect all new toolbar buttons when the edit mode changes.
	Node3DEditor::get_singleton()->connect("gui_input", callable_mp(this, &MapEditor::on_3d_viewport_gui_input));

	// Setup object selection toolbar
	map_object_toolbar_province_selection_button = memnew(Button());
	add_tool_button(map_object_toolbar_province_selection_button, "Province Selection");
	map_object_toolbar_province_selection_button->connect("toggled", callable_mp(this, &MapEditor::_province_selection_button_toggled));

	// have to use ED_SHORTCUT, any other way of making shorcuts doens't work in the editor with toggle buttons I guess.
	map_object_toolbar_province_selection_button->set_shortcut(ED_SHORTCUT("", TTRC("Province Selection"), Key::S));

	map_object_toolbar_container->add_child(map_object_toolbar_province_selection_button);

	map_object_toolbar_container->set_custom_minimum_size(Vector2(20, 0));
	Node3DEditor::get_singleton()->add_control_to_menu_panel(map_object_toolbar_container);

	// Setup province inspector dock
	VBoxContainer *province_inspector_vbox = memnew(VBoxContainer());
	ScrollContainer *province_inspector_scroll_container = memnew(ScrollContainer());
	ScrollContainer *node_scroll_container = memnew(ScrollContainer());
	HSeparator *ps_hsep = memnew(HSeparator());

	// Selection item list
	province_inspector_item_list->connect("multi_selected", callable_mp(this, &MapEditor::province_inspector_item_list_multi_selected));
	province_inspector_item_list->set_select_mode(ItemList::SELECT_TOGGLE);
	province_inspector_item_list->set_max_columns(5);
	province_inspector_item_list->set_allow_search(true);
	province_inspector_item_list->set_h_size_flags(Control::SIZE_EXPAND_FILL);
	province_inspector_item_list->set_v_size_flags(Control::SIZE_EXPAND_FILL);
	province_inspector_item_list->add_theme_constant_override("h_separation", 15);
	province_inspector_scroll_container->add_child(province_inspector_item_list);

	province_inspector_scroll_container->set_follow_focus(true);
	province_inspector_scroll_container->set_horizontal_scroll_mode(ScrollContainer::SCROLL_MODE_DISABLED);
	province_inspector_scroll_container->set_custom_minimum_size(Vector2(0, 250.0));
	province_inspector_vbox->add_child(province_inspector_scroll_container);

	province_inspector_vbox->add_child(ps_hsep);

	// Node item list
	node_item_list->connect("item_selected", callable_mp(this, &MapEditor::province_inspector_item_list_node_selected));
	node_item_list->set_h_size_flags(Control::SIZE_EXPAND_FILL);
	node_item_list->set_v_size_flags(Control::SIZE_EXPAND_FILL);
	node_item_list->set_select_mode(ItemList::SELECT_SINGLE);
	node_item_list->set_max_columns(1);

	node_scroll_container->set_follow_focus(true);
	node_scroll_container->set_horizontal_scroll_mode(ScrollContainer::SCROLL_MODE_DISABLED);
	node_scroll_container->set_custom_minimum_size(Vector2(0, 250.0));

	node_scroll_container->add_child(node_item_list);
	province_inspector_vbox->add_child(node_scroll_container);

	province_inspector_vbox->set_anchors_and_offsets_preset(PRESET_FULL_RECT);
	province_inspector_dock->add_child(province_inspector_vbox);
	province_inspector_dock->add_child(memnew(HSeparator));
}

// Stuff that can't happen in the constructor because the map data hasn't been loaded yet.
void MapEditor::on_map_loaded() {
	// Fill province item list with entity ids
	for (uint32_t i = 1; i < Map::self->get_color_to_id_map().size() + 1; ++i)
		province_inspector_item_list->add_item(itos(i));
}

MapEditor::~MapEditor() { memdelete_notnull(province_inspector_dock); }

/* MapEditorPlugin */

String MapEditorPlugin::get_plugin_name() const { return "MapEditor"; }
bool MapEditorPlugin::has_main_screen() const { return false; }

void MapEditorPlugin::select_province(const int p_province_id) {
	const ProvinceColorMap &map = Map::self->get_color_to_id_map();

	// Find key from value and apply it to the shader
	for (const KeyValue<Color, int> &kv : map) {
		if (kv.value == p_province_id) {
			const Color srgb_province_color = kv.key.linear_to_srgb();
			selected_areas.push_back(srgb_province_color);

			const Ref<ShaderMaterial> material = map_editor_node->map_mesh->get_mesh()->surface_get_material(0);
			material->set_shader_parameter("selected_areas", selected_areas);
			material->set_shader_parameter("selected_areas_total", MAX(10, selected_areas.size()));

			return;
		}
	}
}

void MapEditorPlugin::deselect_province(int p_province_id) {
	const ProvinceColorMap &map = Map::self->get_color_to_id_map();

	// Find key from value and apply it to the shader
	for (const KeyValue<Color, int> &kv : map) {
		if (kv.value == p_province_id) {
			const Color &srgb_province_color = kv.key.linear_to_srgb();
			selected_areas.erase(srgb_province_color);

			const Ref<ShaderMaterial> material = map_editor_node->map_mesh->get_mesh()->surface_get_material(0);
			material->set_shader_parameter("selected_areas", selected_areas);
			material->set_shader_parameter("selected_areas_total", MAX(10, selected_areas.size()));

			return;
		}
	}
}

EditorPlugin::AfterGUIInput MapEditorPlugin::forward_3d_gui_input(Camera3D *p_camera, const Ref<InputEvent> &p_event) {
	if (!map_editor->is_province_selection_enabled())
		return EditorPlugin::AFTER_GUI_INPUT_PASS;

	const Ref<InputEventMouseButton> mb = p_event;

	if (!mb.is_valid())
		return EditorPlugin::AFTER_GUI_INPUT_PASS;

	const Point2 mouse_position(mb->get_position().x, mb->get_position().y);

	Node3DEditorViewport *viewport = nullptr;
	for (uint32_t i = 0; i < Node3DEditor::VIEWPORTS_COUNT; i++) {
		Node3DEditorViewport *vp = Node3DEditor::get_singleton()->get_editor_viewport(i);
		if (vp->get_camera_3d() == p_camera) {
			viewport = vp;
			break;
		}
	}

	ERR_FAIL_NULL_V(viewport, EditorPlugin::AFTER_GUI_INPUT_PASS);

	if (mb->is_pressed() && mb->get_button_index() == MouseButton::LEFT) {
		const Vector2i click_position = get_map_click_position(p_camera, mouse_position);

		// Ignore clicks outside the map
		if (click_position.x > map_dimensions.x or click_position.x < 0 or click_position.y > map_dimensions.y or click_position.y < 0)
			return AFTER_GUI_INPUT_CUSTOM;

		const Color province_color = Map::self->get_lookup_image()->get_pixelv(click_position);
		const ProvinceIndex province_id = Map::self->get_color_to_id_map().get(province_color);
		if (province_id == 0)
			return AFTER_GUI_INPUT_CUSTOM;

		const Color srgb_province_color = province_color.linear_to_srgb();

		// If not holding shift only allow selection of 1 province
		if (!mb->is_shift_pressed()) {
			// If already selected and pressed again remove from selected areas.
			if (selected_areas.has(srgb_province_color)) {
				selected_areas.erase(srgb_province_color);
				map_editor->on_map_province_deselected(province_id);
			} else {
				map_editor->deselect_all_map_provinces();
				selected_areas.clear();
				selected_areas.push_back(srgb_province_color);
				map_editor->on_map_province_selected(province_id);
			}
		} else {
			// If already selected and pressed again remove from selected areas.
			if (selected_areas.has(srgb_province_color)) {
				selected_areas.erase(srgb_province_color);
				map_editor->on_map_province_deselected(province_id);
			} else {
				selected_areas.push_back(srgb_province_color);
				map_editor->on_map_province_selected(province_id);
			}
		}

		const Ref<ShaderMaterial> material = map_editor_node->map_mesh->get_mesh()->surface_get_material(0);
		material->set_shader_parameter("selected_areas", selected_areas);
		material->set_shader_parameter("selected_areas_total", MAX(10, selected_areas.size()));

		return EditorPlugin::AFTER_GUI_INPUT_CUSTOM;
	}

	return EditorPlugin::AFTER_GUI_INPUT_PASS;
}

bool MapEditorPlugin::handles(Object *p_object) const {
	MapEditorNode *scene_root = Object::cast_to<MapEditorNode>(EditorInterface::get_singleton()->get_edited_scene_root());
	if (scene_root != nullptr) {
		if (!MapEditorNode::has_loaded_map) {
			scene_root->load_map(); // Has to be here because to use Map::load_map the MapEditorNode is needed.
			map_editor->on_map_loaded();
		}

		map_editor_node = scene_root;
		return true;
	}

	return false;
};

void MapEditorPlugin::make_visible(bool p_visible) {
	if (p_visible)
		map_editor->show();
	else
		map_editor->hide();
};

MapEditorPlugin::MapEditorPlugin() :
		map_editor(memnew(MapEditor)) {
	if (self == nullptr)
		self = this;
	EditorNode::get_singleton()->get_gui_base()->add_child(map_editor);
	map_editor->hide();
};

#endif
