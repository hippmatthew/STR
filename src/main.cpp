#include "src/include/engine.hpp"

int main()
{
  VECS_SETTINGS.add_device_extension(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);
  
  str::Engine engine;

  engine.load();
  engine.run();

  
  // la::vec<3> eye = { 0.0, 0.0, -2.0 };
  // la::vec<3> center = { 0.0, 0.0, 0.0 };
  // la::vec<3> up = { 0.0, -1.0, 0.0 };

  // la::vec<3> forward = (center - eye).normalized();
  // la::vec<3> right = (forward % up).normalized();
  // la::vec<3> new_up = right % forward;

  // la::mat<4> view{
  //   la::vec<4>(right, { -right * eye }),
  //   la::vec<4>(new_up, { -new_up * eye }),
  //   la::vec<4>(-forward, { forward * eye }),
  //   la::vec<4>{ 0.0, 0.0, 0.0, 1.0 }
  // };

  // std::cout << "u = " << std::to_string(forward) << '\n';
  // std::cout << "v = " << std::to_string(right) << '\n';
  // std::cout << "w = " << std::to_string(new_up) << '\n';

  // std::cout << "view:\n";
  // std::cout << std::to_string(view) << '\n';

  // std::cout << "class view:\n";
  // std::cout << std::to_string(la::mat<4>::view_matrix(eye, center, up));
  
  // float ar = VECS_SETTINGS.width() / VECS_SETTINGS.height();
  
  // std::cout << "\nperspective:\n";
  // std::cout << std::to_string(la::mat<4>::perspective_projection(la::radians(45.0f), ar, 0.01f, 10.0f));
}