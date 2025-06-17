#include "Map.hpp"

#include "core/crypto/hashing_context.h"
#include "core/io/config_file.h"
#include "core/io/file_access.h"
#include "core/math/geometry_2d.h"
#include "core/variant/typed_dictionary.h"

#include "scene/3d/label_3d.h"
#include "scene/3d/mesh_instance_3d.h"
#include "scene/resources/compressed_texture.h"
#include "scene/resources/material.h"
#include "scene/resources/mesh.h"
#include "scene/resources/shader.h"
#include "scene/resources/surface_tool.h"

#include "cg/csv.hpp"

#include "ecs/Provinces.hpp"
#include "ecs/Registry.hpp"

#include "templates/ConstMap.hpp"

using namespace CG;

static constexpr int COLOR_TEXTURE_DIMENSIONS = 255;
const Color discard_color = Color(0, 0, 0);

Vector2 Map::calculate_centroid(const Polygon &p_polygon) {
	Vector2 sum = Vector2(0, 0);
	for (const Vector2 &point : p_polygon)
		sum += point;
	return sum / p_polygon.size();
}

float Map::calculate_orientation(const Polygon &p_polygon, const Vector2 &p_centroid) {
	float mu11 = 0.0;
	float mu20 = 0.0;
	float mu02 = 0.0;
	for (const Vector2 &point : p_polygon) {
		const float x = point.x - p_centroid.x;
		const float y = point.y - p_centroid.y;
		mu20 += x * x;
		mu02 += y * y;
		mu11 += x * y;
	}

	return Math::rad_to_deg(0.5 * atan2(2 * mu11, mu20 - mu02));
}

Color Map::get_random_area_color() { return { CLAMP(Math::randf(), float(0.3), float(0.7)), CLAMP(Math::randf(), float(0.3), float(0.7)), CLAMP(Math::randf(), float(0.3), float(0.7)) }; }

Color Map::get_lookup_color(ProvinceIndex p_province_id) {
	return { float(int(p_province_id) % COLOR_TEXTURE_DIMENSIONS) / (COLOR_TEXTURE_DIMENSIONS - 1), floor(float(p_province_id) / COLOR_TEXTURE_DIMENSIONS) / (COLOR_TEXTURE_DIMENSIONS - 1),
		0.0 };
}

