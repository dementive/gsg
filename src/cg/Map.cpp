#include "ecs/Registry.hpp"
#include "core/math/geometry_2d.h"
#include "entt/core/type_traits.hpp"
#include "scene/3d/label_3d.h"
#include "scene/3d/mesh_instance_3d.h"
#include "scene/resources/mesh.h"
#include "scene/resources/shader.h"
#include "scene/resources/material.h"
#include "scene/resources/surface_tool.h"

#include "nodes/Map3D.hpp"
#include "core/io/config_file.h"
#include "data/Province.hpp"
#include "Map.hpp"
#include "csv.hpp"

using namespace CG;

constexpr int COLOR_TEXTURE_DIMENSIONS = 255;
constexpr float border_map_layer = 0.01;
constexpr float label_map_layer = 0.015;
const Color discard_color = Color(0,0,0);

Vector2 Map::calculate_centroid(const Polygon &p_polygon) {
	Vector2 sum = Vector2(0, 0);
	for (const Vector2 &point : p_polygon) {
		sum += point;
	}
	return sum / p_polygon.size();
}

float Map::calculate_orientation(const Polygon &p_polygon, const Vector2 &p_centroid) {
	float mu11 = 0.0;
	float mu20 = 0.0;
	float mu02 = 0.0;
	for (const Vector2 &point : p_polygon) {
		float x = point.x - p_centroid.x;
		float y = point.y - p_centroid.y;
		mu20 += x * x;
		mu02 += y * y;
		mu11 += x * y;
	}

	return 0.5 * atan2(2 * mu11, mu20 - mu02);
}

Color Map::get_random_area_color() {
	return {
		CLAMP(Math::randf(), float(0.3), float(0.7)),
		CLAMP(Math::randf(), float(0.3), float(0.7)),
		CLAMP(Math::randf(), float(0.3), float(0.7))
	};
}

Color Map::get_lookup_color(ProvinceIndex p_province_id) {
	return {
		float(int(p_province_id) % COLOR_TEXTURE_DIMENSIONS) / (COLOR_TEXTURE_DIMENSIONS - 1),
		floor(float(p_province_id) / COLOR_TEXTURE_DIMENSIONS) / (COLOR_TEXTURE_DIMENSIONS - 1),
		0.0
	};
}

