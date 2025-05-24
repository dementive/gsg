extends Object
class_name Map

const COLOR_TEXTURE_DIMENSIONS: int = 255
var color_to_id_map : Dictionary[Color, int] = {} # lookup image color -> province id
var look_up_image : Image

var provinces: Provinces
var province_adjacencies: ProvinceAdjacencies
var province_borders: ProvinceBorders
var countries: Countries
var areas: Areas
var regions: Regions

const border_map_layer: float = 0.01
const label_map_layer: float = 0.015
const INT_MAX: int = 2147483647

# Provinces with this color in the color texture will not have any special color draw in the map shader.
const discard_color: Color = Color(0,0,0)

enum ProvinceType {
	Land,
	Ocean,
	Lake,
	River, # navigable river
	Impassable, # colored in some map modes
	Uninhabitable # never colored
}

enum ProvinceAdjacencyType {
	Water, # both provinces are water
	Land, # both provinces are land
	Coastal, # one province is water and the other is land
	Impassable, # both provinces are land but movement between them is impossible
	Crossing, # both provinces are land but the adjacency crosses over a water province
}

enum ProvinceBorderType {
	Country=0, # border between 2 countries
	Area=1, # border between 2 areas
	Province=2, # border between 2 land provinces in the same area
	Impassable=3, # border between anything and an impassable province
	Water=4, # border between 2 water provinces
	Coastal=5, # border between a water and land province
	PROVINCE_BORDER_TYPE_MAX=6
}

func province_type_string_to_enum(p_string: String) -> ProvinceType:
	match p_string:
		"land": return ProvinceType.Land
		"ocean": return ProvinceType.Ocean
		"river": return ProvinceType.River
		"impassable": return ProvinceType.Impassable
		"uninhabitable": return ProvinceType.Uninhabitable

	return ProvinceType.Land

# Soa classes for main data types.
class Countries:
	var name: PackedStringArray = PackedStringArray() # loc key/tag
	var owned_provinces: Array[PackedInt32Array] = [] # array of province ids
	var color: PackedColorArray = PackedColorArray() # primary country color
	var capital: PackedInt32Array = PackedInt32Array() # capital province
	
	func resize(p_size: int) -> void:
		name.resize(p_size)
		owned_provinces.resize(p_size)
		color.resize(p_size)
		capital.resize(p_size)

class ProvinceAdjacencies:
	#var distance: PackedFloat32Array = PackedFloat32Array() # distance between province unit locators
	var type: Array[ProvinceAdjacencyType] = []
	var to: PackedInt32Array = PackedInt32Array() # province id 
	var from: PackedInt32Array = PackedInt32Array() # province id
	var crossing_locator: PackedVector4Array # xy = start coord zw = end coord, read from crossings.txt
	
	func resize(p_size: int) -> void:
		#distance.resize(p_size)
		type.resize(p_size)
		to.resize(p_size)
		from.resize(p_size)
		crossing_locator.resize(p_size)
	
class ProvinceBorders:
	var type: Array[ProvinceBorderType] = []
	var to: PackedInt32Array = PackedInt32Array() # province id 
	var from: PackedInt32Array = PackedInt32Array() # province id
	var rid: Array[RID] = [] # border mesh RID

	func resize(p_size: int) -> void:
		type.resize(p_size)
		to.resize(p_size)
		from.resize(p_size)
		rid.resize(p_size)

class Locator:
	var position: PackedVector2Array = PackedVector2Array()
	var orientation: PackedFloat32Array = PackedFloat32Array()
	var scale: PackedFloat32Array = PackedFloat32Array()

	func resize(p_size: int) -> void:
		scale.resize(p_size)
		orientation.resize(p_size)
		position.resize(p_size)

class ProvinceLocators:
	var text: Locator
	var unit: Locator

	func resize(p_size: int) -> void:
		if text == null:
			text = Locator.new() # can remove this in C++, have to have since 'class' in gdscript is RefCounted
		if unit == null:
			unit = Locator.new()
		
		text.resize(p_size)
		unit.resize(p_size)

