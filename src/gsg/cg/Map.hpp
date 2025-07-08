#pragma once

#include "core/io/image.h"
#include "core/templates/a_hash_map.h"

#include "scene/resources/image_texture.h"

#include "ecs/entity.hpp"

#include "templates/Vec.hpp"

#include "defs/singleton.hpp"

class SurfaceTool;
class ArrayMesh;
class ShaderMaterial;
class Node3D;

namespace CG {

class MapLabel;
enum class ProvinceBorderType : uint8_t;
enum class MapMode : uint8_t;

using ProvinceIndex = int; // Index into the province lookup texture.
using ProvinceColorMap = AHashMap<Color, ProvinceIndex>;

static constexpr float border_map_layer = 0.01;
static constexpr float label_map_layer = 0.015;
static constexpr float unit_map_layer = 15.0;
static constexpr float unit_x_rotation = -1.308997;

class Map {
	SINGLETON(Map)
private:
	using Polygon = Vector<Vector2>;
	using Border = Pair<ProvinceEntity, ProvinceEntity>;

	struct CachedMapData {
		float orientation{};
		AABB aabb;
	};

	static Vector2 calculate_centroid(const Polygon &p_polygon);
	static CachedMapData calc_map_data(const Polygon &p_polygon, const Vector2 &p_centroid);

	static Color get_random_area_color();
	static Color get_lookup_color(ProvinceIndex p_province_id);
	ProvinceColorMap load_map_config();

	static bool is_lake_border(const Border &p_border);
	static void fill_province_adjacency_data(const Border &p_border);
	static ProvinceBorderType fill_province_border_data(const Border &p_border, const RID &p_rid);

	static void add_rounded_border_corners(Ref<SurfaceTool> &p_st, const Vector2 &p_v1, const Vector2 &p_v2, float p_radius);
	void create_border_materials();
	static Ref<ArrayMesh> create_border_mesh(const Vec<Vector4> &p_segments, float p_border_thickness, float p_border_rounding);
	void create_map_labels();
	static void create_unit_models(Node3D *p_map);
	void create_border_meshes(const RID &p_scenario, const Dictionary &p_border_dict, bool is_map_editor);
	static void load_locators();
	static void load_map_data();

	Color get_country_map_mode(ProvinceEntity p_province_entity);
	Color get_area_map_mode(ProvinceEntity p_province_entity);
	Color get_region_map_mode(ProvinceEntity p_province_entity);

public:
	template <bool is_map_editor> void load_map(Node3D *p_map);

#ifdef TOOLS_ENABLED
	void load_map_editor(Node3D *p_map);
#endif

	Ref<Image> get_lookup_image();
	Ref<ImageTexture> get_lookup_texture();
	ProvinceColorMap get_color_to_id_map();
	template <MapMode T> Ref<ImageTexture> get_map_mode();

	~Map();

private:
	ProvinceColorMap color_to_id_map; // lookup image color -> province id
	Ref<Image> lookup_image;
	Ref<Image> map_mode_image;

	struct BorderMeshStorage {
		Ref<ArrayMesh> mesh;
		RID instance;
	};
	Vec<BorderMeshStorage> border_meshes;
	Vec<RID> border_materials;
	AHashMap<ProvinceEntity, MapLabel *, EntityHasher> map_labels;
};

} // namespace CG