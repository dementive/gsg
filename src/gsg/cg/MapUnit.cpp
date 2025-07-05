#include "MapUnit.hpp"

#include "scene/3d/camera_3d.h"
#include "scene/main/viewport.h"

using namespace CG;

bool MapUnit::is_inside_selection_box(const Rect2 &p_box) {
	Camera3D *camera = get_viewport()->get_camera_3d();
	const Vector2 point = camera->unproject_position(get_global_position());

	return p_box.has_point(point);
}

void MapUnit::select() {
	if (selected)
		return;
	RS::get_singleton()->material_set_param(get_material(), "albedo", Color(0, 1, 0, 1));
	selected = true;
}

void MapUnit::deselect() {
	if (!selected)
		return;
	RS::get_singleton()->material_set_param(get_material(), "albedo", Color(1, 1, 1, 1));
	selected = false;
}