ProvinceColorMap Map::load_map_config() {
	ProvinceColorMap provinces_map{};

	Ref<ConfigFile> province_config = memnew(ConfigFile());
	Ref<ConfigFile> area_config = memnew(ConfigFile());
	Ref<ConfigFile> region_config = memnew(ConfigFile());
	Ref<ConfigFile> country_config = memnew(ConfigFile());

	if (province_config->load("res://data/provinces.cfg") != OK) return provinces_map;
	if (country_config->load("res://data/countries.cfg") != OK) return provinces_map;
	if (region_config->load("res://data/regions.cfg") != OK) return provinces_map;
	if (area_config->load("res://data/areas.cfg") != OK) return provinces_map;

	List<String> province_sections;
	List<String> country_sections;
	List<String> region_sections;
	List<String> area_sections;

	province_config->get_sections(&province_sections);
	country_config->get_sections(&country_sections);
	region_config->get_sections(&region_sections);
	area_config->get_sections(&area_sections);

	Registry &registry = *Registry::self;

	for (const String &section : province_sections) {
		Color map_color = province_config->get_value(section, "color");
		map_color = Color::from_rgba8(map_color.r, map_color.g, map_color.b, map_color.a);

		const ProvinceType province_type = province_type_string_to_enum(province_config->get_value(section, "type", "land"));
		const ProvinceIndex province_id = section.to_int();

		const ProvinceEntity province_entity = registry.create_entity<ProvinceTag>(province_id);
		registry.emplace<ProvinceType>(province_entity, province_type);
		registry.emplace<Name>(province_entity, String("PROV") + uitos(province_id));
		provinces_map[map_color] = province_id;

		const Color lookup_color = get_lookup_color(province_id);
		color_to_id_map[lookup_color] = province_id;
	}

	for (const String &section : area_sections) {
		const Color color = area_config->get_value(section, "color", get_random_area_color());
		PackedInt32Array area_provinces = area_config->get_value(section, "provinces");
		const int capital = area_config->get_value(section, "capital");
		const ProvinceEntity capital_entity = registry.get_entity<ProvinceTag>(capital);

		const AreaEntity area_entity = registry.create_entity<AreaTag>(section);
		registry.emplace<Color>(area_entity, color);
		registry.emplace<Capital>(area_entity, capital_entity);
		registry.emplace<Name>(area_entity, section);
		registry.emplace<AreaProvinces>(area_entity, area_provinces);

		for (const int province_id : area_provinces) {
			const ProvinceEntity province_entity = registry.get_entity<ProvinceTag>(province_id);
			registry.emplace<AreaComponent>(province_entity, area_entity); // province stores the area it is in.
		}
	}

	// Area *area = Area::self;
	// Province *province = Province::self;
	// Country *country = Country::self;

	// province->initialize(province_sections.size());
	// country->init(country_sections.size());
	// area->init(area_sections.size());
	// //region->init(region_sections.size());


	// for (const String &section : region_sections) {
	// 	const RegionEntity region_id = section.to_int();
	// 	const String name = region_config->get_value(section, "name");
	// 	const Color color = region_config->get_value(section, "color", get_random_area_color());
	// 	const PackedInt32Array region_areas = region_config->get_value(section, "areas");
	// 	const ProvinceEntity capital = region_config->get_value(section, "capital");

	// 	//create_entity
	// 	Registry &registry = *Registry::self;
	// 	entt::entity entity = registry.create_entity<EntityTag::Region>();

	// 	registry.emplace<Color>(entity, color);

	// 	// region->set_capital(region_id, capital);
	// 	// region->set_name(region_id, name);
	// 	// region->set_color(region_id, color);
	// 	// region->set_areas(region_id, region_areas);

	// 	for (const AreaEntity area_id : region_areas) {
	// 		area->set_region(area_id, region_id);
	// 	}
	// }

	// for (const String &section : country_sections) {
	// 	const CountryEntity country_id = section.to_int();
	// 	const String name = country_config->get_value(section, "name");
	// 	const Color color = country_config->get_value(section, "color", get_random_area_color());
	// 	const PackedInt32Array country_provinces = country_config->get_value(section, "provinces");
	// 	const ProvinceEntity capital = country_config->get_value(section, "capital");

	// 	country->set_capital(country_id, capital);
	// 	country->set_name(country_id, name);
	// 	country->set_color(country_id, Color::from_rgba8(color.r, color.g, color.b, color.a));
	// 	country->set_owned_provinces(country_id, country_provinces);

	// 	for (const ProvinceEntity province_id : country_provinces) {
	// 		const CountryEntity owner = province->get_owner(province_id);
	// 		ERR_CONTINUE_MSG(owner != ENTITY_MAX, vformat("Province %s has multiple owners. Provinces can only have one owner, fix countries.cfg.", province_id));
	// 		province->set_owner(province_id, country_id);
	// 	}
	// }

	// for (const String &section : province_sections) {
	// 	Color map_color = province_config->get_value(section, "color");
	// 	map_color = Color::from_rgba8(map_color.r, map_color.g, map_color.b, map_color.a);

	// 	const ProvinceType province_type = province_type_string_to_enum(province_config->get_value(section, "type", "land"));
	// 	const ProvinceEntity province_id = section.to_int();

	// 	province->set_type(province_id, province_type);
	// 	province->set_name(province_id, String("PROV") + uitos(province_id));
	// 	provinces_map[map_color] = province_id;

	// 	const Color lookup_color = get_lookup_color(province_id);
	// 	color_to_id_map[lookup_color] = province_id;
	// }

	return provinces_map;
}

