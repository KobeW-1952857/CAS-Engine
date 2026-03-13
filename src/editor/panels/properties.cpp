#include "editor/panels/properties.h"

#include <ImGui/imgui.h>
#include <ImGui/misc/cpp/imgui_stdlib.h>

#include <glm/gtc/type_ptr.hpp>
#include <variant>

#include "core/asset_manager.h"
#include "core/uuid.h"
#include "editor/selection_context.h"
#include "scene/component_type_list.h"
#include "scene/components.h"
#include "scene/entity.h"

// --- Helpers --------------------------------------------------------------------------------------------------------

static std::string formatUniformName(const std::string& name) {
  // Strip leading 'u_' (snake_case) or 'u' (camelCase)
  std::string s = name;
  if (s.size() > 2 && s[0] == 'u' && s[1] == '_')
    s = s.substr(2);
  else if (s.size() > 1 && s[0] == 'u' && std::isupper(s[1]))
    s = s.substr(1);

  // Convert snake_case and camelCase to "Title Case With Spaces"
  std::string result;
  bool capitalize_next = true;
  for (char c : s) {
    if (c == '_') {
      result += ' ';
      capitalize_next = true;
    } else if (std::isupper(c) && !result.empty() && result.back() != ' ') {
      result += ' ';
      result += c;
      capitalize_next = false;
    } else if (capitalize_next) {
      result += static_cast<char>(std::toupper(c));
      capitalize_next = false;
    } else {
      result += c;
    }
  }
  return result;
}
template <typename T, typename UIFunction>
static void DrawComponent(Entity entity, UIFunction ui_function) {
  if (!entity.hasComponent<T>()) return;

  auto& component = entity.getComponent<T>();

  ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2{4, 4});
  ImGui::Separator();
  ImGui::PushID(typeid(T).hash_code());
  bool open = ImGui::TreeNodeEx("##Component", ImGuiTreeNodeFlags_DefaultOpen, "%s", T::name.c_str());
  ImGui::PopID();
  ImGui::PopStyleVar();

  if (open) {
    ui_function(component);
    ImGui::TreePop();
  }
}

// --- drawMaterial ---------------------------------------------------------------------------------------------------
void PropertiesPanel::drawMaterial(const std::shared_ptr<Material>& material, AssetMetadata& meta_data) {
  auto shaders = m_context.assets.getAssetsMetadataOfType<Shader>();
  auto& shader = material->shader;
  auto& current_meta = m_context.assets.getAssetMetadata(shader->handle);

  // Rename
  {
    std::string material_name = meta_data.filepath.stem().string();
    if (ImGui::InputText("##material_name", &material_name)) {
      std::filesystem::path new_path = meta_data.filepath.parent_path() / (material_name + ".casmat");
      std::filesystem::rename(meta_data.filepath, new_path);
      meta_data.filepath = new_path;
    }
  }

  ImGui::Text("Shader");
  ImGui::SameLine();
  if (ImGui::BeginCombo("##shader", current_meta.filepath.stem().c_str())) {
    for (const auto& [id, meta] : shaders) {
      bool selected = id == shader->handle;
      auto name = meta.filepath.filename().c_str();

      if (ImGui::Selectable(name, selected)) {
        material->shader = m_context.assets.getAsset<Shader>(id);
        material->resetProperties();
      }
    }
    ImGui::EndCombo();
  }

  if (!shader) return;

  auto uniforms = shader->getActiveUniforms();
  for (auto uniform : uniforms) {
    ImGui::Text("%s", formatUniformName(uniform.name).c_str());
    ImGui::SameLine();

    auto label = std::format("##{}", uniform.name);

    switch (uniform.type) {
      case GL_INT: {
        auto val = material->getPropertyOrDefault(uniform.name, 0);
        if (ImGui::DragInt(label.c_str(), &val)) material->setProperty(uniform.name, val);
        break;
      }
      case GL_FLOAT: {
        auto val = material->getPropertyOrDefault(uniform.name, 0.0f);
        if (ImGui::DragFloat(label.c_str(), &val)) material->setProperty(uniform.name, val);
        break;
      }
      case GL_FLOAT_VEC2: {
        auto val = material->getPropertyOrDefault(uniform.name, glm::vec2(0.0f));
        if (ImGui::DragFloat2(label.c_str(), glm::value_ptr(val))) material->setProperty(uniform.name, val);
        break;
      }
      case GL_FLOAT_VEC3: {
        auto val = material->getPropertyOrDefault(uniform.name, glm::vec3(0.0f));
        bool is_color = uniform.name.find("olor") != std::string::npos;
        if (is_color) {
          if (ImGui::ColorEdit3(label.c_str(), glm::value_ptr(val))) material->setProperty(uniform.name, val);
        } else {
          if (ImGui::DragFloat3(label.c_str(), glm::value_ptr(val))) material->setProperty(uniform.name, val);
        }
        break;
      }
      case GL_FLOAT_VEC4: {
        auto val = material->getPropertyOrDefault(uniform.name, glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));
        bool is_color = uniform.name.find("olor") != std::string::npos;
        if (is_color) {
          if (ImGui::ColorEdit4(label.c_str(), glm::value_ptr(val))) material->setProperty(uniform.name, val);
        } else {
          if (ImGui::DragFloat4(label.c_str(), glm::value_ptr(val))) material->setProperty(uniform.name, val);
        }
        break;
      }
    }
  }
}

