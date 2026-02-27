#include "renderer/material.h"

#include <variant>

#include "Nexus/Log.h"

Material::Material(std::shared_ptr<Shader> shader) : shader(std::move(shader)) { type = AssetType::Material; }

void Material::setTexture(const std::string& name, const std::shared_ptr<Texture>& texture) {
  m_textures[name] = texture;
}

std::shared_ptr<Texture> Material::getTexture(const std::string& name) const { return m_textures.at(name); }

void Material::bind() const {
  if (!shader) {
    Nexus::Logger::error("No shader to bind");
    return;
  }
  shader->use();

  for (const auto& [name, value] : m_properties) {
    std::visit(
        [&](auto&& v) {
          using T = std::decay_t<decltype(v)>;
          if constexpr (std::is_same_v<T, int>) {
            shader->setInt(name, v);
          } else if constexpr (std::is_same_v<T, float>) {
            shader->setFloat(name, v);
          } else if constexpr (std::is_same_v<T, glm::vec2>) {
            // m_shader->set;
          } else if constexpr (std::is_same_v<T, glm::vec3>) {
            shader->setVec3(name, v);
          } else if constexpr (std::is_same_v<T, glm::vec4>) {
            shader->setVec4(name, v);
          } else if constexpr (std::is_same_v<T, glm::mat4>) {
            shader->setMat4(name, v);
          }
        },
        value);
  }

  int texture_slot = 0;
  for (const auto& [name, texture] : m_textures) {
    texture->bind(texture_slot);
    shader->setInt(name, texture_slot);
    texture_slot++;
  }
}