class Provinces:
	var type: Array[ProvinceType] = []
	var name: PackedStringArray = PackedStringArray() # loc key
	var centroid: PackedVector2Array = PackedVector2Array() # mean point in province, used for label placement
	var orientation: PackedFloat32Array = PackedFloat32Array() # angle in radians
	var owner: PackedInt32Array = PackedInt32Array() # id of owning country
	var area: PackedInt32Array = PackedInt32Array() # area id
	var adjacencies: Array[PackedInt32Array] = [] # ProvinceAdjacency ids
	var borders: Array[PackedInt32Array] = [] # ProvinceBorder ids

	static func is_impassable(p_type: ProvinceType) -> bool:
		if p_type == ProvinceType.Impassable or p_type == ProvinceType.Uninhabitable:
			return true
		return false
		
	static func is_navigable_water(p_type: ProvinceType) -> bool:
		if p_type == ProvinceType.Ocean or p_type == ProvinceType.River:
			return true
		return false
		
	func has_owner(p_province_id: int) -> bool:
		return false if owner[p_province_id] == INT_MAX else true

	func resize(p_size: int) -> void:
		type.resize(p_size)
		name.resize(p_size)
		centroid.resize(p_size)
		orientation.resize(p_size)
		owner.resize(p_size)
		area.resize(p_size)
		adjacencies.resize(p_size)
		borders.resize(p_size)
		
		# init owners since not every province will always have an owner.
		for i: int in range(p_size):
			owner[i] = INT_MAX
		
	func size() -> int:
		return type.size()

class Areas:
	var name: PackedStringArray = PackedStringArray()
	var provinces: Array[PackedInt32Array] = []
	var region: PackedInt32Array = PackedInt32Array()
	var capital: PackedInt32Array = PackedInt32Array() # capital province
	var color: PackedColorArray = PackedColorArray()

	func resize(p_size: int) -> void:
		name.resize(p_size)
		provinces.resize(p_size)
		region.resize(p_size)
		capital.resize(p_size)
		color.resize(p_size)

class Regions:
	var name: PackedStringArray = PackedStringArray()
	var areas: Array[PackedInt32Array] = []
	var capital: PackedInt32Array = PackedInt32Array() # capital province
	var color: PackedColorArray = PackedColorArray()

	func resize(p_size: int) -> void:
		name.resize(p_size)
		areas.resize(p_size)
		capital.resize(p_size)
		color.resize(p_size)

func create_province_rect(polygon: PackedVector2Array) -> Rect2i:
	var min_x: float = INF
	var max_x: float = -INF
	var min_y: float = INF
	var max_y: float = -INF

	for point: Vector2 in polygon:
		min_x = min(min_x, point.x)
		max_x = max(max_x, point.x)
		min_y = min(min_y, point.y)
		max_y = max(max_y, point.y)
		
	return Rect2i(Vector2(min_x, min_y), Vector2(max_x - min_x, max_y - min_y))

func calculate_centroid(polygon: PackedVector2Array) -> Vector2:
	var sum: Vector2 = Vector2.ZERO
	for point: Vector2 in polygon:
		sum += point
	return sum / polygon.size()
	
func calculate_orientation(polygon: PackedVector2Array, centroid: Vector2) -> float:
	var mu11: float = 0.0
	var mu20: float = 0.0
	var mu02: float = 0.0
	for point: Vector2 in polygon:
		var x: float = point.x - centroid.x
		var y: float = point.y - centroid.y
		mu20 += x * x
		mu02 += y * y
		mu11 += x * y

	return 0.5 * atan2(2 * mu11, mu20 - mu02)

func get_random_color() -> Color:
	return Color(clamp(randf(), 0.3, 0.7), clamp(randf(), 0.3, 0.7), clamp(randf(), 0.3, 0.7))

