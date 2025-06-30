#include "Map.hpp"

#include "core/crypto/hashing_context.h"
#include "core/io/config_file.h"
#include "core/io/file_access.h"
#include "core/math/geometry_2d.h"
#include "core/os/memory.h"
#include "core/variant/typed_dictionary.h"

#include "scene/3d/node_3d.h"
#include "scene/resources/compressed_texture.h"
#include "scene/resources/mesh.h"
#include "scene/resources/shader.h"
#include "scene/resources/surface_tool.h"

#include "cg/csv.hpp"
#include "cg/MapMode.hpp"

#include "ecs/ecs.hpp"
#include "ecs/Provinces.hpp"

#include "ecs_components.hpp"
#include "ecs_entity.hpp"
#include "ecs_tags.hpp"
#include "MapLabel.hpp"

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

Color Map::get_random_area_color() { return { CLAMP(Math::randf(), float(76), float(178)), CLAMP(Math::randf(), float(76), float(178)), CLAMP(Math::randf(), float(76), float(178)) }; }

Color Map::get_lookup_color(ProvinceIndex p_province_id) {
	return { float(int(p_province_id) % COLOR_TEXTURE_DIMENSIONS) / (COLOR_TEXTURE_DIMENSIONS - 1),
		static_cast<float>(std::floor(float(p_province_id) / COLOR_TEXTURE_DIMENSIONS) / (COLOR_TEXTURE_DIMENSIONS - 1)), 0.0 };
}

ProvinceColorMap Map::load_map_config() {
	ECS &ecs = *ECS::self;

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

	Vector<String> province_sections = province_config->get_sections();
	Vector<String> country_sections = country_config->get_sections();
	Vector<String> region_sections = region_config->get_sections();
	Vector<String> area_sections = area_config->get_sections();

	// Register variant components
	ecs.component<String>();
	ecs.component<Color>();

	// Register components
	ecs.component<CrossingLocator>();
	ecs.component<ProvinceBorderMeshRID>();
	ecs.component<UnitLocator>();
	ecs.component<TextLocator>();

	// Register tag components
	ecs.component<AreaTag>();
	ecs.component<CountryTag>();
	ecs.component<RegionTag>();
	ecs.component<ProvinceTag>();

	ecs.component<LandProvinceTag>();
	ecs.component<OceanProvinceTag>();
	ecs.component<RiverProvinceTag>();
	ecs.component<ImpassableProvinceTag>();
	ecs.component<UninhabitableProvinceTag>();

	// Top level scope entities. Adding each entity types to these as children allows using ecs.lookup("p::1") syntax to lookup entities.
	// These entities hold no data they just act as namespaces inside of flecs.
	ecs.register_scopes();

	// Create all relationship entities
	ecs.register_relations();

	for (const String &section : province_sections) {
		const ProvinceIndex province_id = section.to_int();
		if (province_id == 0) // Skip first ID to avoid problems with lookup texture.
			continue;

		Color map_color = province_config->get_value(section, "color");
		map_color = Color::from_rgba8(map_color.r, map_color.g, map_color.b);

		const String province_type = province_config->get_value(section, "type", "land");

		const String province_name = uitos(province_id);
		ProvinceEntity province_entity = ecs.entity(province_name.utf8().ptr());
		province_entity.child_of(ecs.get_scope(Scope::Province));

		province_entity.set<String>(String("PROV") + uitos(province_id));
		province_entity.add<ProvinceTag>();

		if (province_type == "land")
			province_entity.add<LandProvinceTag>();
		else if (province_type == "ocean")
			province_entity.add<OceanProvinceTag>();
		else if (province_type == "river")
			province_entity.add<RiverProvinceTag>();
		else if (province_type == "impassable")
			province_entity.add<ImpassableProvinceTag>();
		else if (province_type == "uninhabitable")
			province_entity.add<UninhabitableProvinceTag>();

		provinces_map[map_color] = province_id;

		const Color lookup_color = get_lookup_color(province_id);
		color_to_id_map[lookup_color] = province_id;
	}

	for (const String &section : area_sections) {
		Color color = area_config->get_value(section, "color", get_random_area_color());
		color = Color::from_rgba8(color.r, color.g, color.b);

		const String capital = area_config->get_value(section, "capital");

		const ProvinceEntity capital_entity = ecs.scope_lookup(Scope::Province, capital);
		const AreaEntity area_entity = ecs.entity(section.utf8().ptr());
		area_entity.child_of(ecs.get_scope(Scope::Area));

		const PackedInt32Array area_provinces_config = area_config->get_value(section, "provinces");
		for (const int province : area_provinces_config) {
			const ProvinceEntity province_entity = ecs.scope_lookup(Scope::Province, uitos(province));
			province_entity.add(Relationship(InArea), area_entity);
			area_entity.add(Relationship(ProvinceIn), province_entity);
		}

		area_entity.add<AreaTag>();
		area_entity.add(Relationship(Capital), capital_entity);

		area_entity.set<Color>(color);
		area_entity.set<String>(section);
	}

	for (const String &section : region_sections) {
		Color color = region_config->get_value(section, "color", get_random_area_color());
		color = Color::from_rgba8(color.r, color.g, color.b);

		const String capital = region_config->get_value(section, "capital");
		const ProvinceEntity capital_entity = ecs.scope_lookup(Scope::Province, capital);

		const RegionEntity region_entity = ecs.entity(section.utf8().ptr());
		region_entity.child_of(ecs.get_scope(Scope::Region));

		const PackedStringArray region_areas_config = region_config->get_value(section, "areas");
		for (const String &area : region_areas_config) {
			const AreaEntity area_entity = ecs.scope_lookup(Scope::Area, area);
			area_entity.add(Relationship(InRegion), region_entity);

			int idx = 0;
			Entity province_entity;
			while ((province_entity = area_entity.target(Relationship(ProvinceIn), idx++)))
				province_entity.add(Relationship(InRegion), region_entity);
		}

		region_entity.add<AreaTag>();
		region_entity.add(Relationship(Capital), capital_entity);

		region_entity.set<Color>(color);
		region_entity.set<String>(section);
	}

	for (const String &section : country_sections) {
		Color color = country_config->get_value(section, "color", get_random_area_color());
		color = Color::from_rgba8(color.r, color.g, color.b);

		const PackedInt32Array owned_provinces_config = country_config->get_value(section, "provinces");
		const String capital = country_config->get_value(section, "capital");
		const ProvinceEntity capital_entity = ecs.scope_lookup(Scope::Province, capital);

		const CountryEntity country_entity = ecs.entity(section.utf8().ptr());
		country_entity.child_of(ecs.get_scope(Scope::Country));

		for (const int &province : owned_provinces_config) {
			const ProvinceEntity province_entity = ecs.scope_lookup(Scope::Province, uitos(province));
			province_entity.add(Relationship(Owner), country_entity);
			country_entity.add(Relationship(Owns), province_entity);
		}

		country_entity.add<CountryTag>();
		country_entity.add(Relationship(Capital), capital_entity);

		country_entity.set<Color>(color);
		country_entity.set<String>(section);
	}

	return provinces_map;
}