bool Map::is_lake_border(const Border &p_border) {
	// ProvinceType to_type = Province::self->get_type(p_border.first);
	// ProvinceType from_type = Province::self->get_type(p_border.second);

	// if (to_type == ProvinceType::Lake or from_type == ProvinceType::Lake)
	// 	return true;
	// else
	// 	return false;
}

Vector<Ref<ShaderMaterial>> Map::create_border_materials() {
	// Ref<Shader> border_shader = ResourceLoader::load("res://gfx/shaders/border.gdshader");
	// Vector<Ref<ShaderMaterial>> border_materials;
	// const int material_count = static_cast<int>(ProvinceBorderType::PROVINCE_BORDER_TYPE_MAX);
	// border_materials.resize(material_count);

	// #define inc_enum(i) ((decltype(i)) (static_cast<int>(i) + 1))
	// for(ProvinceBorderType i = ProvinceBorderType::Country; i < ProvinceBorderType::PROVINCE_BORDER_TYPE_MAX; i = inc_enum(i)) {
	// 	Ref<ShaderMaterial> border_material = memnew(ShaderMaterial);
	// 	border_material->set_shader(border_shader);
	// 	switch (i) {
	// 		case ProvinceBorderType::Country: {
	// 			border_material->set_shader_parameter("border_color", Color(0,0,0,1));
	// 		} break;
	// 		case ProvinceBorderType::Area: {
	// 			border_material->set_shader_parameter("border_color", Color(0.26,0.26,0.26,1));
	// 		} break;
	// 		case ProvinceBorderType::Province: {
	// 			border_material->set_shader_parameter("border_color", Color(0.31,0.31,0.31,0.9));
	// 		} break;
	// 		case ProvinceBorderType::Impassable: {
	// 			border_material->set_shader_parameter("border_color", Color(0.423,0,0,0.878));
	// 		} break;
	// 		case ProvinceBorderType::Water: {
	// 			border_material->set_shader_parameter("border_color", Color(0,0,0,1));
	// 		} break;
	// 		case ProvinceBorderType::Coastal: {
	// 			border_material->set_shader_parameter("border_color", Color(0.23,0.23,0.23,0.95));
	// 		} break;
	// 		case ProvinceBorderType::PROVINCE_BORDER_TYPE_MAX: break;
	// 	}

	// 	border_materials.push_back(border_material);
	// }

	// return border_materials;
}

void Map::fill_province_adjacency_data(ProvinceAdjacencyEntity p_adjacency_id, const Border &p_border) {
	// const ProvinceType to_type = Province::self->get_type(p_border.first);
	// const ProvinceType from_type = Province::self->get_type(p_border.second);
	// ProvinceAdjacencyType adjacency_type = ProvinceAdjacencyType::Land;

	// if (is_navigable_water_province(to_type) and is_navigable_water_province(from_type)) {
	// 	adjacency_type = ProvinceAdjacencyType::Water;
	// } else if (is_impassable_province(to_type) or is_impassable_province(from_type)) {
	// 	adjacency_type = ProvinceAdjacencyType::Impassable;
	// } else if ((to_type == ProvinceType::Ocean and from_type == ProvinceType::Land) or (to_type == ProvinceType::Land and from_type == ProvinceType::Ocean)) {
	// 	adjacency_type = ProvinceAdjacencyType::Coastal;
	// }

	// ProvinceAdjacency::self->set_from(p_adjacency_id, p_border.first);
	// ProvinceAdjacency::self->set_to(p_adjacency_id, p_border.second);
	// ProvinceAdjacency::self->set_type(p_adjacency_id, adjacency_type);
}

