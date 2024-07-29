#version 460

struct Transform {
  uint shape;
  float smoothness;
  vec3 color;
  vec3 size;
  vec3 position;
  vec3 rotation;
};

struct Ray {
  vec3 origin;
  vec3 dir;
  vec3 color;
};

struct HitInfo {
  bool hit;
  float t;
  vec3 point;
  vec3 normal;
  vec3 color;
};

layout(set = 0, binding = 0) buffer TransformSSBO {
  uint size;
  Transform transforms[];
} ssbo;

layout( push_constant ) uniform Camera {
  mat4 view;
  mat4 proj;
} camera;

layout(location = 0) in vec3 pos;

layout(location = 0) out vec3 vPos;

void main() {
  vec4 psPos = camera.proj * camera.view * vec4(pos, 1.0);

  gl_Position = vec4(pos, 1.0);
  vPos = vec3(psPos.xyz / psPos.w);
}