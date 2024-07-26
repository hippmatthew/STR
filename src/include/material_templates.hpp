#ifndef str_material_templates_hpp
#define str_material_templates_hpp

#include "src/include/material.hpp"

namespace str
{

template <typename... Tps>
Material::MaterialBuilder& Material::MaterialBuilder::uniforms()
{
  ( uniform<Tps>(), ... );
  return *this;
}

template <typename T>
void Material::MaterialBuilder::uniform()
{
  UniformInfo info{
    .index  = uniformInfo.size(),
    .size = sizeof(T)
  };

  indexMap.emplace(std::make_pair(typeid(T).name(), info.index));
  uniformInfo.emplace_back(info);
}

template <typename... Tps>
void Material::updateUniforms(Tps&... args)
{
  ( updateUniform<Tps>(args), ... );
}

template <typename T>
void Material::updateUniform(T& data)
{
  if (indexMap.find(typeid(T).name()) == indexMap.end()) return;
  
  auto info = uniformInfo[indexMap.at(typeid(T).name())];

  void * memory = vk_memory.mapMemory(info.offset, info.size);
  memcpy(memory, &data, sizeof(data));
  vk_memory.unmapMemory();
}

} // namespace str

#endif // str_material_templates_hpp