// CRT-like Post Process Shader for SFML
// Ported from godotshaders.com

uniform sampler2D texture; // the input texture
uniform vec4 scanline_color; 
uniform vec4 flicker_color;
uniform float scanlines_count;
uniform float scanlines_intensity;
uniform float flicker_speed;
uniform float flicker_intensity;
uniform float color_offset;
uniform float blur;
uniform float time;
uniform vec2 texture_size; // size of the input texture

void main()
{
    vec2 uv = gl_TexCoord[0].xy;
    vec2 pixel_size = 1.0 / texture_size;
    float offset = color_offset * pixel_size.x;

    // Chromatic aberration
    float channel_r = texture2D(texture, uv + vec2(offset, 0.0)).r;
    float channel_g = texture2D(texture, uv).g;
    float channel_b = texture2D(texture, uv + vec2(-offset, 0.0)).b;
    vec4 aberrate = vec4(channel_r, channel_g, channel_b, 1.0);

    // Flicker effect
    float mix_value = sin((uv.y - (time * flicker_speed)) * 10.0) * flicker_intensity;
    vec4 flicker = mix(aberrate, flicker_color, mix_value);

    // Scanlines
    float scanlines = sin(gl_FragCoord.y * scanlines_count) * scanlines_intensity;
    vec4 final_color = mix(flicker, scanline_color, scanlines);

    gl_FragColor = final_color;
}
