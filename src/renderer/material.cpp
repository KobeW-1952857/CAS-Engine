#include "renderer/material.h"

#include <yaml-cpp/yaml.h>

#include <fstream>
#include <variant>

#include "Nexus/Log.h"
#include "core/asset_manager.h"
#include "renderer/shader.h"
#include "utils/yaml_extension.h"

Material::Material() {
  type = AssetType::Material;
  shader = AssetManager::getDefaultAsset<Shader>();
}

Material::Material(std::shared_ptr<Shader> shader) : shader(std::move(shader)) { type = AssetType::Material; }

std::shared_ptr<Material> Material::load(const std::string& filepath) {
  Nexus::Logger::trace("Loading material from {}...", filepath);
  YAML::Node data;
  try {
    data = YAML::LoadFile(filepath);
  } catch (const YAML::Exception& e) {
    Nexus::Logger::error("Failed to load material file '{}'. Error: {}", filepath, e.what());
    return nullptr;
  }

  auto matNode = data["Material"];
  if (!matNode) {
    Nexus::Logger::error("Invalid material file '{}': Missing 'Material' node.", filepath);
    return nullptr;
  }
  auto shader_node = matNode["ShaderHandle"];
  if (!shader_node) {
    Nexus::Logger::error("Invalid material file '{}': Missing 'Shader' node.", filepath);
    return nullptr;
  }

  auto material = std::make_shared<Material>();
  material->shader = AssetManager::getAsset<Shader>(shader_node.as<UUID>());

  auto properties = matNode["Properties"];
  if (!properties) return material;

  for (const auto& prop : properties) {
    std::string name = prop["Name"].as<std::string>();
    std::string typeStr = prop["Type"].as<std::string>();

    if (typeStr == "int") {
      material->setProperty(name, prop["Value"].as<int>());
    } else if (typeStr == "float") {
      material->setProperty(name, prop["Value"].as<float>());
    } else if (typeStr == "vec2") {
      material->setProperty(name, prop["Value"].as<glm::vec2>());
    } else if (typeStr == "vec3") {
      material->setProperty(name, prop["Value"].as<glm::vec3>());
    } else if (typeStr == "vec4") {
      material->setProperty(name, prop["Value"].as<glm::vec4>());
    } else if (typeStr == "mat4") {
      material->setProperty(name, prop["Value"].as<glm::mat4>());
    } else {
      Nexus::Logger::warn("Unsupported material property type: {}", typeStr);
    }
  }
  material->modified = false;
  return material;
}

void Material::serialize(const std::filesystem::path& filepath) const {
  YAML::Emitter out;
  out << YAML::BeginMap;
  out << YAML::Key << "Material";
  out << YAML::BeginMap;

  out << YAML::Key << "ShaderHandle" << YAML::Value << shader->handle;

  out << YAML::Key << "Properties" << YAML::Value << YAML::BeginSeq;
  for (const auto& [name, prop] : m_properties) {
    out << YAML::BeginMap;
    out << YAML::Key << "Name" << YAML::Value << name;

    std::visit(
        [&](auto&& v) {
          using T = std::decay_t<decltype(v)>;
          if constexpr (std::is_same_v<T, int>) {
            out << YAML::Key << "Type" << YAML::Value << "int";
            out << YAML::Key << "Value" << YAML::Value << v;
          } else if constexpr (std::is_same_v<T, glm::vec2>) {
            out << YAML::Key << "Type" << YAML::Value << "vec2";
            out << YAML::Key << "Value" << YAML::Value << v;
          } else if constexpr (std::is_same_v<T, float>) {
            out << YAML::Key << "Type" << YAML::Value << "float";
            out << YAML::Key << "Value" << YAML::Value << v;
          } else if constexpr (std::is_same_v<T, glm::vec3>) {
            out << YAML::Key << "Type" << YAML::Value << "vec3";
            out << YAML::Key << "Value" << YAML::Value << v;
          } else if constexpr (std::is_same_v<T, glm::vec4>) {
            out << YAML::Key << "Type" << YAML::Value << "vec4";
            out << YAML::Key << "Value" << YAML::Value << v;
          } else if constexpr (std::is_same_v<T, glm::mat4>) {
            out << YAML::Key << "Type" << YAML::Value << "mat4";
            out << YAML::Key << "Value" << YAML::Value << v;
          } else {
            Nexus::Logger::warn("Attempting to serialize unsupported material property type.");
          }
        },
        prop);

    out << YAML::EndMap;
  }
  out << YAML::EndSeq;

  out << YAML::EndMap;  // Material Map
  out << YAML::EndMap;  // Root Map

  std::ofstream fout(filepath);
  fout << out.c_str();
}

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

void Material::resetProperties() {
  m_properties.clear();
  m_textures.clear();
  modified = true;
}