shader_type spatial;

uniform sampler2D edge_clouds_mask_texture : source_color, filter_linear_mipmap;
uniform sampler2D edge_clouds_tile_texture : source_color, filter_linear_mipmap;

vec4 apply_edge_clouds(vec2 uv) {
    float edge_clouds_mask = texture(edge_clouds_mask_texture, uv).a;
    vec3 edge_tile = texture(edge_clouds_tile_texture, uv * 5.0).rgb;
    return vec4(edge_tile, edge_clouds_mask);
}

void fragment() {
	vec4 edge_clouds = apply_edge_clouds(UV);
	ALBEDO = edge_clouds.rgb;
	ALPHA = edge_clouds.a;
}