ProvinceColorMap Map::load_map_config(Registry &p_registry) {
	ProvinceColorMap provinces_map{};

	const Ref<ConfigFile> province_config = memnew(ConfigFile());
	const Ref<ConfigFile> area_config = memnew(ConfigFile());
	const Ref<ConfigFile> region_config = memnew(ConfigFile());
	const Ref<ConfigFile> country_config = memnew(ConfigFile());

	if (province_config->load("res://data/provinces.cfg") != OK)
		return provinces_map;
	if (country_config->load("res://data/countries.cfg") != OK)
		return provinces_map;
	if (region_config->load("res://data/regions.cfg") != OK)
		return provinces_map;
	if (area_config->load("res://data/areas.cfg") != OK)
		return provinces_map;

	List<String> province_sections;
	List<String> country_sections;
	List<String> region_sections;
	List<String> area_sections;

	province_config->get_sections(&province_sections);
	country_config->get_sections(&country_sections);
	region_config->get_sections(&region_sections);
	area_config->get_sections(&area_sections);

	for (const String &section : province_sections) {
		const ProvinceIndex province_id = section.to_int();
		if (province_id == 0) // Skip first ID to avoid problems with lookup texture.
			continue;

		Color map_color = province_config->get_value(section, "color");
		map_color = Color::from_rgba8(map_color.r, map_color.g, map_color.b);

		const String &province_type = province_config->get_value(section, "type", "land");

		const ProvinceEntity province_entity = p_registry.create_entity<ProvinceTag>(province_id);
		if (province_type == "land")
			p_registry.emplace<LandProvinceTag>(province_entity);
		else if (province_type == "ocean")
			p_registry.emplace<OceanProvinceTag>(province_entity);
		else if (province_type == "river")
			p_registry.emplace<RiverProvinceTag>(province_entity);
		else if (province_type == "impassable")
			p_registry.emplace<ImpassableProvinceTag>(province_entity);
		else if (province_type == "uninhabitable")
			p_registry.emplace<UninhabitableProvinceTag>(province_entity);
		p_registry.emplace<Name>(province_entity, String("PROV") + uitos(province_id));
		provinces_map[map_color] = province_id;

		const Color lookup_color = get_lookup_color(province_id);
		color_to_id_map[lookup_color] = province_id;
	}

	for (const String &section : area_sections) {
		const Color color = area_config->get_value(section, "color", get_random_area_color());
		const PackedInt32Array area_provinces_config = area_config->get_value(section, "provinces");
		const TightVec<Entity> area_provinces = p_registry.convert_packed_array<ProvinceTag, TightVec<Entity>>(area_provinces_config);

		const int capital = area_config->get_value(section, "capital");
		const ProvinceEntity capital_entity = p_registry.get_entity<ProvinceTag>(capital);

		const AreaEntity area_entity = p_registry.create_entity<AreaTag>(section);
		p_registry.emplace<Color>(area_entity, color);
		p_registry.emplace<Capital>(area_entity, capital_entity);
		p_registry.emplace<Name>(area_entity, section);
		p_registry.emplace<AreaProvinces>(area_entity, area_provinces);

		for (const ProvinceEntity province_entity : area_provinces)
			p_registry.emplace<AreaComponent>(province_entity, area_entity); // province stores the area it is in.
	}

	for (const String &section : region_sections) {
		const Color color = region_config->get_value(section, "color", get_random_area_color());
		const PackedStringArray region_areas_config = region_config->get_value(section, "areas");
		const TightVec<Entity> region_areas = p_registry.convert_packed_array<AreaTag, TightVec<Entity>>(region_areas_config);

		const int capital = region_config->get_value(section, "capital");
		const ProvinceEntity capital_entity = p_registry.get_entity<ProvinceTag>(capital);

		const RegionEntity region_entity = p_registry.create_entity<RegionTag>(section);
		p_registry.emplace<Color>(region_entity, color);
		p_registry.emplace<Name>(region_entity, section);
		p_registry.emplace<Capital>(region_entity, capital_entity);
		p_registry.emplace<RegionAreas>(region_entity, region_areas);

		for (const AreaEntity &area_entity : region_areas)
			p_registry.emplace<RegionComponent>(area_entity, region_entity);
	}

	for (const String &section : country_sections) {
		const Color color = country_config->get_value(section, "color", get_random_area_color());
		const PackedInt32Array owned_provinces_config = country_config->get_value(section, "provinces");
		const Vec<Entity> owned_provinces = p_registry.convert_packed_array<ProvinceTag>(owned_provinces_config);

		const int capital = country_config->get_value(section, "capital");
		const ProvinceEntity capital_entity = p_registry.get_entity<ProvinceTag>(capital);

		const CountryEntity country_entity = p_registry.create_entity<CountryTag>(section);
		p_registry.emplace<Color>(country_entity, Color::from_rgba8(color.r, color.g, color.b, color.a));
		p_registry.emplace<Name>(country_entity, section);
		p_registry.emplace<Capital>(country_entity, capital_entity);
		p_registry.emplace<OwnedProvinces>(country_entity, owned_provinces);

		for (const ProvinceEntity province_entity : owned_provinces) {
			ERR_CONTINUE_MSG(p_registry.all_of<Owner>(province_entity),
					vformat("Province %d assigned multiple owners. Provinces can only have one owner, fix countries.cfg.", static_cast<int>(province_entity)));
			p_registry.emplace<Owner>(province_entity, country_entity);
		}
	}

	return provinces_map;
}

