shader_type spatial;
render_mode cull_back;

//#define SHOW_PROVINCE_MAP
//#define SHOW_LOOKUP_TEXTURE

#ifdef SHOW_PROVINCE_MAP
uniform sampler2D province_texture : source_color, filter_linear;
#endif
uniform sampler2D lookup_texture : source_color, filter_nearest;
uniform sampler2D color_texture : source_color, filter_nearest;
uniform sampler2D flatmap_texture : source_color, filter_linear_mipmap;
uniform sampler2D texture_normal : hint_roughness_normal, filter_linear_mipmap, repeat_enable;
uniform float normal_scale : hint_range(-4.0, 4.0) = 1.0;
uniform vec4 selected_areas[10]: source_color;
uniform int selected_areas_total = 0;

const vec3 discard_color = vec3(0,0,0); // must be the same as discard_color in map.gd

bool is_same_color(vec3 a, vec3 b) {
	vec3 diff = abs(a - b);
	return  diff.r < 0.000001 &&  diff.g < 0.000001 &&  diff.b < 0.000001;
}

void fragment() {
	vec4 lookup_uv = texture(lookup_texture, UV);
	vec4 flatmap_color = texture(flatmap_texture, UV);
	vec3 color = texture(color_texture, vec2(lookup_uv.x, lookup_uv.y)).rgb;

	// Apply flatmap color
	color *= flatmap_color.rgb;
	if (is_same_color(color, discard_color)) {
		color = flatmap_color.rgb;
	}

	// For selected area
	for (int i = 0; i < selected_areas_total; i++) {
		if(is_same_color(lookup_uv.rgb, selected_areas[i].rgb)){
			color *= 1.5;
		}
	}

	ALBEDO = color;

	NORMAL_MAP = texture(texture_normal, UV).rgb;
	NORMAL_MAP_DEPTH = normal_scale;

#ifdef SHOW_PROVINCE_MAP
	ALBEDO = texture(province_texture, UV).rgb;
#endif

#ifdef SHOW_LOOKUP_TEXTURE
	ALBEDO = lookup_uv.rgb;
#endif

}