func load_map_config() -> Dictionary:
	var provinces_map: Dictionary[Color, int] = {}

	provinces = Provinces.new()
	countries = Countries.new()
	areas = Areas.new()
	regions = Regions.new()

	var province_config: ConfigFile = ConfigFile.new()
	var area_config: ConfigFile = ConfigFile.new()
	var region_config: ConfigFile = ConfigFile.new()
	var country_config: ConfigFile = ConfigFile.new()

	if province_config.load("res://data/provinces.cfg") != OK: return provinces_map
	if country_config.load("res://data/countries.cfg") != OK: return provinces_map
	if region_config.load("res://data/regions.cfg") != OK: return provinces_map
	if area_config.load("res://data/areas.cfg") != OK: return provinces_map

	provinces.resize(province_config.get_sections().size())
	countries.resize(country_config.get_sections().size())
	areas.resize(area_config.get_sections().size())
	regions.resize(region_config.get_sections().size())

	for section: String in area_config.get_sections():
		var area_id: int = section.to_int()
		var name: String = area_config.get_value(section, "name")
		var color: Color = area_config.get_value(section, "color", get_random_color())
		var area_provinces: PackedInt32Array = area_config.get_value(section, "provinces")
		var capital: int = area_config.get_value(section, "capital")
		
		areas.capital[area_id] = capital
		areas.name[area_id] = name
		areas.provinces[area_id] = area_provinces
		areas.color[area_id] = color
		
		for province_id: int in area_provinces:
			provinces.area[province_id] = area_id

	for section: String in region_config.get_sections():
		var region_id: int = section.to_int()
		var name: String = region_config.get_value(section, "name")
		var color: Color = region_config.get_value(section, "color", get_random_color())
		var region_areas: PackedInt32Array = region_config.get_value(section, "areas")
		var capital: int = region_config.get_value(section, "capital")
		
		regions.capital[region_id] = capital
		regions.name[region_id] = name
		regions.areas[region_id] = region_areas
		regions.color[region_id] = color
		
		for area_id: int in region_areas:
			areas.region[area_id] = region_id

	for section: String in country_config.get_sections():
		var country_id: int = section.to_int()
		var name: String = country_config.get_value(section, "name")
		var color: Color = country_config.get_value(section, "color")
		var country_provinces: PackedInt32Array = country_config.get_value(section, "provinces")
		var capital: int = country_config.get_value(section, "capital")

		countries.name[country_id] = name
		countries.capital[country_id] = capital
		countries.owned_provinces[country_id] = country_provinces
		countries.color[country_id] = Color8(int(color.r), int(color.g), int(color.b))

		for province_id: int in country_provinces:
			var owner: int = provinces.owner[province_id]
			if owner != INT_MAX:
				printerr("Province ", province_id,  " has multiple owners. Provinces can only have one owner, fix countries.cfg.")
				continue

			provinces.owner[province_id] = country_id

	for section: String in province_config.get_sections():
		var map_color: Color = province_config.get_value(section, "color")
		map_color = Color8(int(map_color.r), int(map_color.g), int(map_color.b))

		var province_type: ProvinceType = province_type_string_to_enum(province_config.get_value(section, "type", "land"))
		var province_id: int = section.to_int()

		provinces.type[province_id] = province_type
		provinces.name[province_id] = String("PROV") + String(str(province_id))
		provinces_map[map_color] = province_id

		var lookup_color: Color = Color(
			float(int(province_id) % COLOR_TEXTURE_DIMENSIONS) / (COLOR_TEXTURE_DIMENSIONS - 1),
			floor(float(province_id) / COLOR_TEXTURE_DIMENSIONS) / (COLOR_TEXTURE_DIMENSIONS - 1),
			0.0
		)
		color_to_id_map[lookup_color] =  province_id

	return provinces_map

