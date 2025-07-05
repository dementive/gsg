#pragma once

#include "scene/3d/sprite_3d.h"

namespace CG {

class MapUnit : public Sprite3D {
	GDCLASS(MapUnit, Sprite3D)

protected:
	static void _bind_methods() {}

public:
	bool is_inside_selection_box(const Rect2 &p_box);
	void select();
	void deselect();

private:
	bool selected = false;
};

} // namespace CG
