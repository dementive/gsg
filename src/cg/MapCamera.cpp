#include "MapCamera.hpp"

#include "core/input/input.h"
#include "core/input/input_event.h"

#include "scene/3d/camera_3d.h"
#include "scene/main/node.h"
#include "scene/main/viewport.h"

using namespace CG;

#define CAMERA_ZOOM_SPEED_DAMP 0.92
#define CAMERA_SOCKET_ROTATION_MIN -1.57295 // -90.0 * (3.1459 / 180.0)
#define CAMERA_SOCKET_ROTATION_MAX -0.873 // -50.0 * (3.1459 / 180.0)
#define PAN_MARGIN 16
#define CAMERA_MOUSE_ROTATION_SPEED 0.003334
#define CAMERA_KEYBOARD_ROTATION_SPEED 0.042

void MapCamera::_bind_methods() {
	BIND_NODE(MapCamera, camera, Camera3D)
	BIND_NODE(MapCamera, camera_socket, Node3D)

	BIND_PROP(MapCamera, camera_max_distance, INT)
	BIND_PROP(MapCamera, camera_move_speed, INT)
	BIND_PROP(MapCamera, camera_zoom_speed, INT)
	BIND_PROP(MapCamera, camera_zoom_min, INT)
	BIND_PROP(MapCamera, camera_zoom_max, INT)
	BIND_PROP(MapCamera, pan_speed, INT)
	BIND_PROP(MapCamera, mouse_pan_speed, INT)
}

void MapCamera::reset() {
	set_position(Vector3(0, 90, 0));
	set_rotation(Vector3(0.0, 0.0, 0.0));

	camera_socket->set_rotation(Vector3(Math::deg_to_rad(-70.0), 0.0, 0.0));

	const Vector3 camera_position = camera->get_position();
	camera->set_position(Vector3(camera_position.x, camera_position.y, 600));
	camera->set_rotation(Vector3(0.0, 0.0, 0.0));
}

void MapCamera::_notification(int p_what) {
	switch (p_what) {
		case NOTIFICATION_READY: {
			set_process_mode(PROCESS_MODE_ALWAYS);
			set_process_unhandled_input(true);
			set_physics_process(true);
		} break;
		case NOTIFICATION_WM_MOUSE_ENTER: {
			is_mouse_inside = true;
		} break;
		case NOTIFICATION_WM_MOUSE_EXIT: {
			is_mouse_inside = false;
		} break;
		case NOTIFICATION_PHYSICS_PROCESS: {
			if (!camera->is_current())
				return;

			Viewport *viewport = get_viewport();
			const Vector2 mouse_position = viewport->get_mouse_position();
			const Rect2 visible_rect = viewport->get_visible_rect();
			if (mouse_position.x < 0 || mouse_position.y < 0 || mouse_position.x > visible_rect.size.x || mouse_position.y > visible_rect.size.y)
				return;

			camera_zoom_update();
			camera_rotate_to_mouse_offsets();
			camera_base_rotate();
			camera_base_move();

			if (Input::get_singleton()->is_action_just_pressed("camera_pan"))
				fixed_mouse_point = mouse_position;
			if (Input::get_singleton()->is_action_pressed("camera_pan"))
				pan_camera();
			else
				camera_automatic_pan(); // don't edge scroll when middle mouse panning
		} break;
	}
}

void MapCamera::unhandled_input(const Ref<InputEvent> &event) {
	// Camera zoom controls
	if (event->is_action("camera_zoom_in"))
		camera_zoom_direction = -1;
	else if (event->is_action("camera_zoom_out"))
		camera_zoom_direction = 1;

	// Keyboard camera rotation
	if (event->is_action_pressed("camera_rotate_right"))
		camera_rotation_direction = -1;
	else if (event->is_action_pressed("camera_rotate_left"))
		camera_rotation_direction = 1;
	else if (event->is_action_released("camera_rotate_right") or event->is_action_released("camera_rotate_left"))
		camera_rotation_direction = 0;

	// Mouse camera rotation
	if (event->is_action_pressed("camera_rotate"))
		mouse_last_position = get_viewport()->get_mouse_position();
	else if (event->is_action_released("camera_rotate"))
		mouse_last_position = Vector2(0, 0);

	// Movement
	if (event->is_action_released("camera_right") or event->is_action_released("camera_left") or event->is_action_released("camera_forward") or event->is_action_released("camera_backward"))
		velocity_direction = Vector3(0, 0, 0);
	if (Input::get_singleton()->is_action_pressed("camera_forward"))
		velocity_direction -= get_transform().get_basis().get_column(2);
	if (Input::get_singleton()->is_action_pressed("camera_backward"))
		velocity_direction += get_transform().get_basis().get_column(2);
	if (Input::get_singleton()->is_action_pressed("camera_right"))
		velocity_direction += get_transform().get_basis().get_column(0);
	if (Input::get_singleton()->is_action_pressed("camera_left"))
		velocity_direction -= get_transform().get_basis().get_column(0);

	// If holding both left+right or up+down don't do anything
	if ((Input::get_singleton()->is_action_pressed("camera_left") and Input::get_singleton()->is_action_pressed("camera_right")) or
			(Input::get_singleton()->is_action_pressed("camera_forward") and Input::get_singleton()->is_action_pressed("camera_backward"))) {
		velocity_direction = Vector3(0, 0, 0);
	}
}