bool Map::is_lake_border(const Registry &p_registry, const Border &p_border) {
	if (p_registry.all_of<LakeProvinceTag>(p_border.first) or p_registry.all_of<LakeProvinceTag>(p_border.second))
		return true;
	else
		return false;
}

#define inc_enum(i) ((decltype(i))(static_cast<int>(i) + 1))

void Map::create_border_materials() {
	const int material_count = static_cast<int>(ProvinceBorderType::PROVINCE_BORDER_TYPE_MAX);
	border_materials.reserve(material_count);

	RenderingServer &rs = *RS::get_singleton();
	const Ref<Shader> border_shader = ResourceLoader::load("res://gfx/shaders/border.gdshader", "Shader", ResourceFormatLoader::CACHE_MODE_IGNORE_DEEP);

	for (ProvinceBorderType i = ProvinceBorderType::Country; i < ProvinceBorderType::PROVINCE_BORDER_TYPE_MAX; i = inc_enum(i)) {
		const RID material_rid = rs.material_create();
		rs.material_set_shader(material_rid, border_shader->get_rid());
		switch (i) {
			case ProvinceBorderType::Country: {
				rs.material_set_param(material_rid, "border_color", Color(0, 0, 0, 1));
			} break;
			case ProvinceBorderType::Area: {
				rs.material_set_param(material_rid, "border_color", Color(0.26, 0.26, 0.26, 1));
			} break;
			case ProvinceBorderType::Province: {
				rs.material_set_param(material_rid, "border_color", Color(0.31, 0.31, 0.31, 0.9));
			} break;
			case ProvinceBorderType::Impassable: {
				rs.material_set_param(material_rid, "border_color", Color(0.423, 0, 0, 0.878));
			} break;
			case ProvinceBorderType::Water: {
				rs.material_set_param(material_rid, "border_color", Color(0, 0, 0, 1));
			} break;
			case ProvinceBorderType::Coastal: {
				rs.material_set_param(material_rid, "border_color", Color(0.23, 0.23, 0.23, 0.95));
			} break;
			case ProvinceBorderType::PROVINCE_BORDER_TYPE_MAX: break;
		}

		border_materials.push_back(material_rid);
	}
}

void Map::fill_province_adjacency_data(Registry &p_registry, const Border &p_border) {
	ProvinceAdjacencyType adjacency_type = ProvinceAdjacencyType::Land;

	if (is_navigable_water_province(p_registry, p_border.first) and is_navigable_water_province(p_registry, p_border.second))
		adjacency_type = ProvinceAdjacencyType::Water;
	else if (is_impassable_province(p_registry, p_border.first) or is_impassable_province(p_registry, p_border.second))
		adjacency_type = ProvinceAdjacencyType::Impassable;
	else if ((p_registry.all_of<OceanProvinceTag>(p_border.first) and p_registry.all_of<LandProvinceTag>(p_border.second)) or
			(p_registry.all_of<LandProvinceTag>(p_border.first) and p_registry.all_of<OceanProvinceTag>(p_border.second)))
		adjacency_type = ProvinceAdjacencyType::Coastal;

	const ProvinceAdjacencyEntity adjacency_entity = p_registry.create();
	p_registry.emplace<AdjacencyTo>(adjacency_entity, p_border.first);
	p_registry.emplace<AdjacencyFrom>(adjacency_entity, p_border.second);
	p_registry.emplace<ProvinceAdjacencyType>(adjacency_entity, adjacency_type);

	ProvinceAdjacencies &to_province_adjacencies = p_registry.get_or_emplace<ProvinceAdjacencies>(p_border.first, ProvinceAdjacencies());
	to_province_adjacencies.push_back(adjacency_entity);

	ProvinceAdjacencies &from_province_adjacencies = p_registry.get_or_emplace<ProvinceAdjacencies>(p_border.second, ProvinceAdjacencies());
	from_province_adjacencies.push_back(adjacency_entity);
}

