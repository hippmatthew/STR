#version 460

layout( push_constant ) uniform Variables {
  mat4 mvp;
} vars;

layout (location = 0) in vec3 position;

mat4 matrix = mat4(
  vec4(1.0, 0.0, 0.0, 0.0),
  vec4(0.0, 1.0, 0.0, 0.0),
  vec4(0.0, 0.0, 1.0, 0.0),
  vec4(0.0, 0.0, 0.0, 1.0)
);

void main()
{
  gl_Position = vars.mvp * vec4(position, 1.0);
}