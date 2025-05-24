shader_type spatial;
render_mode cull_back, unshaded;

uniform vec4 border_color: source_color = vec4(0.0, 0.0, 0.0, 1.0);

void fragment() {
    ALBEDO = border_color.rgb;
    ALPHA = border_color.a;
}