ProvinceBorderType Map::fill_province_border_data(Registry &p_registry, const Border &p_border, const RID &p_rid) {
	ProvinceBorderType border_type = ProvinceBorderType::Country;

	if ((p_registry.all_of<Owner>(p_border.first) and p_registry.all_of<Owner>(p_border.second) and p_registry.get<Owner>(p_border.first) != p_registry.get<Owner>(p_border.second))) {
		border_type = ProvinceBorderType::Country;
	} else if (is_navigable_water_province(p_registry, p_border.first) and is_navigable_water_province(p_registry, p_border.second)) {
		border_type = ProvinceBorderType::Water;
	} else if (is_impassable_province(p_registry, p_border.first) or is_impassable_province(p_registry, p_border.second)) {
		border_type = ProvinceBorderType::Impassable;
	} else if ((p_registry.all_of<OceanProvinceTag>(p_border.first) and p_registry.all_of<LandProvinceTag>(p_border.second)) or
			(p_registry.all_of<LandProvinceTag>(p_border.first) and p_registry.all_of<OceanProvinceTag>(p_border.second))) {
		border_type = ProvinceBorderType::Coastal;
	}

	if (p_registry.all_of<AreaComponent>(p_border.first) and p_registry.all_of<AreaComponent>(p_border.second)) {
		const AreaEntity to_area = p_registry.get<AreaComponent>(p_border.first);
		const AreaEntity from_area = p_registry.get<AreaComponent>(p_border.second);
		if (from_area != to_area)
			border_type = ProvinceBorderType::Area;
		else if (from_area == to_area)
			border_type = ProvinceBorderType::Province;
	}

	const ProvinceBorderEntity border_entity = p_registry.create();
	p_registry.emplace<AdjacencyTo>(border_entity, p_border.first);
	p_registry.emplace<AdjacencyFrom>(border_entity, p_border.second);
	p_registry.emplace<ProvinceBorderType>(border_entity, border_type);
	p_registry.emplace<ProvinceBorderMeshRID>(border_entity, p_rid);

	ProvinceBorders &to_province_borders = p_registry.get_or_emplace<ProvinceBorders>(p_border.first, ProvinceBorders());
	to_province_borders.push_back(border_entity);

	ProvinceBorders &from_province_borders = p_registry.get_or_emplace<ProvinceBorders>(p_border.second, ProvinceBorders());
	from_province_borders.push_back(border_entity);

	return border_type;
}

void Map::add_rounded_border_corners(Ref<SurfaceTool> &p_st, const Vector2 &p_v1, const Vector2 &p_v2, float p_radius) {
	const Vector2 center = (p_v1 + p_v2) / 2;
	const float angle_start = atan2(p_v2.y - p_v1.y, p_v2.x - p_v1.x) + (std::numbers::pi / 2);
	const float angle_end = angle_start + std::numbers::pi;

	const int segments = 2;
	for (int i = 0; i < segments + 1; ++i) {
		const float angle = angle_start + ((angle_end - angle_start) * (i / float(segments)));
		const float x = center.x + (cos(angle) * p_radius);
		const float y = center.y + (sin(angle) * p_radius);
		p_st->add_vertex(Vector3(x, y, 0));
	}
}

Ref<ArrayMesh> Map::create_border_mesh(const Vec<Vector4> &p_segments, float p_border_thickness, float p_border_rounding) {
	Ref<SurfaceTool> st = memnew(SurfaceTool);
	st->begin(Mesh::PRIMITIVE_TRIANGLES);

	for (const Vector4 &seg : p_segments) {
		const Vector2 start = Vector2(seg.x, seg.y);
		const Vector2 end = Vector2(seg.z, seg.w);
		const Vector2 dir = (end - start).normalized();
		const Vector2 perp = Vector2(-dir.y, dir.x);

		const Vector2 v0 = start + perp * p_border_thickness;
		const Vector2 v1 = start - perp * p_border_thickness;
		const Vector2 v2 = end - perp * p_border_thickness;
		const Vector2 v3 = end + perp * p_border_thickness;

		st->add_vertex(Vector3(v0.x, v0.y, 0));
		st->add_vertex(Vector3(v1.x, v1.y, 0));
		st->add_vertex(Vector3(v2.x, v2.y, 0));

		st->add_vertex(Vector3(v2.x, v2.y, 0));
		st->add_vertex(Vector3(v3.x, v3.y, 0));
		st->add_vertex(Vector3(v0.x, v0.y, 0));

		add_rounded_border_corners(st, v0, v1, p_border_rounding);
		add_rounded_border_corners(st, v2, v3, p_border_rounding);
	}

	return st->commit();
}

