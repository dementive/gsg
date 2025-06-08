#pragma once

#include "scene/3d/node_3d.h"

#include "defs/Bind.hpp"

class InputEvent;
class Camera3D;

namespace CG {

class MapCamera : public Node3D {
	GDCLASS(MapCamera, Node3D)
	GUI_NODE(camera, Camera3D)
	GUI_NODE(camera_socket, Node3D)

private:
	// Camera Bounds
	int camera_max_distance = 250000;

	// Movement
	uint16_t camera_move_speed = 1500;

	// Zoom
	uint16_t camera_zoom_speed = 4500;
	uint32_t camera_zoom_min = 15000;
	uint32_t camera_zoom_max = 280000;

	// Panning
	uint16_t pan_speed = 1500;
	uint16_t mouse_pan_speed = 60;
	Vector2 fixed_mouse_point = Vector2(0, 0);

	Vector2 mouse_last_position = Vector2(0, 0);
	Vector3 velocity_direction = Vector3(0, 0, 0);
	float camera_zoom_direction = 0.0;
	int8_t camera_rotation_direction = 0;
	bool is_mouse_inside = true;
	inline static bool edge_scrolling = true; // static so it effects all MapCamera instances

protected:
	static void _bind_methods();
	void _notification(int p_what);

public:
	void reset();

	PROP(camera_max_distance, int)
	PROP(camera_move_speed, int)
	PROP(camera_zoom_speed, int)
	PROP(camera_zoom_min, int)
	PROP(camera_zoom_max, int)
	PROP(pan_speed, int)
	PROP(mouse_pan_speed, int)

	void unhandled_input(const Ref<InputEvent> &event) final;

	void camera_base_move();
	void camera_zoom_update();
	void camera_rotate_to_mouse_offsets();
	void camera_base_rotate();
	void camera_socket_rotate(float direction);
	void camera_base_rotate_left_right(float direction);
	void camera_automatic_pan();
	void pan_camera();
	static void set_edge_scrolling(bool checked);
};

} // namespace CG