ProvinceBorderType Map::fill_province_border_data(ProvinceBorderEntity p_border_id, const Border &p_border) {
	// const ProvinceType to_type = Province::self->get_type(p_border.first);
	// const ProvinceType from_type = Province::self->get_type(p_border.second);

	// const int to_area = Province::self->get_area(p_border.first);
	// const int from_area = Province::self->get_area(p_border.second);
	// const int to_owner = Province::self->get_owner(p_border.first);
	// const int from_owner = Province::self->get_owner(p_border.second);
	// ProvinceBorderType border_type = ProvinceBorderType::Country;

	// if ((province_has_owner(p_border.first) and province_has_owner(p_border.second) and from_owner != to_owner))
	// 	border_type = ProvinceBorderType::Country;
	// else if (is_navigable_water_province(to_type) and is_navigable_water_province(from_type))
	// 	border_type = ProvinceBorderType::Water;
	// else if (is_impassable_province(to_type) or is_impassable_province(from_type))
	// 	border_type = ProvinceBorderType::Impassable;
	// else if ((to_type == ProvinceType::Ocean and from_type == ProvinceType::Land) or (to_type == ProvinceType::Land and from_type == ProvinceType::Ocean))
	// 	border_type = ProvinceBorderType::Coastal;
	// else if (from_area != to_area)
	// 	border_type = ProvinceBorderType::Area;
	// else if (from_area == to_area)
	// 	border_type = ProvinceBorderType::Province;

	// ProvinceBorder::self->set_from(p_border_id, p_border.second);
	// ProvinceBorder::self->set_to(p_border_id, p_border.first);
	// ProvinceBorder::self->set_type(p_border_id, border_type);

	// return border_type;
}

void Map::add_rounded_border_corners(Ref<SurfaceTool> &p_st, const Vector2 &p_v1, const Vector2 &p_v2, float p_radius) {
	Vector2 center = (p_v1 + p_v2) / 2;
	float angle_start = atan2(p_v2.y - p_v1.y, p_v2.x - p_v1.x) + (std::numbers::pi / 2);
	float angle_end = angle_start + std::numbers::pi;

	int segments  = 2;
	for (int i = 0; i < segments+1; ++i) {
		float angle = angle_start + ((angle_end - angle_start) * (i / float(segments)));
		float x = center.x + (cos(angle) * p_radius);
		float y = center.y + (sin(angle) * p_radius);
		p_st->add_vertex(Vector3(x, y, 0));
	}
}