void Map::create_map_labels(const Registry &p_registry, Node3D *p_map, int p_map_width, int p_map_height) {
	const auto province_view = p_registry.view<LandProvinceTag, TextLocator, Name>();

	for (auto [entity, locator, name] : province_view.each()) {
		Label3D *label = memnew(Label3D);
		label->set_position(Vector3(locator.position.x - (p_map_width / 2.0), label_map_layer, locator.position.y - (p_map_height / 2.0)));
		label->set_rotation(Vector3(-1.570796, locator.orientation, 0.0));
		label->set_scale(Vector3(locator.scale, locator.scale, locator.scale));

		label->set_text(label->tr(name)); // TODO - make spaces new lines?
		label->set_draw_flag(Label3D::FLAG_DOUBLE_SIDED, false);
		label->set_modulate(Color(0, 0, 0));
		label->set_outline_modulate(Color(1, 1, 1, 0));

		p_map->call_deferred("add_child", label);
	}
}

void Map::create_border_meshes(Registry &p_registry, Node3D *p_map, Dictionary p_border_dict, bool is_map_editor) {
	// Create border materials
	create_border_materials();
	RenderingServer &rs = *RS::get_singleton();

	// Create border meshes
	const Array border_keys = p_border_dict.keys();
	border_meshes.reserve(border_keys.size());

	for (const Variant &border_key : border_keys) {
		const PackedInt32Array key = border_key;
		const PackedVector4Array value = p_border_dict[key];
		const Border border = Border(p_registry.get_entity<ProvinceTag>(key[0]), p_registry.get_entity<ProvinceTag>(key[1]));

		const Ref<ArrayMesh> border_mesh_resource = create_border_mesh(value, 0.75, 0.75);
		const RID border_mesh = border_mesh_resource->get_rid();
		const RID mesh_instance = rs.instance_create2(border_mesh, p_map->get_world_3d()->get_scenario());

		const BorderMesh border_mesh_storage{ .mesh = border_mesh_resource, .instance = mesh_instance };
		border_meshes.push_back(border_mesh_storage);

		const ProvinceBorderType border_type = fill_province_border_data(p_registry, border, border_mesh);
		if (!is_map_editor)
			fill_province_adjacency_data(p_registry, border);

		const Transform3D mesh_transform = Transform3D(Basis().rotated(Vector3(1, 0, 0), 1.570796), Vector3(0, border_map_layer, 0));
		rs.instance_set_transform(mesh_instance, mesh_transform);
		rs.mesh_surface_set_material(border_mesh, 0, border_materials[static_cast<int>(border_type)]);
	}
}

template void Map::load_map<false>(Node3D *p_map);
template void Map::load_map<true>(Node3D *p_map);

#ifdef TOOLS_ENABLED

