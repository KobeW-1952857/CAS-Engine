#include <imgui.h>

#include "Nexus.h"
#include "Nexus/Log.h"
#include "Nexus/Window/GLFWWindow.h"
#include "bezier_curve.h"
#include "camera.h"
#include "framebuffer.h"
#include "planet.h"
#include "resource_manager.h"
#include "tesselation_shader.h"
#include "utils.h"

void update(float dt, float simulation_speed, Planet& sun, Planet& earth) {
  sun.update(dt, simulation_speed);
  earth.update(dt, simulation_speed);
}

void renderUI(float& simulation_speed, Planet& sun, Planet& earth, BezierCurve& curve) {
  ImGui::Begin("Settings");
  ImGui::Text("Simulation speed");
  ImGui::SameLine();
  ImGui::SliderFloat("##simulation_speed", &simulation_speed, 0.0f, 1000.0f);
  ImGui::End();

  ImGui::Begin("Planets");
  sun.drawUI();
  earth.drawUI();
  curve.drawUI();
  ImGui::End();
}

int main() {
  Nexus::Logger::setLevel(Nexus::LogLevel::Trace);
  Nexus::Window::s_api = Nexus::Window::API::GLFW;
  auto window = Nexus::Window::create();

  int width, height;
  glfwGetFramebufferSize((GLFWwindow*)window->getNativeWindow(), &width, &height);

  window->setVSync(true);
  window->onResize([&width, &height, &window](int new_width, int new_height) -> bool {
    glfwGetFramebufferSize((GLFWwindow*)window->getNativeWindow(), &width, &height);
    Nexus::Logger::debug("Resized to {}x{}", width, height);

    glViewport(0, 0, width, height);
    return false;
  });

  Camera camera({0, 0, 3}, 90.0f);
  camera.addControls(*window);

  Framebuffer framebuffer(width, height);

  Shader& shader = ResourceManager::LoadShader("basic", "assets/shaders/basic.vert", "assets/shaders/basic.frag");
  Shader& line = ResourceManager::LoadShader("line", "assets/shaders/line.vert", "assets/shaders/line.frag");
  TesselationShader tessShader("tess", "assets/shaders/tess.vert", "assets/shaders/tess.frag",
                               "assets/shaders/tess.tcs", "assets/shaders/tess.tes");

  Planet sun("Sun", Color{1.0f, 1.0f, 1.0f, 1.0f});
  sun.addTexture("assets/textures/sun.jpg");
  Planet earth("Earth", Color{0.0f, 1.0f, 0.0f, 1.0f}, &sun, 3.0f);
  earth.addTexture("assets/textures/earth.jpg");
  BezierCurve curve({{-0.8f, -0.5f, 0.0f}, {-0.4f, 0.8f, 0.0f}, {0.4f, -0.8f, 0.0f}, {0.8f, 0.5f, 0.0f}});

  float dt = 0.0f;
  float lastFrame = 0.0f;
  float simulation_speed = 1.0f;

  glEnable(GL_DEPTH_TEST);
  glDisable(GL_CULL_FACE);
  glViewport(0, 0, width, height);

  window->whileOpen([&]() {
    checkOpenGLError("Start render");
    ImGui::DockSpaceOverViewport(0, ImGui::GetMainViewport());

    float currentFrame = glfwGetTime();
    dt = currentFrame - lastFrame;
    lastFrame = currentFrame;

    camera.processInput(*window);
    update(dt, simulation_speed, sun, earth);

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    ImGui::Begin("framebuffer");
    {
      auto size = ImGui::GetContentRegionAvail();
      framebuffer.bind();
      framebuffer.resize(size.x, size.y);
      framebuffer.clear({0.0f, 0.0f, 0.0f, 1.0f});

      curve.render(tessShader);
      curve.renderControlPoints(line);

      sun.render(shader, camera.getView(), camera.getProjection(size.x, size.y));
      earth.render(shader, camera.getView(), camera.getProjection(size.x, size.y));

      framebuffer.unbind();
      ImGui::Image((void*)(intptr_t)framebuffer.getTexture(), size, {0, 1}, {1, 0});
    }
    ImGui::End();

    renderUI(simulation_speed, sun, earth, curve);
  });
}
