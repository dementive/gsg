#pragma once

#include "scene/main/canvas_item.h"

namespace CG {

class ClickLayer : public CanvasItem {
	GDCLASS(ClickLayer, CanvasItem)

protected:
	static void _bind_methods() {}
	void _notification(int p_what);
	void unhandled_input(const Ref<InputEvent> &p_event) final;

#ifdef TOOLS_ENABLED
public:
	void _edit_set_position(const Point2 &p_position) final {};
	Point2 _edit_get_position() const final { return {}; };
	void _edit_set_scale(const Size2 &p_scale) final {};
	Size2 _edit_get_scale() const final { return {}; };
	Transform2D get_transform() const final { return {}; };
#endif

private:
	bool selecting = false;
	Vector2 drag_start;
	Rect2 select_box;
};

} // namespace CG