bool Map::is_lake_border(const Border &p_border) {
	if (p_border.first.has<LakeProvinceTag>() or p_border.second.has<LakeProvinceTag>())
		return true;
	else
		return false;
}

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

void Map::fill_province_adjacency_data(const Border &p_border) {
	// ProvinceAdjacencyType adjacency_type = ProvinceAdjacencyType::Land;

	// if (is_navigable_water_province(p_border.first) and is_navigable_water_province(p_border.second))
	// 	adjacency_type = ProvinceAdjacencyType::Water;
	// else if (is_impassable_province(p_border.first) or is_impassable_province(p_border.second))
	// 	adjacency_type = ProvinceAdjacencyType::Impassable;
	// else if ((p_border.first.has<OceanProvinceTag>() and p_border.second.has<LandProvinceTag>()) or (p_border.first.has<LandProvinceTag>() and p_border.second.has<OceanProvinceTag>()))
	// 	adjacency_type = ProvinceAdjacencyType::Coastal;

	// TODO

	// const ProvinceAdjacencyEntity adjacency_entity = p_registry.create();
	// p_registry.emplace<AdjacencyTo>(adjacency_entity, p_border.first);
	// p_registry.emplace<AdjacencyFrom>(adjacency_entity, p_border.second);
	// p_registry.emplace<ProvinceAdjacencyType>(adjacency_entity, adjacency_type);

	// ProvinceAdjacencies &to_province_adjacencies = p_registry.get_or_emplace<ProvinceAdjacencies>(p_border.first, ProvinceAdjacencies());
	// to_province_adjacencies.push_back(adjacency_entity);

	// ProvinceAdjacencies &from_province_adjacencies = p_registry.get_or_emplace<ProvinceAdjacencies>(p_border.second, ProvinceAdjacencies());
	// from_province_adjacencies.push_back(adjacency_entity);
}