// --- drawComponents -------------------------------------------------------------------------------------------------

template <typename T>
void PropertiesPanel::addComponentToEntity(Entity entity, Scene* scene) {
  if constexpr (requires(Scene& s) { T::create(s); })
    entity.addComponent<T>(T::create(scene));
  else
    entity.addComponent<T>();
  ;
}

void PropertiesPanel::drawComponents(Entity entity, Scene* scene) {
  if (!entity || !scene) return;
  auto& assets = m_context.assets;

  if (entity.hasComponent<TagComponent>()) {
    auto& tag = entity.getComponent<TagComponent>().tag;
    if (ImGui::InputText("##Tag", &tag)) {
      entity.getComponent<TagComponent>().tag = tag;
    }
  }
  ImGui::SameLine();
  ImGui::PushItemWidth(-1);
  if (ImGui::Button("+")) ImGui::OpenPopup("Add component");

  if (ImGui::BeginPopup("Add component")) {
    forEachComponentType([&]<typename T>() {
      if constexpr (T::user_addable)
        if (!entity.hasComponent<T>())
          if (ImGui::MenuItem(T::name.data())) addComponentToEntity<T>(entity, scene);
    });
    ImGui::EndPopup();
  }
  ImGui::PopItemWidth();

  forEachComponentType([&]<typename T>() {
    if constexpr (T::user_editable)
      if (auto* c = entity.tryGetComponent<T>()) {
        if (ImGui::CollapsingHeader(T::name.data(), ImGuiTreeNodeFlags_DefaultOpen)) T::drawUI(*c, m_context);
      }
  });
}

// --- ImGui -----------------------------------------------------------------------------------------------------------
void PropertiesPanel::onImGuiRender(SelectionContext& selection_context, Scene* active_scene) {
  ImGui::Begin("Properties");
  if (std::holds_alternative<Entity>(selection_context)) {
    drawComponents(std::get<Entity>(selection_context), active_scene);
  } else if (std::holds_alternative<UUID>(selection_context)) {
    auto id = std::get<UUID>(selection_context);
    auto& meta = m_context.assets.getAssetMetadata(id);

    switch (meta.type) {
      case AssetType::Material: {
        auto asset = m_context.assets.getAsset<Material>(id);
        drawMaterial(asset, meta);
        break;
      }
      default:
        break;
    }

  } else {
    ImGui::Text("Nothing selected");
  }
  ImGui::End();
}

template <typename T>
void PropertiesPanel::displayAddComponent(Entity entity) {
  if (entity.hasComponent<T>()) return;

  if (ImGui::MenuItem(T::name.c_str())) {
    entity.addComponent<T>();
    ImGui::CloseCurrentPopup();
  }
}
