#include "planet.h"

#include <imgui.h>

#include <format>
#include <glm/gtc/type_ptr.hpp>

Planet::Planet(std::string name, Color color, Planet* primary, float distance, float orbit, Point position,
               glm::quat rotation, float scale)
    : m_mesh("assets/models/planet.obj"),
      m_name(name),
      m_primary(primary),
      m_orbit(orbit),
      m_distance(distance),
      m_color(color),
      m_position(position),
      m_rotation(rotation),
      m_scale(scale) {
  calculatePosition();
}

void Planet::addTexture(const std::string& path) {
  m_texture.load(path);
  m_hasTexture = true;
}

void Planet::update(float dt, float simulation_speed) {
  m_orbit = m_orbit + dt * simulation_speed;
  if (m_orbit > 360.0f) m_orbit = 0.0f;
}

void Planet::render(Shader& shader, const glm::mat4& view, const glm::mat4& projection) {
  glm::mat4 model = glm::mat4(1.0f);
  auto translate = glm::translate(model, calculatePosition());
  auto rotate = glm::mat4_cast(m_rotation);
  auto scale = glm::scale(model, glm::vec3(m_scale));
  model = translate * rotate * scale;

  shader.use();

  if (!m_primary) {
    shader.setVec3("lightPos", m_position);
    shader.setVec3("lightColor", glm::vec3(m_color));
    shader.setFloat("intensity", m_intensity);
    shader.setBool("isEmitter", true);
  } else {
    shader.setBool("isEmitter", false);
  }

  if (m_hasTexture) {
    m_texture.bind(0);
    shader.setBool("hasTexture", true);
    shader.setInt("ourTexture", 0);
  } else {
    shader.setBool("hasTexture", false);
  }

  shader.setVec4("ourColor", m_color);
  shader.setMVP(model, view, projection);

  m_mesh.render(shader, model, view, projection);
}

void Planet::drawUI() {
  if (ImGui::CollapsingHeader(m_name.c_str())) {
    if (!m_primary) {
      ImGui::Text("Position");
      ImGui::SameLine();
      ImGui::DragFloat3(std::format("##{}_position", m_name).c_str(), glm::value_ptr(m_position));

      ImGui::Text("Intensity");
      ImGui::SameLine();
      ImGui::DragFloat(std::format("##{}_intensity", m_name).c_str(), &m_intensity, 0.1f, 0.1f);
    } else {
      ImGui::Text("Orbit");
      ImGui::SameLine();
      ImGui::Text("%.2f", m_orbit);

      ImGui::Text("Distance");
      ImGui::SameLine();
      ImGui::DragFloat(std::format("##{}_distance", m_name).c_str(), &m_distance, 0.1f, 0.0f);
    }

    ImGui::Text("Scale");
    ImGui::SameLine();
    ImGui::SliderFloat(std::format("##{}_scale", m_name).c_str(), &m_scale, 0.1f, 10.0f);

    ImGui::Text("Color");
    ImGui::SameLine();
    ImGui::ColorEdit4(std::format("##{}_color", m_name).c_str(), glm::value_ptr(m_color));
  }
}

Point Planet::calculatePosition() {
  if (!m_primary) return m_position;

  m_position = m_primary->m_position +
               Point(cos(glm::radians(m_orbit)) * m_distance, 0, sin(glm::radians(m_orbit)) * m_distance);
  return m_position;
}