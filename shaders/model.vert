varying float diffuse_value;
uniform vec4 blend_color;
attribute vec3 face_normal;
uniform vec3 default_color;

float FogEyeRadial(vec4 Rh)
{
    vec4 Re = Rh / Rh.w;
    return length(Re);
}

void main() {
    vec3 vertex_light_position = gl_LightSource[0].position.xyz;
    vec3 vertex_normal;
    vec3 normal = vec3(gl_Normal.x, gl_Normal.z, gl_Normal.y);
    diffuse_value = 0.0;
    if (normal.x == 0.0 && normal.y == 0.0 && normal.z == 0.0) {
        //no shading
        diffuse_value = 0.0;
    } else {
        vertex_normal = normalize(gl_NormalMatrix * normal);
        diffuse_value = dot(vertex_normal, vertex_light_position);
    }
    vertex_normal = normalize(gl_NormalMatrix * face_normal);
    diffuse_value += dot(vertex_normal, vertex_light_position);
    vec4 color = gl_Color;
    if (color.r == 0.0 && color.g == 0.0 && color.b == 0.0)
        color.rgb = default_color;
    gl_FrontColor = color * blend_color;
    gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
    gl_FogFragCoord = FogEyeRadial(gl_ModelViewMatrix * gl_Vertex);
}