# Load provinces.png and province data and then create a lookup image from the data
func load_map(p_map: Map3D) -> void:
	var provinces_map: Dictionary[Color, int] = load_map_config() # province map color to province id
	var borders: Dictionary[PackedInt32Array, PackedVector4Array] = {} # should be Pair<int, int> instead of PackedInt32Array
	var pixel_dict: Dictionary[Color, PackedVector2Array] = {}
	
	#Load provinces.png
	var province_image: Image = preload("res://gfx/map/provinces.png").get_image()
	var width: int = province_image.get_width()
	var height: int = province_image.get_height()
	
	# Set Map3D node position, makes the world coords the same as the map coords
	p_map.set_position(Vector3(width/2.0, 0, height/2.0))

	# Create lookup texture
	look_up_image = Image.create_empty(width, height, false, Image.FORMAT_RGF);

	for x: int in range(width):
		for y: int in range(height):
			var current_color: Color = province_image.get_pixel(x, y)

			# Set lookup texture pixels
			var province_id: int = provinces_map[current_color];
			var lookup_color: Color = Color(
				(float(province_id % COLOR_TEXTURE_DIMENSIONS)/ (COLOR_TEXTURE_DIMENSIONS - 1)),
				(floor(float(province_id)/COLOR_TEXTURE_DIMENSIONS) / (COLOR_TEXTURE_DIMENSIONS - 1)),
				0.0
			);
			look_up_image.set_pixel(x, y, lookup_color);

			# Make pixel dict for polygon calculations
			if not pixel_dict.has(current_color):
				pixel_dict[current_color] = PackedVector2Array()
			pixel_dict[current_color].append(Vector2(x, y))

			# Get border segments
			if x + 1 < width:
				var right_color: Color = province_image.get_pixel(x + 1, y)
				if current_color != right_color:
					var key: PackedInt32Array = [provinces_map[current_color], provinces_map[right_color]]
					# Filter out borders and adjacencies with lakes
					# movement to/from lakes is impossible and borders should never be draw on lake provinces.
					if is_lake_border(key):
						continue
					key.sort() # sort to prevent duplicates
					if not borders.has(key):
						borders[key] = PackedVector4Array()
					borders[key].append(Vector4(x + 1, y, x + 1, y + 1))

			# Compare bottom neighbor if exists
			if y + 1 < height:
				var bottom_color: Color = province_image.get_pixel(x, y + 1)
				if current_color != bottom_color:
					var key: PackedInt32Array = [provinces_map[current_color], provinces_map[bottom_color]]
					if is_lake_border(key):
						continue
					key.sort()
					if not borders.has(key):
						borders[key] = PackedVector4Array()
					borders[key].append(Vector4(x, y + 1, x + 1, y + 1))

	# Fill in Provinces data from pixel data
	for color: Color in pixel_dict:
		var province_id: int = provinces_map[color]
		var pixels: PackedVector2Array = pixel_dict[color]
		var centroid: Vector2 = calculate_centroid(pixels)
		
		provinces.centroid[province_id] = centroid
		if provinces.type[province_id] != ProvinceType.Land:
			provinces.orientation[province_id] = 0.0
		else:
			provinces.orientation[province_id] = calculate_orientation(Geometry2D.convex_hull(pixels), centroid)

	create_map_labels(p_map, width, height)

	# For each border between province pairs, create one mesh with all line segments
	var border_keys: Array[PackedInt32Array] = borders.keys()
	var crossings: Array[Array] = CSV.parse_file("res://data/crossings.txt")
	province_adjacencies = ProvinceAdjacencies.new()
	province_borders = ProvinceBorders.new()
	
	province_adjacencies.resize(border_keys.size() + crossings.size())
	province_borders.resize(border_keys.size())

	# Fill in crossing adjacencies
	var province_adjacency_id: int = 0
	for crossing: Array in crossings:
		province_adjacencies.from[province_adjacency_id] = crossing[0]
		province_adjacencies.to[province_adjacency_id] = crossing[1]
		province_adjacencies.type[province_adjacency_id] = ProvinceAdjacencyType.Crossing
		#province_adjacencies.distance[province_adjacency_id] = Vector2()
		province_adjacencies.crossing_locator[province_adjacency_id] = Vector4(
			crossing[2], crossing[3], crossing[4], crossing[5]
		)
		province_adjacency_id += 1

	var border_materials: Array[ShaderMaterial] = create_border_materials()
	var border_id: int = 0
	for border_pair: PackedInt32Array in border_keys:
		var line_segments: PackedVector4Array = borders[border_pair]
		var border_mesh: Mesh = create_border_mesh(line_segments, 0.75, 0.75)
		var border_mesh_instance: MeshInstance3D = MeshInstance3D.new()

		var border_type: ProvinceBorderType = fill_province_border_data(border_id, border_pair)
		province_borders.rid[border_id] = border_mesh.get_rid()
		fill_province_adjacency_data(province_adjacency_id, border_pair)
		border_id += 1
		province_adjacency_id += 1

		border_mesh_instance.set_rotation_degrees(Vector3(90, 0, 0))
		border_mesh_instance.set_position(Vector3(-width / 2.0, border_map_layer, -height / 2.0))
		border_mesh_instance.mesh = border_mesh
		border_mesh.surface_set_material(0, border_materials[border_type])
		p_map.call_deferred(&"add_child", border_mesh_instance)

