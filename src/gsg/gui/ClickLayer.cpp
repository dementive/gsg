#include "ClickLayer.hpp"

#include "core/input/input_event.h"

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
			// update_selected_units();
			queue_redraw();
		}
	} else if (selecting and mm.is_valid()) {
		const float x_min = MIN(drag_start.x, mm->get_position().x);
		const float y_min = MIN(drag_start.y, mm->get_position().y);
		select_box = Rect2(x_min, y_min, MAX(drag_start.x, mm->get_position().x) - x_min, MAX(drag_start.y, mm->get_position().y) - y_min);
		// update_selected_units();
		queue_redraw();
	}
}
