#@tool
extends Node3D
class_name Map3D

var map: Map
@onready var map_mesh: MeshInstance3D = %MapMesh
var map_dimensions: Vector2i = Vector2i(1024, 1024)

func _ready() -> void:
	map = Map.new()
	map.load_map(self)
	map_mesh.get_mesh().surface_get_material(0).set_shader_parameter('color_texture', map.get_color_texture())
	map_mesh.get_mesh().surface_get_material(0).set_shader_parameter('lookup_texture', map.get_lookup_texture())

func _notification(p_what: int) -> void:
	if p_what == NOTIFICATION_WM_CLOSE_REQUEST:
		if map != null:
			map.free()
	if p_what == NOTIFICATION_EXIT_TREE:
		if map != null:
			map.free()

func _unhandled_input(event: InputEvent) -> void:
	if !event.is_class("InputEventMouseButton"): return
	
	var mouse_event: InputEventMouseButton = event as InputEventMouseButton
	if !mouse_event.is_pressed(): return

	var vp: Viewport = get_viewport()
	var camera: Camera3D = vp.get_camera_3d()
	var mouse_position: Vector2 = vp.get_mouse_position()
	var origin: Vector3 = camera.project_ray_origin(mouse_position)
	var direction: Vector3 = camera.project_ray_normal(mouse_position)

	var distance: float = -origin.y/direction.y
	var xz_pos: Vector3 = origin + direction * distance
	var pos: Vector3 = Vector3(xz_pos.x, 0.0, xz_pos.z)
	@warning_ignore("narrowing_conversion")
	var click_position: Vector2i = Vector2i(pos.x, pos.z)

	# Don't care if click is outside the map
	if click_position.x > map_dimensions.x or click_position.x < 0 or click_position.y > map_dimensions.y or click_position.y < 0:
		return

	var province_color: Color = map.get_lookup_image().get_pixelv(click_position)
	var province_id: int = map.get_color_to_id_map().get(province_color)

	if !province_id:
		return

	var province_type: Map.ProvinceType = map.get_provinces().type[province_id]
	if province_type == Map.ProvinceType.Land:
		map_mesh.get_mesh().surface_get_material(0).set_shader_parameter('selected_area', province_color.linear_to_srgb())
		get_viewport().set_input_as_handled()
