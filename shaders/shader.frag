#version 460

layout(set = 0, binding = 0) uniform Color {
  vec3 value;
} color;

layout(location = 0) out vec4 fColor;

void main()
{
  fColor = vec4(color.value, 1.0);
}