void Map::load_map_editor(Node3D *p_map) {
	const Ref<Texture2D> province_texture = ResourceLoader::load("res://gfx/map/provinces.png", "Texture2D", ResourceFormatLoader::CACHE_MODE_IGNORE_DEEP);
	String province_data_file = FileAccess::open("res://data/provinces.cfg", FileAccess::READ)->get_as_text();

	// Generating the lookup image and map data can be very slow and 90% of the time the map editor is loaded the province config and png haven't actually been changed.
	// To make it faster to open in the common case of no data changing, hash provinces.png and provinces.cfg together and compare them to a cached hash to check if the map data needs to be
	// regenerated. If the hashes are the same, nothing has changed and can just load the saved data. If the hashes are different then have to regenerate all the map data.
	Ref<HashingContext> hash_context = memnew(HashingContext());
	hash_context->start(HashingContext::HashType::HASH_MD5);
	hash_context->update(province_texture->get_image()->get_data());
	hash_context->update(province_data_file.md5_buffer());
	const PackedByteArray hash_bytes = hash_context->finish();
	const String hash_string = String::hex_encode_buffer(hash_bytes.ptr(), hash_bytes.size());

	if (!FileAccess::exists("user://map_data_hash.cache")) [[unlikely]]
		FileAccess::open("user://map_data_hash.cache", FileAccess::WRITE);

	Ref<FileAccess> map_data_cache = FileAccess::open("user://map_data_hash.cache", FileAccess::READ);
	const String cached_hash = map_data_cache->get_line();

	if (hash_string == cached_hash) {
		load_map<true>(p_map);
		return;
	}

	map_data_cache->close();
	map_data_cache = FileAccess::open("user://map_data_hash.cache", FileAccess::WRITE);
	map_data_cache->store_string(hash_string);
	print_line("Map data has changed, regenerating data");

	Registry &registry = *Registry::self;
	ProvinceColorMap provinces_map = load_map_config(registry);

	TypedDictionary<PackedInt32Array, PackedVector4Array> borders_dict;
	AHashMap<ProvinceEntity, Vec<Vector2>, EntityHasher> pixel_dict;

	const Ref<Image> province_image = province_texture->get_image();
	const int province_image_width = province_image->get_width();
	const int province_image_height = province_image->get_width();

	Ref<ConfigFile> map_data_config = memnew(ConfigFile());
	map_data_config->set_value("map_data", "width", province_image_width);
	map_data_config->set_value("map_data", "height", province_image_height);

	// Set Map node position, makes the world coords the same as the map coords
	p_map->set_position(Vector3(province_image_width / 2.0, 0, province_image_height / 2.0));

	// Create lookup texture as RGF Image
	float *lookup_write_ptr{};
	Vector<uint8_t> lookup_image_data;
	lookup_image_data.resize(static_cast<size_t>(province_image_width) * province_image_height * 2 * sizeof(float));
	uint8_t *lookup_ptr = lookup_image_data.ptrw();
	lookup_write_ptr = reinterpret_cast<float *>(lookup_ptr);

	for (int x = 0; x < province_image_width; ++x) {
		for (int y = 0; y < province_image_height; ++y) {
			const Color current_color = province_image->get_pixel(x, y);

			// Set lookup texture pixels
			const ProvinceIndex province_id = provinces_map[current_color];
			const Color lookup_color = get_lookup_color(province_id);
			const size_t lookup_index = (static_cast<size_t>(y) * province_image_width + x) * 2;
			lookup_write_ptr[lookup_index + 0] = lookup_color.r;
			lookup_write_ptr[lookup_index + 1] = lookup_color.g;

			// Make pixel dict for polygon calculations
			const ProvinceEntity province_entity = registry.get_entity<ProvinceTag>(province_id);
			pixel_dict[province_entity].push_back(Vector2(x, y));

			// Get border segments
			if (x + 1 < province_image_width) {
				const Color right_color = province_image->get_pixel(x + 1, y);
				if (current_color != right_color) {
					const int from = provinces_map[right_color];
					PackedInt32Array arr;
					if (province_id > from) { // Sort to prevent duplicates
						arr.push_back(province_id);
						arr.push_back(from);
					} else {
						arr.push_back(from);
						arr.push_back(province_id);
					}

					// Filter out borders and adjacencies with lakes
					// movement to/from lakes is impossible and borders should never be draw on lake provinces.
					if (is_lake_border(registry, Border(province_entity, registry.get_entity<ProvinceTag>(from))))
						continue;

					PackedVector4Array borders_arr = borders_dict[arr];
					borders_arr.push_back(Vector4(x + 1, y, x + 1, y + 1));
					borders_dict[arr] = borders_arr;
				}
			}

			// Compare bottom neighbor if exists
			if (y + 1 < province_image_height) {
				const Color bottom_color = province_image->get_pixel(x, y + 1);
				if (current_color != bottom_color) {
					const int from = provinces_map[bottom_color];
					PackedInt32Array arr;
					if (province_id > from) {
						arr.push_back(province_id);
						arr.push_back(from);
					} else {
						arr.push_back(from);
						arr.push_back(province_id);
					}

					if (is_lake_border(registry, Border(province_entity, registry.get_entity<ProvinceTag>(from))))
						continue;

					PackedVector4Array borders_arr = borders_dict[arr];
					borders_arr.push_back(Vector4(x, y + 1, x + 1, y + 1));
					borders_dict[arr] = borders_arr;
				}
			}
		}
	}

	// Create lookup image from bytes
	lookup_image = Image::create_from_data(province_image_width, province_image_height, false, Image::FORMAT_RGF, lookup_image_data);
	lookup_image->save_exr("res://gfx/gen/province_lookup.exr");

	// Fill in Provinces data from pixel data
	Ref<ConfigFile> province_data_config = memnew(ConfigFile());
	for (const KeyValue<ProvinceEntity, Vec<Vector2>> &kv : pixel_dict) {
		const Vector2 centroid = calculate_centroid(kv.value);
		float orientation = 0.0;
		int province_id = 0;

		// TODO - use find_if
		for (const KeyValue<int, Entity> &provinces_kv : registry.provinces)
			if (provinces_kv.value == kv.key)
				province_id = provinces_kv.key;

		if (province_id == 0)
			return;

		if (!registry.all_of<LandProvinceTag>(kv.key)) {
			province_data_config->set_value(itos(province_id), "orientation", orientation);
		} else {
			orientation = calculate_orientation(Geometry2D::convex_hull(kv.value), centroid);
			province_data_config->set_value(itos(province_id), "orientation", orientation);
		}

		province_data_config->set_value(itos(province_id), "centroid", centroid);
	}

	province_data_config->save("res://data/gen/province_data.cfg");

	map_data_config->set_value("map_data", "borders", borders_dict);
	map_data_config->save("res://data/gen/map_data.cfg");

	create_border_meshes(registry, p_map, borders_dict, true);
}

