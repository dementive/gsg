#include "MapLabel.hpp"

#include "scene/resources/font.h"
#include "scene/theme/theme_db.h"

#include "cg/NodeManager.hpp"

#include "nodes/Map3D.hpp"

using namespace CG;

Ref<Font> MapLabel::_get_font_or_default() const {
	const StringName theme_name = SceneStringName(font);
	Vector<StringName> theme_types;
	ThemeDB::get_singleton()->get_native_type_dependencies("Label3D", theme_types);

	ThemeContext *global_context = ThemeDB::get_singleton()->get_default_theme_context();
	Vector<Ref<Theme>> themes = global_context->get_themes();
	if (Engine::get_singleton()->is_editor_hint())
		themes.insert(0, ThemeDB::get_singleton()->get_project_theme());

	for (const Ref<Theme> &theme : themes) {
		if (theme.is_null())
			continue;

		for (const StringName &E : theme_types) {
			if (!theme->has_font(theme_name, E))
				continue;

			Ref<Font> f = theme->get_font(theme_name, E);
			return f;
		}
	}

	Ref<Font> f = global_context->get_fallback_theme()->get_font(theme_name, StringName());
	return f;
}

void MapLabel::_generate_glyph_surfaces(const Glyph &p_glyph, Vector2 &r_offset, const Color &p_modulate, int p_priority, int p_outline_size) {
	if (p_glyph.index == 0) {
		r_offset.x += p_glyph.advance * pixel_size * p_glyph.repeat; // Non visual character, skip.
		return;
	}

	Vector2 gl_of;
	Vector2 gl_sz;
	Rect2 gl_uv;
	Size2 texs;
	RID tex;

	if (p_glyph.font_rid.is_valid()) {
		tex = TS->font_get_glyph_texture_rid(p_glyph.font_rid, Vector2i(p_glyph.font_size, p_outline_size), p_glyph.index);
		if (tex.is_valid()) {
			gl_of = (TS->font_get_glyph_offset(p_glyph.font_rid, Vector2i(p_glyph.font_size, p_outline_size), p_glyph.index) + Vector2(p_glyph.x_off, p_glyph.y_off)) * pixel_size;
			gl_sz = TS->font_get_glyph_size(p_glyph.font_rid, Vector2i(p_glyph.font_size, p_outline_size), p_glyph.index) * pixel_size;
			gl_uv = TS->font_get_glyph_uv_rect(p_glyph.font_rid, Vector2i(p_glyph.font_size, p_outline_size), p_glyph.index);
			texs = TS->font_get_glyph_texture_size(p_glyph.font_rid, Vector2i(p_glyph.font_size, p_outline_size), p_glyph.index);
		}
	} else if (((p_glyph.flags & TextServer::GRAPHEME_IS_VIRTUAL) != TextServer::GRAPHEME_IS_VIRTUAL) &&
			((p_glyph.flags & TextServer::GRAPHEME_IS_EMBEDDED_OBJECT) != TextServer::GRAPHEME_IS_EMBEDDED_OBJECT)) {
		gl_sz = TS->get_hex_code_box_size(p_glyph.font_size, p_glyph.index) * pixel_size;
		gl_of = Vector2(0, -gl_sz.y);
	}

	if (gl_uv.size.x <= 2 || gl_uv.size.y <= 2) {
		r_offset.x += p_glyph.advance * pixel_size * p_glyph.repeat; // Nothing to draw.
		return;
	}

	bool msdf = TS->font_is_multichannel_signed_distance_field(p_glyph.font_rid);

	for (int j = 0; j < p_glyph.repeat; j++) {
		SurfaceKey key = SurfaceKey(tex.get_id(), p_priority, p_outline_size);
		if (!surfaces.has(key)) {
			SurfaceData surf;
			surf.material = RenderingServer::get_singleton()->material_create();
			// Set defaults for material, names need to match up those in StandardMaterial3D
			RS::get_singleton()->material_set_param(surf.material, "albedo", Color(1, 1, 1, 1));
			RS::get_singleton()->material_set_param(surf.material, "specular", 0.5);
			RS::get_singleton()->material_set_param(surf.material, "metallic", 0.0);
			RS::get_singleton()->material_set_param(surf.material, "roughness", 1.0);
			RS::get_singleton()->material_set_param(surf.material, "uv1_offset", Vector3(0, 0, 0));
			RS::get_singleton()->material_set_param(surf.material, "uv1_scale", Vector3(1, 1, 1));
			RS::get_singleton()->material_set_param(surf.material, "uv2_offset", Vector3(0, 0, 0));
			RS::get_singleton()->material_set_param(surf.material, "uv2_scale", Vector3(1, 1, 1));
			RS::get_singleton()->material_set_param(surf.material, "alpha_scissor_threshold", alpha_scissor_threshold);
			RS::get_singleton()->material_set_param(surf.material, "alpha_hash_scale", alpha_hash_scale);
			RS::get_singleton()->material_set_param(surf.material, "alpha_antialiasing_edge", alpha_antialiasing_edge);
			if (msdf) {
				RS::get_singleton()->material_set_param(surf.material, "msdf_pixel_range", TS->font_get_msdf_pixel_range(p_glyph.font_rid));
				RS::get_singleton()->material_set_param(surf.material, "msdf_outline_size", p_outline_size);
			}

			BaseMaterial3D::Transparency mat_transparency = BaseMaterial3D::Transparency::TRANSPARENCY_ALPHA;

			RID shader_rid;
			StandardMaterial3D::get_material_for_2d(
					false, mat_transparency, false, false, false, msdf, false, false, StandardMaterial3D::TEXTURE_FILTER_LINEAR, StandardMaterial3D::ALPHA_ANTIALIASING_OFF, &shader_rid);

			RS::get_singleton()->material_set_shader(surf.material, shader_rid);
			RS::get_singleton()->material_set_param(surf.material, "texture_albedo", tex);
			RS::get_singleton()->material_set_param(surf.material, "albedo_texture_size", texs);
			surf.z_shift = p_priority * pixel_size;

			surfaces[key] = surf;
		}
		SurfaceData &s = surfaces[key];

		s.mesh_vertices.resize((s.offset + 1) * 4);
		s.mesh_normals.resize((s.offset + 1) * 4);
		s.mesh_tangents.resize((s.offset + 1) * 16);
		s.mesh_colors.resize((s.offset + 1) * 4);
		s.mesh_uvs.resize((s.offset + 1) * 4);

		s.mesh_vertices.write[(s.offset * 4) + 3] = Vector3(r_offset.x + gl_of.x, r_offset.y - gl_of.y - gl_sz.y, s.z_shift);
		s.mesh_vertices.write[(s.offset * 4) + 2] = Vector3(r_offset.x + gl_of.x + gl_sz.x, r_offset.y - gl_of.y - gl_sz.y, s.z_shift);
		s.mesh_vertices.write[(s.offset * 4) + 1] = Vector3(r_offset.x + gl_of.x + gl_sz.x, r_offset.y - gl_of.y, s.z_shift);
		s.mesh_vertices.write[(s.offset * 4) + 0] = Vector3(r_offset.x + gl_of.x, r_offset.y - gl_of.y, s.z_shift);

		for (int i = 0; i < 4; i++) {
			s.mesh_normals.write[(s.offset * 4) + i] = Vector3(0.0, 0.0, 1.0);
			s.mesh_tangents.write[(s.offset * 16) + (i * 4) + 0] = 1.0;
			s.mesh_tangents.write[(s.offset * 16) + (i * 4) + 1] = 0.0;
			s.mesh_tangents.write[(s.offset * 16) + (i * 4) + 2] = 0.0;
			s.mesh_tangents.write[(s.offset * 16) + (i * 4) + 3] = 1.0;
			s.mesh_colors.write[(s.offset * 4) + i] = p_modulate;
			s.mesh_uvs.write[(s.offset * 4) + i] = Vector2();
		}

		if (tex.is_valid()) {
			s.mesh_uvs.write[(s.offset * 4) + 3] = Vector2(gl_uv.position.x / texs.x, (gl_uv.position.y + gl_uv.size.y) / texs.y);
			s.mesh_uvs.write[(s.offset * 4) + 2] = Vector2((gl_uv.position.x + gl_uv.size.x) / texs.x, (gl_uv.position.y + gl_uv.size.y) / texs.y);
			s.mesh_uvs.write[(s.offset * 4) + 1] = Vector2((gl_uv.position.x + gl_uv.size.x) / texs.x, gl_uv.position.y / texs.y);
			s.mesh_uvs.write[(s.offset * 4) + 0] = Vector2(gl_uv.position.x / texs.x, gl_uv.position.y / texs.y);
		}

		s.indices.resize((s.offset + 1) * 6);
		s.indices.write[(s.offset * 6) + 0] = (s.offset * 4) + 0;
		s.indices.write[(s.offset * 6) + 1] = (s.offset * 4) + 1;
		s.indices.write[(s.offset * 6) + 2] = (s.offset * 4) + 2;
		s.indices.write[(s.offset * 6) + 3] = (s.offset * 4) + 0;
		s.indices.write[(s.offset * 6) + 4] = (s.offset * 4) + 2;
		s.indices.write[(s.offset * 6) + 5] = (s.offset * 4) + 3;

		s.offset++;
		r_offset.x += p_glyph.advance * pixel_size;
	}
}