func is_lake_border(p_border_pair: PackedInt32Array) -> bool:
	var to: int = p_border_pair[0]
	var from: int = p_border_pair[1]
	var to_type: ProvinceType = provinces.type[to]
	var from_type: ProvinceType = provinces.type[from]
	if to_type == ProvinceType.Lake or from_type == ProvinceType.Lake:
		return true
	else:
		return false

func create_border_materials() -> Array[ShaderMaterial]:
	var border_shader: Shader = load("res://gfx/shaders/border.gdshader")
	var border_materials: Array[ShaderMaterial] = []
	border_materials.resize(ProvinceBorderType.PROVINCE_BORDER_TYPE_MAX)

	for border_type: int in range(ProvinceBorderType.PROVINCE_BORDER_TYPE_MAX):
		var border_material: ShaderMaterial = ShaderMaterial.new()
		border_material.set_shader(border_shader)
		match border_type:
			ProvinceBorderType.Country:
				border_material.set_shader_parameter("border_color", Color(0,0,0,1))
			ProvinceBorderType.Water:
				border_material.set_shader_parameter("border_color", Color(0,0,0,1))
			ProvinceBorderType.Impassable:
				border_material.set_shader_parameter("border_color", Color(0.423,0,0,0.878))
			ProvinceBorderType.Coastal:
				border_material.set_shader_parameter("border_color", Color(0.23,0.23,0.23,0.95))
			ProvinceBorderType.Area:
				border_material.set_shader_parameter("border_color", Color(0.26,0.26,0.26,1))
			ProvinceBorderType.Province:
				border_material.set_shader_parameter("border_color", Color(0.31,0.31,0.31,0.9))
		border_materials[border_type] = border_material
	
	return border_materials

func fill_province_adjacency_data(p_adjacency_id: int, p_border_pair: PackedInt32Array) -> void:
	var to: int = p_border_pair[0]
	var from: int = p_border_pair[1]
	var to_type: ProvinceType = provinces.type[to]
	var from_type: ProvinceType = provinces.type[from]
	var adjacency_type: ProvinceAdjacencyType = ProvinceAdjacencyType.Land
	
	if Provinces.is_navigable_water(to_type) and Provinces.is_navigable_water(from_type):
		adjacency_type = ProvinceAdjacencyType.Water
	elif Provinces.is_impassable(to_type) or Provinces.is_impassable(from_type):
		adjacency_type = ProvinceAdjacencyType.Impassable
	elif (to_type == ProvinceType.Ocean and from_type == ProvinceType.Land) or (to_type == ProvinceType.Land and from_type == ProvinceType.Ocean):
		adjacency_type = ProvinceAdjacencyType.Coastal

	province_adjacencies.from[p_adjacency_id] = to
	province_adjacencies.to[p_adjacency_id] = from
	province_adjacencies.type[p_adjacency_id] = adjacency_type
	#province_adjacencies.distance[p_adjacency_id] = Vector2()

func fill_province_border_data(p_border_id: int, p_border_pair: PackedInt32Array) -> ProvinceBorderType:
	var to: int = p_border_pair[0]
	var from: int = p_border_pair[1]
	var to_type: ProvinceType = provinces.type[to]
	var from_type: ProvinceType = provinces.type[from]
	var from_area: int = provinces.area[from]
	var to_area: int = provinces.area[to]
	var from_owner: int = provinces.area[from]
	var to_owner: int = provinces.area[to]
	var border_type: ProvinceBorderType = ProvinceBorderType.Country

	if (provinces.has_owner(to) and provinces.has_owner(from) and from_owner != to_owner):
		border_type = ProvinceBorderType.Country
	elif Provinces.is_navigable_water(to_type) and Provinces.is_navigable_water(from_type):
		border_type = ProvinceBorderType.Water
	elif Provinces.is_impassable(to_type) or Provinces.is_impassable(from_type):
		border_type = ProvinceBorderType.Impassable
	elif (to_type == ProvinceType.Ocean and from_type == ProvinceType.Land) or (to_type == ProvinceType.Land and from_type == ProvinceType.Ocean):
		border_type = ProvinceBorderType.Coastal
	elif from_area != to_area:
		border_type = ProvinceBorderType.Area
	elif from_area == to_area:
		border_type = ProvinceBorderType.Province
	
	province_borders.to[p_border_id] = to
	province_borders.from[p_border_id] = from
	province_borders.type[p_border_id] = border_type
	
	return border_type

