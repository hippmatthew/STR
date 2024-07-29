#include "src/include/engine.hpp"

#include <iostream>
#include <vector>

struct Plane
{
  std::vector<la::vec<2>> vertices;
  std::vector<unsigned int> indices;
};

Plane plane(unsigned int, unsigned int);
void printVertices(const std::vector<la::vec<2>>&, unsigned int);
void printIndices(const std::vector<unsigned int>&);

int main()
{
  std::cout << std::to_string(la::mat<4>::view_matrix({0.0, 0.0, 0.0}, {1.0, 0.0, 0.0}, {0.0, -1.0, 0.0})) << '\n';

  VECS_SETTINGS.add_device_extension(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);

  str::Engine engine;

  engine.load();
  engine.run();
}

Plane plane(unsigned int width, unsigned int height)
{
  std::vector<la::vec<2>> vertices;
  std::vector<unsigned int> indices;

  unsigned int rows = height;
  unsigned int cols = width;
  float dx = 2.0 / cols;
  float dy = 2.0 / rows;

  for (unsigned long i = 0; i <= rows; ++i)
  {
    for (unsigned long j = 0; j <= cols; ++j)
    {
      vertices.emplace_back(la::vec<2>{ j * dx - 1, i * dy - 1 });

      if (i == rows || j == cols) continue;

      unsigned int topL = i * (cols + 1) + j;
      unsigned int topR = topL + 1;
      unsigned int botL = (i + 1) * (cols + 1) + j;
      unsigned int botR = botL + 1;

      indices.insert(indices.end(), { topL, botL, topR, topR, botL, botR });
    }
  }

  return Plane{ vertices, indices };
}

void printVertices(const std::vector<la::vec<2>>& vertices, unsigned int width)
{
  std::cout << "vertices (" << vertices.size() << ")\n";

  for (unsigned int i = 0, j = 0; i < vertices.size(); ++i, j = (++j % (width + 1)))
    std::cout << i << ( j == width ? "\n" : "\t");
}

void printIndices(const std::vector<unsigned int>& indices)
{
  std::cout << "indices (" << indices.size() << ")\n";

  for (unsigned int i = 0, j = 0; i < indices.size(); ++i, j = ++j % 6)
  {
    std::cout << indices[i] << (j == 5 ? '\n' : '\t');
  }
}