void MapLabel::_shape() {
	// When a shaped text is invalidated by an external source, we want to reshape it.
	if (!TS->shaped_text_is_ready(text_rid))
		dirty_text = true;

	for (const RID &line_rid : lines_rid) {
		if (!TS->shaped_text_is_ready(line_rid)) {
			dirty_lines = true;
			break;
		}
	}

	// Clear mesh.
	RS::get_singleton()->mesh_clear(mesh);

	// Clear materials.
	for (const KeyValue<SurfaceKey, SurfaceData> &E : surfaces)
		RenderingServer::get_singleton()->free(E.value.material);
	surfaces.clear();

	Ref<Font> font = _get_font_or_default();
	ERR_FAIL_COND(font.is_null());

	// Update text buffer.
	if (dirty_text) {
		TS->shaped_text_clear(text_rid);
		TS->shaped_text_set_direction(text_rid, TextServer::DIRECTION_AUTO);

		String txt = (uppercase) ? TS->string_to_upper(xl_text, "") : xl_text;
		TS->shaped_text_add_string(text_rid, txt, font->get_rids(), font_size, font->get_opentype_features(), "");

		TypedArray<Vector3i> stt;
		stt = TS->parse_structured_text(TextServer::STRUCTURED_TEXT_DEFAULT, Array(), txt);
		TS->shaped_text_set_bidi_override(text_rid, stt);

		dirty_text = false;
		dirty_font = false;
		dirty_lines = true;
	} else if (dirty_font) {
		int spans = TS->shaped_get_span_count(text_rid);
		for (int i = 0; i < spans; i++)
			TS->shaped_set_span_update_font(text_rid, i, font->get_rids(), font_size, font->get_opentype_features());

		dirty_font = false;
		dirty_lines = true;
	}

	if (dirty_lines) {
		for (const RID i : lines_rid)
			TS->free_rid(i);
		lines_rid.clear();

		BitField<TextServer::LineBreakFlag> autowrap_flags = TextServer::BREAK_MANDATORY | TextServer::BREAK_TRIM_START_EDGE_SPACES | TextServer::BREAK_TRIM_END_EDGE_SPACES;

		PackedInt32Array line_breaks = TS->shaped_text_get_line_breaks(text_rid, width, 0, autowrap_flags);
		float max_line_w = 0.0;
		for (int i = 0; i < line_breaks.size(); i = i + 2) {
			RID line = TS->shaped_text_substr(text_rid, line_breaks[i], line_breaks[i + 1] - line_breaks[i]);
			max_line_w = MAX(max_line_w, TS->shaped_text_get_width(line));
			lines_rid.push_back(line);
		}
		dirty_lines = false;
	}

	// Generate surfaces and materials.
	float total_h = 0.0;
	for (const RID i : lines_rid)
		total_h += (TS->shaped_text_get_size(i).y + line_spacing) * pixel_size;
	float vbegin = (total_h - line_spacing * pixel_size) / 2.0;
	Vector2 offset = Vector2(0, vbegin * pixel_size);
	for (const RID i : lines_rid) {
		const Glyph *glyphs = TS->shaped_text_get_glyphs(i);
		int gl_size = TS->shaped_text_get_glyph_count(i);
		float line_width = TS->shaped_text_get_width(i) * pixel_size;
		offset.x = -line_width / 2.0;
		offset.x += pixel_size;
		offset.y -= TS->shaped_text_get_ascent(i) * pixel_size;

		if (outline_modulate.a != 0.0 && outline_size > 0) {
			// Outline surfaces.
			Vector2 ol_offset = offset;
			for (int j = 0; j < gl_size; j++)
				_generate_glyph_surfaces(glyphs[j], ol_offset, outline_modulate, outline_render_priority, outline_size);
		}

		// Main text surfaces.
		for (int j = 0; j < gl_size; j++)
			_generate_glyph_surfaces(glyphs[j], offset, modulate, render_priority);
		offset.y -= (TS->shaped_text_get_descent(i) + line_spacing) * pixel_size;
	}

	for (const KeyValue<SurfaceKey, SurfaceData> &E : surfaces) {
		Array mesh_array;
		mesh_array.resize(RS::ARRAY_MAX);
		mesh_array[RS::ARRAY_VERTEX] = E.value.mesh_vertices;
		mesh_array[RS::ARRAY_NORMAL] = E.value.mesh_normals;
		mesh_array[RS::ARRAY_TANGENT] = E.value.mesh_tangents;
		mesh_array[RS::ARRAY_COLOR] = E.value.mesh_colors;
		mesh_array[RS::ARRAY_TEX_UV] = E.value.mesh_uvs;
		mesh_array[RS::ARRAY_INDEX] = E.value.indices;

		RS::SurfaceData sd;
		RS::get_singleton()->mesh_create_surface_data_from_arrays(&sd, RS::PRIMITIVE_TRIANGLES, mesh_array);

		sd.material = E.value.material;

		RS::get_singleton()->mesh_add_surface(mesh, sd);
	}
}

