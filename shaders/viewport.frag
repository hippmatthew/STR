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

layout(location = 0) in vec3 vPos;

layout(location = 0) out vec4 fColor;

const uint MAX_BOUNCES = 1;

HitInfo rayToSphere(Transform transform, Ray ray) {
  vec3 L = ray.origin - transform.position;
  float R = transform.size[0];

  float b = 2 * dot(L, ray.dir);
  float c = dot(L, L) - R * R;
  float disc = b * b - 4 * c;

  if (disc < 0) {
    return HitInfo(
      false,
      0.0,
      vec3(0.0, 0.0, 0.0),
      vec3(0.0, 0.0, 0.0),
      vec3(0.0, 0.0, 0.0)
    );
  }

  float t = min(-(b + sqrt(disc)) / 2, (-b + sqrt(disc)) / 2);

  vec3 P = ray.origin + t * ray.dir;
  vec3 normal = normalize(P - transform.position);

  return HitInfo(
    true,
    t,
    P,
    normal,
    transform.color
  );
}

void main()
{
  Ray ray = Ray(
    vec3(camera.view[3]),
    vPos,
    vec3(0.0, 0.0, 0.0)
  );

  for (uint i = 0; i < MAX_BOUNCES; ++i) {
    HitInfo hit = HitInfo(
      false,
      0.0,
      vec3(0.0, 0.0, 0.0),
      vec3(0.0, 0.0, 0.0),
      vec3(0.0, 0.0, 0.0)
    );

    for (uint j = 0; j < ssbo.size; ++j) {
      HitInfo info = rayToSphere(ssbo.transforms[j], ray);

      if (!info.hit) continue;

      if (info.t < hit.t) {
        hit = info;
        continue;
      }
    }

    if (!hit.hit) {
      break;
    }

    int invert = 1;
    if (dot(ray.dir, hit.normal) < 0) {
      invert = -1;
    }

    vec3 dir = invert * normalize(ray.dir - 2 * dot(ray.dir, hit.normal) * hit.normal);
    vec3 color = hit.color;

    ray = Ray(
      hit.point,
      dir,
      color
    );
  }

  fColor = vec4(camera.proj[0].xyz, 1.0);
}