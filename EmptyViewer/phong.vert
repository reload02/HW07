#version 330 core
layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
uniform mat4 modeling;
uniform mat4 modeling_inv_tr;
uniform mat4 camera;
uniform mat4 projection;
out vec3 fragPos;
out vec3 fragNormal;
void main() {
    vec4 worldPos = modeling * vec4(position, 1.0);
    fragPos = vec3(worldPos);
    fragNormal = mat3(modeling_inv_tr) * normal;
    gl_Position = projection * camera * worldPos;
}
