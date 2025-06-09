#pragma once

#include "core/io/image.h"
#include "core/templates/a_hash_map.h"

#include "scene/resources/image_texture.h"

#include "ecs/Entity.hpp"

#include "templates/Vec.hpp"

#include "defs/singleton.hpp"

class SurfaceTool;
class ArrayMesh;
class ShaderMaterial;

namespace CG {

class Map3D;
struct Registry;
enum class ProvinceBorderType : uint8_t;

using ProvinceIndex = int; // Index into the province lookup texture.
using ProvinceColorMap = AHashMap<Color, ProvinceIndex>;

class Map {
	SINGLETON(Map)
private:
	using Polygon = Vector<Vector2>;
	using Border = Pair<ProvinceEntity, ProvinceEntity>;

	static Vector2 calculate_centroid(const Polygon &p_polygon);
	static float calculate_orientation(const Polygon &p_polygon, const Vector2 &p_centroid);
	static Color get_random_area_color();
	static Color get_lookup_color(ProvinceIndex p_province_id);
	ProvinceColorMap load_map_config(Registry &p_registry);

	static bool is_lake_border(const Registry &p_registry, const Border &p_border);
	static void fill_province_adjacency_data(Registry &p_registry, const Border &p_border);
	static ProvinceBorderType fill_province_border_data(Registry &p_registry, const Border &p_border, const RID &p_rid);

	static void add_rounded_border_corners(Ref<SurfaceTool> &p_st, const Vector2 &p_v1, const Vector2 &p_v2, float p_radius);
	static Vector<Ref<ShaderMaterial>> create_border_materials();
	static Ref<ArrayMesh> create_border_mesh(const Vec<Vector4> &p_segments, float p_border_thickness, float p_border_rounding);
	static void create_map_labels(const Registry &p_registry, Map3D *p_map, int p_map_width, int p_map_height);

public:
	void load_map(Map3D *p_map);
	Ref<Image> get_lookup_image();
	Ref<ImageTexture> get_lookup_texture();
	ProvinceColorMap get_color_to_id_map();
	Ref<ImageTexture> get_country_map_mode();

private:
	ProvinceColorMap color_to_id_map; // lookup image color -> province id
	Ref<Image> lookup_image;
};

} // namespace CG