Ref<ArrayMesh> Map::create_border_mesh(const Vector<Vector4> &p_segments, float p_border_thickness,  float p_border_rounding) {
	Ref<SurfaceTool> st = memnew(SurfaceTool);
	st->begin(Mesh::PRIMITIVE_TRIANGLES);

	for (const Vector4 &seg : p_segments) {
		Vector2 start = Vector2(seg.x, seg.y);
		Vector2 end = Vector2(seg.z, seg.w);
		Vector2 dir = (end - start).normalized();
		Vector2 perp = Vector2(-dir.y, dir.x);

		Vector2 v0 = start + perp * p_border_thickness;
		Vector2 v1 = start - perp * p_border_thickness;
		Vector2 v2 = end - perp * p_border_thickness;
		Vector2 v3 = end + perp * p_border_thickness;

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


void Map::create_map_labels(Map3D *p_map, int p_map_width, int p_map_height) {
	// Province *provinces = Province::self;
	// for (ProvinceEntity i = 0; i < Province::self->size(); ++i) {
	// 	ProvinceType type = Province::self->get_type(i);
	// 	if (type != ProvinceType::Land) continue;

	// 	Label3D *label = memnew(Label3D);

	// 	Vector2 centroid = provinces->centroid[i];
	// 	float orientation = provinces->orientation[i];
		
	// 	label->set_position(Vector3(centroid.x-(p_map_width/2.0), label_map_layer, centroid.y-(p_map_height/2.0)));
	// 	label->set_rotation(Vector3(Math::deg_to_rad(-90.0),orientation,0.0));
	// 	label->set_scale(Vector3(100.0, 100.0, 100.0));
		
	// 	label->set_text(label->tr(provinces->name[i])); // TODO - make spaces new lines?
	// 	label->set_draw_flag(Label3D::FLAG_DOUBLE_SIDED, false);
	// 	label->set_modulate(Color(0,0,0));
	// 	label->set_outline_modulate(Color(1,1,1,0));

	// 	p_map->call_deferred("add_child", label);
	// }
}

void Map::load_map(Map3D  *p_map) {
	ProvinceColorMap provinces_map = load_map_config();
	// HashMap<Border, PackedVector4Array, PairHash<ProvinceEntity, ProvinceEntity>> borders;
	// HashMap<Color, PackedVector2Array> pixel_dict;

	// Load provinces.png
	// Ref<Texture2D> province_texture = ResourceLoader::load("res://gfx/map/provinces.png");
	// Ref<Image> province_image = province_texture->get_image();
	// int province_image_width = province_image->get_width();
	// int province_image_height = province_image->get_width();

	// // Set Map3D node position, makes the world coords the same as the map coords
	// p_map->set_position(Vector3(province_image_width/2.0, 0, province_image_height/2.0));

	// // Create lookup texture
	// lookup_image = Image::create_empty(province_image_width, province_image_height, false, Image::FORMAT_RGF);

	// for (int x = 0; x < province_image_width; ++x) {
	// 	for (int y = 0; y < province_image_height; ++y) {
	// 		const Color current_color = province_image->get_pixel(x,y);

	// 		// Set lookup texture pixels
	// 		const int province_id = provinces_map[current_color];
	// 		const Color lookup_color = get_lookup_color(province_id);
	// 		lookup_image->set_pixel(x, y, lookup_color);

	// 		// Make pixel dict for polygon calculations
	// 		if (!pixel_dict.has(current_color))
	// 			pixel_dict[current_color] = PackedVector2Array();
	// 		pixel_dict[current_color].append(Vector2(x, y));

	// 		// Get border segments
	// 		if (x + 1 < province_image_width) {
	// 			const Color right_color = province_image->get_pixel(x + 1, y);
	// 			if (current_color != right_color) {
	// 				const ProvinceEntity to = provinces_map[current_color];
	// 				const ProvinceEntity from = provinces_map[right_color];
	// 				Border key;
	// 				if (to > from) // sort by largest to prevent duplicates
	// 					key = Border(to, from);
	// 				else
	// 					key = Border(from, to);

	// 				// Filter out borders and adjacencies with lakes
	// 				// movement to/from lakes is impossible and borders should never be draw on lake provinces.
	// 				if (is_lake_border(key))
	// 					continue;

	// 				if (!borders.has(key))
	// 					borders[key] = PackedVector4Array();
	// 				borders[key].append(Vector4(x + 1, y, x + 1, y + 1));
	// 			}
	// 		}

	// 		// Compare bottom neighbor if exists
	// 		if (y + 1 < province_image_height) {
	// 			const Color bottom_color = province_image->get_pixel(x, y + 1);
	// 			if (current_color != bottom_color) {
	// 				const ProvinceEntity to = provinces_map[current_color];
	// 				const ProvinceEntity from = provinces_map[bottom_color];
	// 				Border key;
	// 				if (to > from) // sort by largest to prevent duplicates
	// 					key = Border(to, from);
	// 				else
	// 					key = Border(from, to);

	// 				// Filter out borders and adjacencies with lakes
	// 				// movement to/from lakes is impossible and borders should never be draw on lake provinces.
	// 				if (is_lake_border(key))
	// 					continue;

	// 				if (!borders.has(key))
	// 					borders[key] = PackedVector4Array();
	// 				borders[key].append(Vector4(x, y + 1, x + 1, y + 1));
	// 			}
	// 		}
	// 	}	
	// }

	// // Fill in Provinces data from pixel data
	// Province *provinces = Province::self;
	// for (const KeyValue<Color, PackedVector2Array> &kv : pixel_dict) {
	// 	const ProvinceEntity province_id = provinces_map[kv.key];
	// 	const Vector2 centroid = calculate_centroid(kv.value);
	// 	provinces->set_centroid(province_id, centroid);
	// 	if (provinces->get_type(province_id) != ProvinceType::Land)
	// 		provinces->set_orientation(province_id, 0.0);
	// 	else
	// 		provinces->set_orientation(province_id, calculate_orientation(Geometry2D::convex_hull(kv.value), centroid));
	// }

	// // Setup map labels
	// create_map_labels(p_map, province_image_width, province_image_height);

	// // Parse border crossings
	// Vector<Vector<Variant>> crossings = CSV::parse_file("res://data/crossings.txt");

	// ProvinceAdjacency *province_adjacency = ProvinceAdjacency::self;
	// province_adjacency->init(borders.size() + crossings.size());
	// ProvinceBorder::self->init(borders.size());
	// ProvinceCrossing::self->crossing_locator.resize(crossings.size());

	// // Fill in crossing adjacencies
	// ProvinceAdjacencyEntity province_adjacency_id = 0;
	// for (const Vector<Variant> &crossing: crossings) {
	// 	province_adjacency->set_from(province_adjacency_id, crossing[0]);
	// 	province_adjacency->set_to(province_adjacency_id, crossing[1]);
	// 	province_adjacency->set_type(province_adjacency_id, ProvinceAdjacencyType::Crossing);

	// 	ProvinceCrossing::self->crossing_locator.push_back(Vector4(
	// 		crossing[2], crossing[3], crossing[4], crossing[5]
	// 	));

	// 	province_adjacency_id++;
	// }

	// // // Create border materials
	// Vector<Ref<ShaderMaterial>> border_materials = create_border_materials();

	// // Create border meshes
	// for (ProvinceBorderEntity province_border_id = 0; const KeyValue<Border, PackedVector4Array> &kv: borders) {
	// 	Ref<Mesh> border_mesh = create_border_mesh(kv.value, 0.75, 0.75);
	// 	//MeshInstance3D *border_mesh_instance = memnew(MeshInstance3D);

	// 	ProvinceBorderType border_type = fill_province_border_data(province_border_id, kv.key);
	// 	ProvinceBorder::self->set_rid(province_border_id, border_mesh->get_rid());
	// 	fill_province_adjacency_data(province_adjacency_id, kv.key);

	// 	province_border_id++;
	// 	province_adjacency_id++;

	// 	border_mesh_instance->set_rotation_degrees(Vector3(90, 0, 0));
	// 	border_mesh_instance->set_position(Vector3(-province_image_width / 2.0, border_map_layer, -province_image_height / 2.0));
	// 	border_mesh_instance->set_mesh(border_mesh);
	// 	border_mesh->surface_set_material(0, border_materials[static_cast<int>(border_type)]);
	// 	p_map->call_deferred("add_child", border_mesh_instance);
	// }
}

Ref<ImageTexture> Map::get_lookup_texture() {
	return ImageTexture::create_from_image(lookup_image);
}

Ref<Image> Map::get_lookup_image() {
	return lookup_image;
}

ProvinceColorMap Map::get_color_to_id_map() {
	return color_to_id_map;
}

Ref<ImageTexture> Map::get_country_map_mode() {
	// Ref<Image> country_map_map_image = Image::create_empty(COLOR_TEXTURE_DIMENSIONS, COLOR_TEXTURE_DIMENSIONS, false, Image::FORMAT_RGBAF);

	// for (ProvinceEntity i = 0; i < color_to_id_map.size(); ++i) {
	// 	Vector2i uv = Vector2i(i % COLOR_TEXTURE_DIMENSIONS, floor(float(i) / COLOR_TEXTURE_DIMENSIONS));
	// 	int owner = Province::self->get_owner(i);
	// 	Color country_color;
	// 	if (owner == INT_MAX)
	// 		country_color = discard_color;
	// 	else
	// 		country_color = Country::self->color[owner];
	// 	country_map_map_image->set_pixel(uv.x, uv.y, country_color);

	// }
	
	// return ImageTexture::create_from_image(country_map_map_image);
}
