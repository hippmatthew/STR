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

layout(location = 0) in vec3 origin;
layout(location = 1) in vec3 vPos;

layout(location = 0) out vec4 fColor;

const float inf = float(1.0 / 0.0);
const vec3 SKY_LIGHT = vec3(0.5294, 0.8078, 0.9216);
const vec3 SKY_DARK = vec3(0.0980, 0.0980, 0.4392);
const uint MAX_BOUNCES = 10;

HitInfo RaySphere(Ray, Transform);
Ray trace(Ray);

void main() {
  Ray ray = Ray(
    origin,
    normalize(vPos - origin),
    vec3(0.0, 0.0, 0.0)
  );

  ray = trace(ray);

  fColor = vec4(ray.color, 1.0);
}

HitInfo RaySphere(Transform transform, Ray ray) {
  vec3 O = ray.origin - transform.position;
  float R = transform.size[0];

  float b = 2 * dot(O, ray.dir);
  float c = dot(O, O) - R * R;
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

  float t = -(b + sqrt(disc)) / 2;
  vec3 P = ray.origin + t * ray.dir;

  return HitInfo(
    true,
    t,
    P,
    normalize(P - transform.position),
    transform.color
  );
}

Ray trace(Ray ray) {
  for (uint i = 0; i < MAX_BOUNCES; ++i) {
    HitInfo hit = HitInfo(
      false,
      float(inf),
      vec3(0.0, 0.0, 0.0),
      vec3(0.0, 0.0, 0.0),
      vec3(0.0, 0.0, 0.0)
    );

    for (uint j = 0; j < ssbo.size; ++j) {
      HitInfo info = RaySphere(ssbo.transforms[j], ray);

      if (info.hit && info.t < hit.t) {
        hit = info;
      }
    }

    if (!hit.hit) {
      float a = abs(dot(ray.dir, vec3(0.0, -1.0, 0.0)));
      ray.color += (1 - a) * SKY_LIGHT + a * SKY_DARK;
      return ray;
    }

    ray.color += abs(dot(ray.dir, hit.normal)) * hit.color;
    ray.origin = hit.point;

    float alignment = dot(ray.dir, hit.normal);
    int invert = int(alignment / abs(alignment));

    ray.dir = invert * (ray.dir - 2 * alignment * hit.normal);
  }

  return ray;
}