#endif

void Map::load_locators(Registry &p_registry) {
	Ref<ConfigFile> text_config = memnew(ConfigFile());
	Ref<ConfigFile> unit_config = memnew(ConfigFile());

	text_config->load("res://data/locators/text.cfg");
	unit_config->load("res://data/locators/unit.cfg");

	List<String> text_sections;
	text_config->get_sections(&text_sections);
	List<String> unit_sections;
	unit_config->get_sections(&unit_sections);

	for (const String &section : text_sections) {
		const int province_id = section.to_int();
		const Entity entity = p_registry.get_entity<ProvinceTag>(province_id);

		TextLocator locator;
		locator.position = text_config->get_value(section, "position");
		locator.orientation = text_config->get_value(section, "orientation");
		locator.scale = text_config->get_value(section, "scale");

		p_registry.emplace<TextLocator>(entity, locator);
	}

	for (const String &section : unit_sections) {
		const int province_id = section.to_int();
		const Entity entity = p_registry.get_entity<ProvinceTag>(province_id);

		UnitLocator locator;
		locator.position = unit_config->get_value(section, "position");
		locator.orientation = unit_config->get_value(section, "orientation");
		locator.scale = unit_config->get_value(section, "scale");

		p_registry.emplace<UnitLocator>(entity, locator);
	}
}

