#pragma once

#include <glm/glm.hpp>
#include <string>
#include <unordered_map>
#include <variant>

#include "shader.h"
#include "texture.h"

using MaterialProperty = std::variant<int, float, glm::vec2, glm::vec3, glm::vec4, glm::mat4>;

class Material : public Asset {
 public:
  Material() { type = AssetType::Material; };
  Material(std::shared_ptr<Shader> shader);

  template <typename T>
  void setProperty(const std::string& name, const T& value) {
    m_properties[name] = value;
  }

  template <typename T>
  T getProperty(const std::string& name) const {
    return std::get<T>(m_properties.at(name));
  }

  template <typename T>
  T getPropertyOrDefault(const std::string& name, const T& defaultValue) {
    if (m_properties.find(name) != m_properties.end()) {
      return std::get<T>(m_properties.at(name));
    } else {
      return defaultValue;
    }
  }

  template <typename T>
  bool hasProperty(const std::string& name) const {
    return m_properties.find(name) != m_properties.end();
  }
  void setTexture(const std::string& name, const std::shared_ptr<Texture>& texture);

  std::shared_ptr<Texture> getTexture(const std::string& name) const;

  void bind() const;

 public:
  std::shared_ptr<Shader> shader;

 private:
  std::unordered_map<std::string, MaterialProperty> m_properties;
  std::unordered_map<std::string, std::shared_ptr<Texture>> m_textures;
};