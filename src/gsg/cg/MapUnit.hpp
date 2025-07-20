#pragma once

#include "scene/3d/sprite_3d.h"

namespace CG {

class MapUnit : public Sprite3D {
	GDCLASS(MapUnit, Sprite3D)

protected:
	static void _bind_methods() {}

public:
	void select();
	void deselect();

private:
	bool selected = false;
};

} // namespace CG