void MapCamera::camera_base_move() {
	const Vector3 new_position = get_position() + velocity_direction.normalized() * camera_move_speed;
	if (new_position.distance_to(Vector3(0, 0, 0)) < camera_max_distance)
		set_position(new_position);
}

void MapCamera::camera_zoom_update() {
	Vector3 camera_position = camera->get_position();
	float new_zoom = camera_position.z + (camera_zoom_speed * camera_zoom_direction);
	new_zoom = CLAMP(new_zoom, camera_zoom_min, camera_zoom_max);

	camera_position.z = new_zoom;
	camera->set_position(camera_position);
	camera_zoom_direction *= CAMERA_ZOOM_SPEED_DAMP;
}

void MapCamera::camera_rotate_to_mouse_offsets() {
	if (mouse_last_position == Vector2(0, 0))
		return;

	Vector2 mouse_offset = get_viewport()->get_mouse_position();
	mouse_offset = mouse_offset - mouse_last_position;
	mouse_last_position = get_viewport()->get_mouse_position();

	Vector3 current_rotation = get_rotation();
	current_rotation.y += mouse_offset.x * CAMERA_MOUSE_ROTATION_SPEED;
	set_rotation(current_rotation);

	camera_socket_rotate(mouse_offset.y);
}

void MapCamera::camera_base_rotate() {
	if (camera_rotation_direction == 0)
		return;

	camera_base_rotate_left_right(camera_rotation_direction);
}

void MapCamera::camera_socket_rotate(const float direction) {
	float new_rotation_x = camera_socket->get_rotation().x;
	new_rotation_x -= direction * CAMERA_MOUSE_ROTATION_SPEED;
	new_rotation_x = CLAMP(new_rotation_x, CAMERA_SOCKET_ROTATION_MIN, CAMERA_SOCKET_ROTATION_MAX);

	Vector3 current_rotation = camera_socket->get_rotation();
	current_rotation.x = new_rotation_x;
	camera_socket->set_rotation(current_rotation);
}

void MapCamera::camera_base_rotate_left_right(const float direction) {
	Vector3 current_rotation = get_rotation();
	current_rotation.y += direction * CAMERA_KEYBOARD_ROTATION_SPEED;
	set_rotation(current_rotation);
}

void MapCamera::camera_automatic_pan() {
	if (!edge_scrolling)
		return;

	Viewport *current_viewport = get_viewport();
	Vector2 pan_direction = Vector2(-1, -1);
	const Rect2i viewport_visible_rect = Rect2i(current_viewport->get_visible_rect());
	const Vector2i viewport_size = viewport_visible_rect.get_size();
	const Vector2 current_mouse_position = current_viewport->get_mouse_position();

	if (!is_mouse_inside)
		return;

	const Vector3 current_position = get_position();

	// X Panning
	if (current_mouse_position.x < PAN_MARGIN || current_mouse_position.x > viewport_size.x - PAN_MARGIN) {
		if (current_mouse_position.x > viewport_size.x / 2.0)
			pan_direction.x = 1;

		const Vector3 new_position = current_position + Vector3(pan_direction.x * pan_speed, 0, 0);
		if (new_position.distance_to(Vector3(0, 0, 0)) <= camera_max_distance)
			translate(Vector3(pan_direction.x * pan_speed, 0, 0));
	}

	// Y Panning
	if (current_mouse_position.y < PAN_MARGIN || current_mouse_position.y > viewport_size.y - PAN_MARGIN) {
		if (current_mouse_position.y > viewport_size.y / 2.0)
			pan_direction.y = 1;

		const Vector3 new_position = current_position + Vector3(0, 0, pan_direction.y * pan_speed);
		if (new_position.distance_to(Vector3(0, 0, 0)) <= camera_max_distance)
			translate(Vector3(0, 0, pan_direction.y * pan_speed));
	}
}

void MapCamera::pan_camera() {
	const Vector2 position = get_viewport()->get_mouse_position();
	Vector3 new_position = get_position();
	new_position.x = new_position.x + (position.x - fixed_mouse_point.x) / mouse_pan_speed;
	new_position.z = new_position.z + (position.y - fixed_mouse_point.y) / mouse_pan_speed;

	if (new_position.distance_to(Vector3(0, 0, 0)) <= camera_max_distance)
		set_position(new_position);
}

void MapCamera::set_edge_scrolling(bool checked) { edge_scrolling = checked; }