# Get the color texture used for map modes
func get_color_texture() -> ImageTexture:
	var owner_map : Image = Image.create_empty(
		COLOR_TEXTURE_DIMENSIONS,
		COLOR_TEXTURE_DIMENSIONS,
		false,
		Image.FORMAT_RGBAF
	)
	for province_id: int in range(color_to_id_map.size()):
		var uv: Vector2i = Vector2i(province_id % COLOR_TEXTURE_DIMENSIONS, floor(float(province_id) / COLOR_TEXTURE_DIMENSIONS))
		var owner: int = provinces.owner[province_id]
		var country_color: Color
		if owner == INT_MAX:
			country_color = discard_color
		else:
			country_color = countries.color[provinces.owner[province_id]]
		owner_map.set_pixel(uv.x, uv.y, country_color)

	return ImageTexture.create_from_image(owner_map)

func get_lookup_texture() -> ImageTexture:
	return ImageTexture.create_from_image(look_up_image)

func get_lookup_image() -> Image:
	return look_up_image

func get_color_to_id_map() -> Dictionary[Color, int]:
	return color_to_id_map

func get_provinces() -> Provinces:
	return provinces

# TODO - This can be improved so the borders are rounded better.
func create_border_mesh(segments: PackedVector4Array, border_thickness: float, border_rounding: float) -> Mesh:
	var st: SurfaceTool = SurfaceTool.new()
	st.begin(Mesh.PRIMITIVE_TRIANGLES)
	
	var corner_radius: float = border_rounding

	for seg: Vector4 in segments:
		var start: Vector2 = Vector2(seg.x, seg.y)
		var end: Vector2 = Vector2(seg.z, seg.w)
		var dir: Vector2 = (end - start).normalized()
		var perp: Vector2 = Vector2(-dir.y, dir.x)

		var v0: Vector2 = start + perp * border_thickness
		var v1: Vector2 = start - perp * border_thickness
		var v2: Vector2 = end - perp * border_thickness
		var v3: Vector2 = end + perp * border_thickness

		st.add_vertex(Vector3(v0.x, v0.y, 0))
		st.add_vertex(Vector3(v1.x, v1.y, 0))
		st.add_vertex(Vector3(v2.x, v2.y, 0))

		st.add_vertex(Vector3(v2.x, v2.y, 0))
		st.add_vertex(Vector3(v3.x, v3.y, 0))
		st.add_vertex(Vector3(v0.x, v0.y, 0))

		add_rounded_border_corner(st, v0, v1, corner_radius)
		add_rounded_border_corner(st, v2, v3, corner_radius)

	return st.commit()

func add_rounded_border_corner(st: SurfaceTool, v1: Vector2, v2: Vector2, radius: float) -> void:
	var center: Vector2 = (v1 + v2) / 2
	var angle_start: float = atan2(v2.y - v1.y, v2.x - v1.x) + PI / 2
	var angle_end: float = angle_start + PI

	var segments: int = 2  # Number of segments for the arc
	for i: int in range(segments + 1):
		var angle: float = angle_start + (angle_end - angle_start) * (i / float(segments))
		var x: float = center.x + cos(angle) * radius
		var y: float = center.y + sin(angle) * radius
		st.add_vertex(Vector3(x, y, 0))

func create_map_labels(p_map: Node, p_map_width: int, p_map_height: int) -> void:
	for province_id: int in provinces.size():
		var type: ProvinceType = provinces.type[province_id]
		if type != ProvinceType.Land:
			continue # only draw labels on land provinces
	
		var label: Label3D = Label3D.new()
		var centroid: Vector2 = provinces.centroid[province_id]
		var orientation: float = provinces.orientation[province_id]
		
		label.set_position(Vector3(centroid.x-p_map_width/2.0, label_map_layer, centroid.y-p_map_height/2.0))
		label.set_rotation(Vector3(deg_to_rad(-90.0),orientation,0.0))
		label.set_scale(Vector3(100.0, 100.0, 100.0))
		
		label.set_text(tr(provinces.name[province_id])) # TODO - make spaces new lines?
		label.set_draw_flag(Label3D.FLAG_DOUBLE_SIDED, false)
		label.set_modulate(Color(0,0,0))
		label.set_outline_modulate(Color(1,1,1,0))

		p_map.call_deferred("add_child", label)
