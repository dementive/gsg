#include "Map3D.hpp"
#include "core/input/input_event.h"
#include "scene/main/viewport.h"
#include "scene/3d/camera_3d.h"
#include "scene/resources/3d/world_3d.h"

using namespace CG;

void Map3D::_bind_methods() {}

void Map3D::_notification(int p_what) {
	switch (p_what) {
		case NOTIFICATION_READY: {
			set_process_unhandled_input(true);
			// map = Map.new()
			// map.load_map(self)
			// map_mesh.get_mesh().surface_get_material(0).set_shader_parameter('color_texture', map.get_color_texture())
			// map_mesh.get_mesh().surface_get_material(0).set_shader_parameter('lookup_texture', map.get_lookup_texture())
		} break;
		case NOTIFICATION_WM_CLOSE_REQUEST: {
			// if map != null:
			// 	map.free()
		} break;
		case NOTIFICATION_EXIT_TREE: {
			// if map != null:
			// 	map.free()
		} break;
	}
}

void Map3D::unhandled_input(const Ref<InputEvent> &p_event) {
	const Ref<InputEventMouseButton> input_event_mouse_button = p_event;
	if (!input_event_mouse_button.is_valid() or !input_event_mouse_button->is_pressed() or input_event_mouse_button->get_button_index() != MouseButton::LEFT)
		return;

	Viewport *vp = get_viewport();
	Camera3D *camera = vp->get_camera_3d();
	Vector2 mouse_position = vp->get_mouse_position();
	Vector3 origin = camera->project_ray_origin(mouse_position);
	Vector3 direction = camera->project_ray_normal(mouse_position);

	float distance = -origin.y/direction.y;
	Vector3 xz_pos = origin + direction * distance;
	Vector3 pos = Vector3(xz_pos.x, 0.0, xz_pos.z);
	Vector2i click_position = Vector2i(Math::round(pos.x), Math::round(pos.z));

	// Ignore clicks outside the map
	if (click_position.x > map_dimensions.x or click_position.x < 0 or click_position.y > map_dimensions.y or click_position.y < 0)
		return;

	// Color province_color = map.get_lookup_image().get_pixelv(click_position);
	// int province_id = map.get_color_to_id_map().get(province_color);

	//if (!province_id)
	//	return;
	//Map::ProvinceType province_type = map.get_provinces().type[province_id]
	// if (province_type == Map::ProvinceType::Land) {
	// 	map_mesh.get_mesh().surface_get_material(0).set_shader_parameter('selected_area', province_color.linear_to_srgb());
	// 	vp->set_input_as_handled();
	// }
}
