#include "MapUnit.hpp"

using namespace CG;

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
