#include "Map3D.hpp"

#include "core/input/input_event.h"

#include "scene/3d/camera_3d.h"
#include "scene/3d/mesh_instance_3d.h"
#include "scene/main/viewport.h"
#include "scene/resources/3d/world_3d.h"

#include "cg/Map.hpp"

#include "ecs/Registry.hpp"

using namespace CG;

void Map3D::_bind_methods() {}

void Map3D::_notification(int p_what) {
	switch (p_what) {
		case NOTIFICATION_READY: {
			set_process_unhandled_input(true);
			Map::self = memnew(Map);
			Map::self->load_map<false>(this);
			map_mesh = Object::cast_to<MeshInstance3D>(get_node(NodePath("%MapMesh")));

			const Ref<ShaderMaterial> material = map_mesh->get_mesh()->surface_get_material(0);
			material->set_shader_parameter("color_texture", Map::self->get_country_map_mode());
			material->set_shader_parameter("lookup_texture", Map::self->get_lookup_texture());
		} break;
		case NOTIFICATION_EXIT_TREE: {
			memdelete_notnull(Map::self);
		} break;
	}
}

void Map3D::unhandled_input(const Ref<InputEvent> &p_event) {
	const Ref<InputEventMouseButton> input_event_mouse_button = p_event;
	if (!input_event_mouse_button.is_valid() or !input_event_mouse_button->is_pressed() or input_event_mouse_button->get_button_index() != MouseButton::LEFT)
		return;

	Viewport *vp = get_viewport();
	const Camera3D *camera = vp->get_camera_3d();
	const Vector2 mouse_position = vp->get_mouse_position();
	const Vector3 origin = camera->project_ray_origin(mouse_position);
	const Vector3 direction = camera->project_ray_normal(mouse_position);

	const float distance = -origin.y / direction.y;
	const Vector3 xz_pos = origin + direction * distance;
	const Vector3 pos = Vector3(xz_pos.x, 0.0, xz_pos.z);
	const Vector2i click_position = Vector2i(Math::round(pos.x), Math::round(pos.z));

	// Ignore clicks outside the map
	if (click_position.x > map_dimensions.x or click_position.x < 0 or click_position.y > map_dimensions.y or click_position.y < 0)
		return;

	const Color province_color = Map::self->get_lookup_image()->get_pixelv(click_position);
	const ProvinceIndex province_id = Map::self->get_color_to_id_map().get(province_color);
	if (province_id == 0)
		return;

	const ProvinceEntity province_entity = Registry::self->get_entity<ProvinceTag>(province_id);
	if (Registry::self->all_of<LandProvinceTag>(province_entity)) {
		const Ref<ShaderMaterial> material = map_mesh->get_mesh()->surface_get_material(0);
		material->set_shader_parameter("selected_area", province_color.linear_to_srgb());
		vp->set_input_as_handled();
	}
}
