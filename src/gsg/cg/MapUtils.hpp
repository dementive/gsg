#pragma once

#include "scene/3d/camera_3d.h"
namespace CG {

static const Vector2i map_dimensions{ 1024, 1024 }; // TODO don't hardcode.

inline Vector2 get_map_click_position(const Camera3D *p_camera, Vector2 p_mouse_position) {
	const Vector3 origin = p_camera->project_ray_origin(p_mouse_position);
	const Vector3 direction = p_camera->project_ray_normal(p_mouse_position);

	const float distance = -origin.y / direction.y;
	const Vector3 xz_pos = origin + direction * distance;
	const Vector3 pos = Vector3(xz_pos.x, 0.0, xz_pos.z);
	return { Math::round(pos.x), Math::round(pos.z) };
}

} // namespace CG