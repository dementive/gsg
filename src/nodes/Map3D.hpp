#pragma once

#include "scene/3d/node_3d.h"

class MeshInstance3D;

namespace CG {

enum class MapMode : uint8_t;

class Map3D : public Node3D {
	GDCLASS(Map3D, Node3D)

private:
	MeshInstance3D *map_mesh{};
	static inline Vector2i map_dimensions{ 1024, 1024 }; // TODO don't hardcode.

protected:
	static void _bind_methods();
	void _notification(int p_what);
	void unhandled_input(const Ref<InputEvent> &p_event) final;

public:
	void set_map_mode(MapMode p_map_mode);
};

} // namespace CG
