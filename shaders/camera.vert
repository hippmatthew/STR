#version 460

layout( push_constant ) uniform Camera {
  mat4 view;
  vec3 near_plane_dimensions;
} camera;

layout(location = 0) in vec2 pos;

layout(location = 0) out vec3 origin;
layout(location = 1) out vec3 vPos;

void main() {
  vec4 vsPos = vec4(pos, 1.0, 1.0);
  vsPos.xyz *= camera.near_plane_dimensions;
  vsPos = inverse(camera.view) * vsPos;

  gl_Position = vec4(pos, 0.0, 1.0);
  origin = camera.view[3].xyz;
  vPos = vsPos.xyz;
}