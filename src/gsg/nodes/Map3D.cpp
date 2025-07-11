#include "Map3D.hpp"

#include "core/input/input_event.h"

#include "scene/3d/camera_3d.h"
#include "scene/3d/mesh_instance_3d.h"
#include "scene/main/viewport.h"
#include "scene/resources/3d/world_3d.h"

#include "cg/Map.hpp"
#include "cg/MapMode.hpp"
#include "cg/MapUtils.hpp"
#include "cg/NodeManager.hpp"

#include "ecs/components.hpp"
#include "ecs/tags.hpp"

#include "gui/Hud.hpp"

using namespace CG;

void Map3D::_bind_methods() {}

void Map3D::_notification(int p_what) {
	switch (p_what) {
		case NOTIFICATION_READY: {
			NM::map = this;
			NM::init_gui();

			// Add hud
			add_child(NM::hud->get_parent());

			// Load map
			set_process_unhandled_input(true);
			Map::self = memnew(Map);
			Map::self->load_map<false>(this);
			map_mesh = Object::cast_to<MeshInstance3D>(get_node(NodePath("%MapMesh")));

			const Ref<ShaderMaterial> material = map_mesh->get_mesh()->surface_get_material(0);

			material->set_shader_parameter("color_texture", Map::self->get_map_mode<MapMode::Country>());
			material->set_shader_parameter("lookup_texture", Map::self->get_lookup_texture());
		} break;
		case NOTIFICATION_EXIT_TREE: {
			NM::clear_temporary_nodes();
			memdelete_notnull(Map::self);
		} break;
	}
}

void Map3D::unhandled_input(const Ref<InputEvent> &p_event) {
	const Ref<InputEventMouseButton> mb = p_event;
	if (!mb.is_valid() or !mb->is_released() or (mb->get_button_index() != MouseButton::LEFT and mb->get_button_index() != MouseButton::RIGHT))
		return;

	Viewport *vp = get_viewport();
	const Camera3D *camera = vp->get_camera_3d();
	const Vector2 mouse_position = vp->get_mouse_position();
	const Vector2i click_position = get_map_click_position(camera, mouse_position);

	// Ignore clicks outside the map
	if (click_position.x > map_dimensions.x or click_position.x < 0 or click_position.y > map_dimensions.y or click_position.y < 0)
		return;

	const Color province_color = Map::self->get_lookup_image()->get_pixelv(click_position);
	const ProvinceIndex province_id = Map::self->get_color_to_id_map().get(province_color);
	if (province_id == 0)
		return;

	const ProvinceEntity province_entity = ECS::self->scope_lookup(Scope::Province, uitos(province_id));
	ECS &ecs = *ECS::self;
	if (mb->is_ctrl_pressed() and ecs.has_relation(province_entity, Relation::Owner)) {
		const CountryEntity owner = ecs.get_target(province_entity, Relation::Owner);
		const CountryEntity current_player = ecs.get<Player>();

		if (current_player == owner) {
			ecs.set<Player>(ecs.lookup(OBSERVER_TAG));
			print_line("Set player to: Observer");
		} else {
			ecs.set<Player>(owner);
			print_line("Set player to: ", owner.name().c_str());
		}

		vp->set_input_as_handled();
		return;
	}

	if (!province_entity.has<LandProvinceTag>())
		return;

	const Ref<ShaderMaterial> material = map_mesh->get_mesh()->surface_get_material(0);
	PackedColorArray selected_areas;

	if (mb->get_button_index() == MouseButton::RIGHT) {
		if (!ecs.has_relation(province_entity, Relation::Owner))
			return;

		const CountryEntity owner = ecs.get_target(province_entity, Relation::Owner);
		const RelationEntity province_relation = ecs.get_relation(Relation::Province);

		ProvinceEntity entity;
		int idx = 0;

		while ((entity = owner.target(province_relation, idx++))) {
			const int p_id = atoi(entity.name());
			const auto kv = Map::self->get_color_to_id_map().get_by_index(p_id - 1); // province ids start at 1
			selected_areas.push_back(kv.key.linear_to_srgb());
		}

		material->set_shader_parameter("selected_areas", selected_areas);
		material->set_shader_parameter("selected_areas_total", selected_areas.size());
		vp->set_input_as_handled();
		return;
	} else {
		selected_areas.push_back(province_color.linear_to_srgb());
		material->set_shader_parameter("selected_areas", selected_areas);
		material->set_shader_parameter("selected_areas_total", 1);
		vp->set_input_as_handled();
	}
}

void Map3D::set_map_mode(MapMode p_map_mode) {
	const Ref<ShaderMaterial> material = map_mesh->get_mesh()->surface_get_material(0);

	switch (p_map_mode) {
		case MapMode::Country: {
			material->set_shader_parameter("color_texture", Map::self->get_map_mode<MapMode::Country>());
		} break;
		case MapMode::Area: {
			material->set_shader_parameter("color_texture", Map::self->get_map_mode<MapMode::Area>());
		} break;
		case MapMode::Region: {
			material->set_shader_parameter("color_texture", Map::self->get_map_mode<MapMode::Region>());
		} break;
	}
}