MapLabel::MapLabel() {
	text_rid = TS->create_shaped_text();
	mesh = RS::get_singleton()->mesh_create();
	instance = RS::get_singleton()->instance_create2(mesh, NM::map->get_world_3d()->get_scenario());

	RS::get_singleton()->instance_geometry_set_flag(instance, RS::INSTANCE_FLAG_USE_BAKED_LIGHT, false);
	RS::get_singleton()->instance_geometry_set_flag(instance, RS::INSTANCE_FLAG_USE_DYNAMIC_GI, false);
	RS::get_singleton()->instance_geometry_set_cast_shadows_setting(instance, RS::ShadowCastingSetting::SHADOW_CASTING_SETTING_OFF);
}

MapLabel::~MapLabel() {
	for (const RID line : lines_rid)
		TS->free_rid(line);
	lines_rid.clear();
	TS->free_rid(text_rid);

	RS::get_singleton()->free(instance);
	RenderingServer::get_singleton()->free(mesh);
	for (const KeyValue<SurfaceKey, SurfaceData> &E : surfaces)
		RenderingServer::get_singleton()->free(E.value.material);
	surfaces.clear();
}

void MapLabel::set_text(const String &p_string) {
	if (text == p_string)
		return;

	text = p_string;
	xl_text = NM::map->atr(p_string);
	dirty_text = true;
	_shape();
}

void MapLabel::set_font_size(int p_size) {
	if (font_size != p_size) {
		font_size = p_size;
		dirty_font = true;
		_shape();
	}
}

void MapLabel::set_visible(bool p_visible) { RS::get_singleton()->instance_set_visible(instance, p_visible); }

void MapLabel::set_transform(const Transform3D &p_transform) { RS::get_singleton()->instance_set_transform(instance, p_transform); }
