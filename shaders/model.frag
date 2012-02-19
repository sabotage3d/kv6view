varying float diffuse_value;
uniform vec3 default_color;

void main() {
    vec4 color = gl_Color;
    // max mult: 1.3, min mult: 0.6
    color.rgb = color.rgb * ((diffuse_value + 1.0) * 0.40 + 0.6);
    color = mix(gl_Fog.color, color, saturate((gl_Fog.end - gl_FogFragCoord)
        * gl_Fog.scale));
    gl_FragColor = color;
}