template <bool is_map_editor> void Map::load_map(Node3D *p_map) {
	Registry &registry = *Registry::self;
	ProvinceColorMap provinces_map = load_map_config(registry);

	Ref<ConfigFile> map_data_config = memnew(ConfigFile());
	map_data_config->load("res://data/gen/map_data.cfg");

	const int province_image_width = map_data_config->get_value("map_data", "width");
	const int province_image_height = map_data_config->get_value("map_data", "height");

	// Set Map node position, makes the world coords the same as the map coords
	p_map->set_position(Vector3(province_image_width / 2.0, 0, province_image_height / 2.0));

	// Load lookup image
	Ref<CompressedTexture2D> compressed_lookup_texture = ResourceLoader::load("res://gfx/gen/province_lookup.exr");
	lookup_image = compressed_lookup_texture->get_image();

	if constexpr (!is_map_editor) {
		load_locators(registry);

		// Setup map labels
		create_map_labels(registry, p_map, province_image_width, province_image_height);

		// Parse border crossings
		const Vector<Vector<Variant>> crossings = CSV::parse_file("res://data/crossings.txt");
		static constexpr ConstMap<std::string_view, int, 6> adjacency_config{ { "to", 0 }, { "from", 1 }, { "startx", 2 }, { "starty", 3 }, { "endx", 4 }, { "endy", 5 } };

		// Fill in crossing adjacencies
		for (const Vector<Variant> &crossing : crossings) {
			const ProvinceAdjacencyEntity adjacency_entity = registry.create();
			const ProvinceEntity to_entity = registry.get_entity<ProvinceTag>(crossing[adjacency_config["to"]]);
			const ProvinceEntity from_entity = registry.get_entity<ProvinceTag>(crossing[adjacency_config["from"]]);

			registry.emplace<AdjacencyTo>(adjacency_entity, to_entity);
			registry.emplace<AdjacencyFrom>(adjacency_entity, from_entity);
			registry.emplace<ProvinceAdjacencyType>(adjacency_entity, ProvinceAdjacencyType::Crossing);
			// clang-format off
			registry.emplace<CrossingLocator>(
				adjacency_entity,
				Vector4(
					crossing[adjacency_config["startx"]], crossing[adjacency_config["starty"]], crossing[adjacency_config["endx"]], crossing[adjacency_config["endy"]]
				)
			);
			// clang-format on
		}

		create_border_meshes(registry, p_map, map_data_config->get_value("map_data", "borders"), false);
	}

	if constexpr (is_map_editor)
		create_border_meshes(registry, p_map, map_data_config->get_value("map_data", "borders"), true);
}

Ref<ImageTexture> Map::get_lookup_texture() { return ImageTexture::create_from_image(lookup_image); }

Ref<Image> Map::get_lookup_image() { return lookup_image; }

ProvinceColorMap Map::get_color_to_id_map() { return color_to_id_map; }

Ref<ImageTexture> Map::get_country_map_mode() {
	const Registry &registry = *Registry::self;
	const Ref<Image> country_map_mode_image = Image::create_empty(COLOR_TEXTURE_DIMENSIONS, COLOR_TEXTURE_DIMENSIONS, false, Image::FORMAT_RGBAF);

	// Iteration starts at 1 because province ID 0 does not exist.
	for (uint32_t i = 1; i < color_to_id_map.size() + 1; ++i) {
		const Vector2i uv = Vector2i(i % COLOR_TEXTURE_DIMENSIONS, floor(float(i) / COLOR_TEXTURE_DIMENSIONS));
		const ProvinceEntity province_entity = registry.get_entity<ProvinceTag>(i);

		Color country_color = discard_color;
		if (registry.all_of<Owner>(province_entity)) {
			const CountryEntity owner = registry.get<Owner>(province_entity);
			country_color = registry.get<Color>(owner);
		}

		country_map_mode_image->set_pixel(uv.x, uv.y, country_color);
	}

	return ImageTexture::create_from_image(country_map_mode_image);
}

Map::~Map() {
	for (const BorderMesh &border_mesh : border_meshes)
		RS::get_singleton()->free(border_mesh.instance);

	for (const RID &material_rid : border_materials)
		RS::get_singleton()->free(material_rid);
}