ProvinceBorderType Map::fill_province_border_data(const Border &p_border, const RID &p_rid) {
	ProvinceBorderType border_type = ProvinceBorderType::Country;
	ECS &ecs = *ECS::self;

	if ((ecs.has_relation(p_border.first, Relation::Owner) and ecs.has_relation(p_border.second, Relation::Owner) and
				ecs.get_target(p_border.first, Relation::Owner) != ecs.get_target(p_border.second, Relation::Owner)))
		border_type = ProvinceBorderType::Country;
	else if (is_navigable_water_province(p_border.first) and is_navigable_water_province(p_border.second))
		border_type = ProvinceBorderType::Water;
	else if (is_impassable_province(p_border.first) or is_impassable_province(p_border.second))
		border_type = ProvinceBorderType::Impassable;
	else if ((p_border.first.has<OceanProvinceTag>() and p_border.second.has<LandProvinceTag>()) or (p_border.first.has<LandProvinceTag>() and p_border.second.has<OceanProvinceTag>()))
		border_type = ProvinceBorderType::Coastal;

	AreaEntity to_area = ecs.get_target(p_border.first, Relation::InArea);
	AreaEntity from_area = ecs.get_target(p_border.second, Relation::InArea);
	if (to_area.is_valid() and from_area.is_valid()) {
		if (from_area != to_area)
			border_type = ProvinceBorderType::Area;
		else
			border_type = ProvinceBorderType::Province;
	}

	// TODO

	// const Entity border_entity = ECS::self->entity();
	// p_registry.emplace<AdjacencyTo>(border_entity, p_border.first);
	// p_registry.emplace<AdjacencyFrom>(border_entity, p_border.second);
	// p_registry.emplace<ProvinceBorderType>(border_entity, border_type);
	// p_registry.emplace<ProvinceBorderMeshRID>(border_entity, p_rid);

	// ProvinceBorders &to_province_borders = p_registry.get_or_emplace<ProvinceBorders>(p_border.first, ProvinceBorders());
	// to_province_borders.push_back(border_entity);

	// ProvinceBorders &from_province_borders = p_registry.get_or_emplace<ProvinceBorders>(p_border.second, ProvinceBorders());
	// from_province_borders.push_back(border_entity);

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

void Map::create_map_labels(Node3D *p_map, int p_map_width, int p_map_height) {
	const auto province_query = ECS::self->query_builder<TextLocator, String>().with<LandProvinceTag>().build();

	province_query.each([p_map, this](flecs::entity entity, const TextLocator &locator, const String &loc_key) {
		MapLabel *label = memnew(MapLabel());

		Transform3D text_transform;
		text_transform.origin = Vector3(locator.position.x, label_map_layer, locator.position.y);
		text_transform.basis = Basis();
		text_transform.basis.scale(Vector3(locator.scale, locator.scale, locator.scale));
		text_transform.basis.rotate(Vector3(-1.570796, locator.orientation, 0.0));

		label->set_text(p_map->tr(loc_key));
		label->set_transform(text_transform);
		map_labels[entity] = label;
	});
}

void Map::create_border_meshes(Node3D *p_map, const Dictionary &p_border_dict, bool is_map_editor) {
	// Create border materials
	create_border_materials();
	RenderingServer &rs = *RS::get_singleton();
	ECS &ecs = *ECS::self;

	// Create border meshes
	const Array border_keys = p_border_dict.keys();
	border_meshes.reserve(border_keys.size());

	for (const Variant &border_key : border_keys) {
		const PackedInt32Array key = border_key;
		const PackedVector4Array value = p_border_dict[key];
		const Border border = Border(ecs.scope_lookup("p", uitos(key[0])), ecs.scope_lookup("p", uitos(key[1])));

		const Ref<ArrayMesh> border_mesh_resource = create_border_mesh(value, 0.75, 0.75);
		const RID border_mesh = border_mesh_resource->get_rid();
		const RID mesh_instance = rs.instance_create2(border_mesh, p_map->get_world_3d()->get_scenario());

		const BorderMeshStorage border_mesh_storage{ .mesh = border_mesh_resource, .instance = mesh_instance };
		border_meshes.push_back(border_mesh_storage);

		const ProvinceBorderType border_type = fill_province_border_data(border, border_mesh);
		if (!is_map_editor)
			fill_province_adjacency_data(border);

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

	ProvinceColorMap provinces_map = load_map_config();

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

	ECS &ecs = *ECS::self;

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
			const ProvinceEntity province_entity = ecs.scope_lookup("p", uitos(province_id));
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
					if (is_lake_border(Border(province_entity, ecs.scope_lookup("p", uitos(from)))))
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

					if (is_lake_border(Border(province_entity, ecs.scope_lookup("p", uitos(from)))))
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
		int province_id = atoi(kv.key.name());

		if (province_id == 0)
			return;

		if (!kv.key.has<LandProvinceTag>()) {
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

	create_border_meshes(p_map, borders_dict, true);
}

#endif

void Map::load_locators() {
	Ref<ConfigFile> text_config = memnew(ConfigFile());
	Ref<ConfigFile> unit_config = memnew(ConfigFile());

	text_config->load("res://data/locators/text.cfg");
	unit_config->load("res://data/locators/unit.cfg");

	Vector<String> text_sections = text_config->get_sections();
	Vector<String> unit_sections = unit_config->get_sections();

	ECS &ecs = *ECS::self;

	for (const String &section : text_sections) {
		const int province_id = section.to_int();
		const ProvinceEntity entity = ecs.scope_lookup("p", uitos(province_id));

		TextLocator locator;
		locator.position = text_config->get_value(section, "position");
		locator.orientation = text_config->get_value(section, "orientation");
		locator.scale = text_config->get_value(section, "scale");

		entity.set<TextLocator>(locator);
	}

	for (const String &section : unit_sections) {
		const int province_id = section.to_int();
		const ProvinceEntity entity = ecs.scope_lookup("p", uitos(province_id));

		UnitLocator locator;
		locator.position = unit_config->get_value(section, "position");
		locator.orientation = unit_config->get_value(section, "orientation");
		locator.scale = unit_config->get_value(section, "scale");

		entity.set<UnitLocator>(locator);
	}
}

template <bool is_map_editor> void Map::load_map(Node3D *p_map) {
	ECS &ecs = *ECS::self;

	// TODO - need to clear some data on reload but most data probably doesn't change
	// probably just add a "bool reloading" parameter to load_map but it's tough because some parts of load_map write to the registry.
	ecs.reset();

	ProvinceColorMap provinces_map = load_map_config();

	Ref<ConfigFile> map_data_config = memnew(ConfigFile());
	map_data_config->load("res://data/gen/map_data.cfg");

	const int province_image_width = map_data_config->get_value("map_data", "width");
	const int province_image_height = map_data_config->get_value("map_data", "height");

	// Set Map node position, makes the world coords the same as the map coords
	p_map->set_position(Vector3(province_image_width / 2.0, 0, province_image_height / 2.0));

	// Load lookup image
	Ref<CompressedTexture2D> compressed_lookup_texture = ResourceLoader::load("res://gfx/gen/province_lookup.exr");
	lookup_image = compressed_lookup_texture->get_image();
	map_mode_image = Image::create_empty(COLOR_TEXTURE_DIMENSIONS, COLOR_TEXTURE_DIMENSIONS, false, Image::FORMAT_RGBF);

	if constexpr (!is_map_editor) {
		load_locators();

		// Setup map labels
		create_map_labels(p_map, province_image_width, province_image_height);

		// Parse border crossings
		const Vector<Vector<Variant>> crossings = CSV::parse_file("res://data/crossings.txt");

		// TODO
		// Fill in crossing adjacencies
		// for (const Vector<Variant> &crossing : crossings) {
		// 	const ProvinceAdjacencyEntity adjacency_entity = registry.create();
		// 	const ProvinceEntity to_entity = registry.get_entity<ProvinceTag>(crossing[0]);
		// 	const ProvinceEntity from_entity = registry.get_entity<ProvinceTag>(crossing[1]);

		// 	registry.emplace<AdjacencyTo>(adjacency_entity, to_entity);
		// 	registry.emplace<AdjacencyFrom>(adjacency_entity, from_entity);
		// 	registry.emplace<ProvinceAdjacencyType>(adjacency_entity, ProvinceAdjacencyType::Crossing);
		// 	// clang-format off
		// 	registry.emplace<CrossingLocator>(
		// 		adjacency_entity,
		// 		Vector4(
		// 			crossing[2], crossing[3], crossing[4], crossing[5]
		// 		)
		// 	);
		// 	// clang-format on
		// }

		create_border_meshes(p_map, map_data_config->get_value("map_data", "borders"), false);
	}

	if constexpr (is_map_editor)
		create_border_meshes(p_map, map_data_config->get_value("map_data", "borders"), true);
}

Ref<ImageTexture> Map::get_lookup_texture() { return ImageTexture::create_from_image(lookup_image); }

Ref<Image> Map::get_lookup_image() { return lookup_image; }

ProvinceColorMap Map::get_color_to_id_map() { return color_to_id_map; }

Color Map::get_area_map_mode(ProvinceEntity p_province_entity) {
	bool has_label = map_labels.has(p_province_entity) ? true : false;
	MapLabel *label{};
	if (has_label) {
		label = map_labels[p_province_entity];
		label->set_visible(false);
	}

	const AreaEntity area_entity = ECS::self->get_target(p_province_entity, Relation::InArea);
	if (area_entity.is_valid()) {
		if (has_label) {
			const ProvinceEntity area_capital_entity = ECS::self->get_target(area_entity, Relation::Capital);
			// Only draw label on area capital
			if (area_capital_entity == p_province_entity) {
				label->set_text(area_entity.get<String>());
				label->set_visible(true);
			}
		}

		return area_entity.get<Color>();
	}

	return discard_color;
}

Color Map::get_region_map_mode(ProvinceEntity p_province_entity) {
	bool has_label = map_labels.has(p_province_entity) ? true : false;
	MapLabel *label{};
	if (has_label) {
		label = map_labels[p_province_entity];
		label->set_visible(false);
	}

	const RegionEntity region_entity = ECS::self->get_target(p_province_entity, Relation::InRegion);
	if (region_entity.is_valid()) {
		if (has_label) {
			const ProvinceEntity region_capital_entity = ECS::self->get_target(region_entity, Relation::Capital);
			// Only draw label on region capital
			if (region_capital_entity == p_province_entity) {
				label->set_text(region_entity.get<String>());
				label->set_visible(true);
			}
		}

		return region_entity.get<Color>();
	}

	return discard_color;
}

Color Map::get_country_map_mode(ProvinceEntity p_province_entity) {
	bool has_label = map_labels.has(p_province_entity) ? true : false;
	MapLabel *label{};
	if (has_label) {
		label = map_labels[p_province_entity];
		label->set_visible(false);
	}

	ECS &ecs = *ECS::self;

	if (ecs.has_relation(p_province_entity, Relation::Owner)) {
		const CountryEntity owner = ecs.get_target(p_province_entity, Relation::Owner);

		if (has_label) {
			const ProvinceEntity country_capital_entity = ecs.get_target(owner, Relation::Capital);

			// Only draw label on country capital
			if (country_capital_entity == p_province_entity) {
				label->set_text(owner.get<String>());
				label->set_visible(true);
			}
		}

		return owner.get<Color>();
	}

	return discard_color;
}

template Ref<ImageTexture> Map::get_map_mode<MapMode::Area>();
template Ref<ImageTexture> Map::get_map_mode<MapMode::Region>();
template Ref<ImageTexture> Map::get_map_mode<MapMode::Country>();

template <MapMode T> Ref<ImageTexture> Map::get_map_mode() {
	float *write_ptr = reinterpret_cast<float *>(map_mode_image->ptrw());

	for (uint32_t i = 1; i < color_to_id_map.size() + 1; ++i) {
		const Vector2i uv = Vector2i(i % COLOR_TEXTURE_DIMENSIONS, floor(float(i) / COLOR_TEXTURE_DIMENSIONS));
		const ProvinceEntity province_entity = ECS::self->scope_lookup("p", uitos(i));
		Color color;

		if constexpr (T == MapMode::Area)
			color = get_area_map_mode(province_entity);
		else if constexpr (T == MapMode::Region)
			color = get_region_map_mode(province_entity);
		else if constexpr (T == MapMode::Country)
			color = get_country_map_mode(province_entity);

		const uint32_t ofs = (uv.y * COLOR_TEXTURE_DIMENSIONS) + uv.x;
		write_ptr[(ofs * 3) + 0] = color.r;
		write_ptr[(ofs * 3) + 1] = color.g;
		write_ptr[(ofs * 3) + 2] = color.b;
	}

	return ImageTexture::create_from_image(map_mode_image);
}

Map::~Map() {
	for (const BorderMeshStorage &border_mesh : border_meshes)
		RS::get_singleton()->free(border_mesh.instance);

	for (const RID &material_rid : border_materials)
		RS::get_singleton()->free(material_rid);

	for (const auto &kv : map_labels)
		if (kv.value != nullptr)
			memdelete(kv.value);
	map_labels.clear();
}
