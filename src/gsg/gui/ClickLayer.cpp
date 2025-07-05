#include "ClickLayer.hpp"

#include "core/input/input_event.h"
#include "core/string/print_string.h"

#include "scene/3d/camera_3d.h"
#include "scene/main/viewport.h"

#include "ecs/components.hpp"
#include "ecs/ecs.hpp"

#include "MapUnit.hpp"

using namespace CG;

void ClickLayer::_notification(int p_what) {
	switch (p_what) {
		case NOTIFICATION_READY: {
			set_process_unhandled_input(true);
		} break;
		case CanvasItem::NOTIFICATION_DRAW: {
			if (!selecting)
				return;

			draw_rect(select_box, Color(1, 1, 1, 1), false, 2.0);
		} break;
	}
}

void ClickLayer::unhandled_input(const Ref<InputEvent> &p_event) {
	const Ref<InputEventMouseButton> mb = p_event;
	const Ref<InputEventMouseMotion> mm = p_event;

	if (mb.is_valid() and mb->get_button_index() == MouseButton::LEFT) {
		if (mb->is_pressed()) {
			selecting = true;
			drag_start = mb->get_position();
		} else {
			selecting = false;
			if (drag_start.is_equal_approx(mb->get_position()))
				select_box = Rect2(mb->get_position(), Vector2(0, 0));
			update_selected_units<true>();
			queue_redraw();
		}
	} else if (selecting and mm.is_valid()) {
		const float x_min = MIN(drag_start.x, mm->get_position().x);
		const float y_min = MIN(drag_start.y, mm->get_position().y);
		select_box = Rect2(x_min, y_min, MAX(drag_start.x, mm->get_position().x) - x_min, MAX(drag_start.y, mm->get_position().y) - y_min);
		update_selected_units<false>();
		queue_redraw();
	}
}

template void ClickLayer::update_selected_units<false>();
template void ClickLayer::update_selected_units<true>();

template <bool is_click> void ClickLayer::update_selected_units() {
	const CountryEntity player = ECS::self->get<Player>();
	const RelationEntity unit_relation = ECS::self->get_relation(Relation::Unit);

	UnitEntity unit;
	int idx{};
	Vector3 pos;
	Viewport *vp = get_viewport();

	if constexpr (is_click) {
		const Camera3D *camera = vp->get_camera_3d();
		const Vector2 mouse_position = vp->get_mouse_position();

		const Vector3 origin = camera->project_ray_origin(mouse_position);
		const Vector3 direction = camera->project_ray_normal(mouse_position);

		const float distance = -origin.y / direction.y;
		const Vector3 xz_pos = origin + direction * distance;
		pos = Vector3(xz_pos.x, 15.0, xz_pos.z);
	}

	while ((unit = player.target(unit_relation, idx++))) {
		MapUnit *unit_mesh = unit.get_mut<UnitModel>().ptr();

		if constexpr (is_click) {
			const AABB aabb = unit_mesh->get_global_transform().xform(unit_mesh->get_aabb());
			if (aabb.has_point(pos)) {
				unit_mesh->select();
				vp->set_input_as_handled();
			} else if (select_box.get_size() == Vector2(0, 0)) {
				unit_mesh->deselect();
			}
		} else if (unit_mesh->is_inside_selection_box(select_box)) {
			unit_mesh->select();
			vp->set_input_as_handled();
		} else {
			unit_mesh->deselect();
		}
	}
}
