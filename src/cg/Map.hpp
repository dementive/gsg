#pragma once

#include "core/templates/hash_map.h"

class Image;

namespace CG {

const int COLOR_TEXTURE_DIMENSIONS = 255;
const float border_map_layer = 0.01;
const float label_map_layer = 0.015;
const Color discard_color = Color(0,0,0);

class SurfaceTool;
class Mesh;
class Map3D;
class ShaderMaterial;

enum ProvinceBorderType {};

class Map {
private:
	using Polygon = Vector<Vector2>;
	using Border = Pair<int, int>;
	using ProvinceColorMap = HashMap<Color, int>;

	ProvinceColorMap color_to_id_map; // lookup image color -> province id
	Image *look_up_image{};

	// var provinces: Provinces
	// var province_adjacencies: ProvinceAdjacencies
	// var province_borders: ProvinceBorders
	// var countries: Countries
	// var areas: Areas
	// var regions: Regions
	Vector2 calculate_centroid(Polygon p_polygon);
	float calculate_orientation(Polygon p_polygon, Vector2 p_centroid);
	Color get_random_area_color();
	ProvinceColorMap load_map_config();

	bool is_lake_border(Border p_border);
	Vector<ShaderMaterial> create_border_materials();
	void fill_province_adjacency_data(int p_adjacency_id, Border p_border);
	ProvinceBorderType fill_province_border_data(int p_border_id, Border p_border);

	Mesh create_border_mesh(Vector<Vector4> p_segments, float p_border_thickness,  float p_border_rounding);
	void add_rounded_border_corners(SurfaceTool *p_st, Vector2 p_v1, Vector2 p_v2, float p_radius);
	void create_map_labels(Map3D *p_map, int p_map_width, int p_map_height);

public:
	void load_map(Map3D *p_map);
	Image get_lookup_image();
	ProvinceColorMap get_color_to_id_map();
};
}