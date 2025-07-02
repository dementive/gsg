#pragma once

#include "core/templates/hash_map.h"
#include "core/variant/variant.h"

struct Glyph;
class Font;

namespace CG {

// 3D map label. Same as Label3D except it's not a Node and is simpler. Most of this code is stolen from Label3D/GeometryInstance3D
class MapLabel {
private:
	RID instance;
	RID mesh;

	String text;
	String xl_text;

	struct SurfaceData {
		PackedVector3Array mesh_vertices;
		PackedVector3Array mesh_normals;
		PackedFloat32Array mesh_tangents;
		PackedColorArray mesh_colors;
		PackedVector2Array mesh_uvs;
		PackedInt32Array indices;
		int offset = 0;
		float z_shift = 0.0;
		RID material;
	};

	struct SurfaceKey {
		uint64_t texture_id;
		int32_t priority;
		int32_t outline_size;

		bool operator==(const SurfaceKey &p_b) const { return (texture_id == p_b.texture_id) && (priority == p_b.priority) && (outline_size == p_b.outline_size); }

		SurfaceKey(uint64_t p_texture_id, int p_priority, int p_outline_size) :
				texture_id(p_texture_id),
				priority(p_priority),
				outline_size(p_outline_size) {}
	};

	struct SurfaceKeyHasher {
		_FORCE_INLINE_ static uint32_t hash(const SurfaceKey &p_a) { return hash_murmur3_buffer(&p_a, sizeof(SurfaceKey)); }
	};

	HashMap<SurfaceKey, SurfaceData, SurfaceKeyHasher> surfaces;

	RID text_rid;
	Vector<RID> lines_rid;

	RID base_material;
	AABB aabb;
	AABB province_aabb;
	Transform3D transform;

	bool dirty_lines = true;
	bool dirty_font = true;
	bool dirty_text = true;

	static constexpr int font_size = 32;
	static constexpr int outline_size = 1;
	inline static Color outline_modulate = Color(0, 0, 0, 0);
	inline static Color modulate = Color(0, 0, 0, 0.93);

	static constexpr int render_priority = 2;
	static constexpr int outline_render_priority = 1;
	static constexpr float line_spacing = 0.F;
	static constexpr float pixel_size = 0.005;
	static constexpr float alpha_scissor_threshold = 0.5;
	static constexpr float alpha_hash_scale = 1.0;
	static constexpr float alpha_antialiasing_edge = 0.0;
	static constexpr bool uppercase = false;
	static constexpr float width = 500.0;

	Ref<Font> _get_font_or_default() const;
	void _generate_glyph_surfaces(const Glyph &p_glyph, Vector2 &r_offset, const Color &p_modulate, int p_priority = 0, int p_outline_size = 0);
	void _shape();

public:
	void set_text(const String &p_string);
	void set_visible(bool p_visible);
	void set_transform(const Transform3D &p_transform);
	AABB get_aabb() const;

	// Set the province AABB before doing anything else.
	void set_province_aabb(const AABB &p_aabb);

	MapLabel();
	~MapLabel();
};

} // namespace CG
