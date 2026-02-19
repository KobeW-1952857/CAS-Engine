#include <imgui.h>

#include "Nexus.h"
#include "Nexus/Log.h"
#include "Nexus/Window/GLFWWindow.h"
#include "camera.h"
#include "planet.h"
#include "resource_manager.h"
#include "utils.h"
#define GLM_ENABLE_EXPERIMENTAL
#include "glm/gtx/string_cast.hpp"

void moveCamera(Camera& camera, GLFWwindow* window) {
  float speed = 0.05f;

  if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) camera.move({0, 0, speed});   // Forward (Z)
  if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) camera.move({0, 0, -speed});  // Backward
  if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) camera.move({-speed, 0, 0});  // Left (X)
  if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) camera.move({speed, 0, 0});   // Right
  if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) camera.move({0, speed, 0});   // Up (Y)
  if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) camera.move({0, -speed, 0});  // Down
}

int main() {
  Nexus::Window::s_api = Nexus::Window::API::GLFW;
  auto window = Nexus::Window::create();
  int width, height;
  glfwGetFramebufferSize((GLFWwindow*)window->getNativeWindow(), &width, &height);

  Nexus::Logger::setLevel(Nexus::LogLevel::Trace);

  window->onResize([&width, &height, &window](int new_width, int new_height) -> bool {
    glfwGetFramebufferSize((GLFWwindow*)window->getNativeWindow(), &width, &height);

    glViewport(0, 0, width, height);
    return false;
  });

  window->setVSync(true);

  Camera camera({0, 0, 3}, 90.0f);
  Shader& shader = ResourceManager::LoadShader("basic", "assets/shaders/basic.vert", "assets/shaders/basic.frag");

  window->onMouseMove([&camera, width, height, &window](double xpos, double ypos) -> bool {
    static bool firstMouse = true;
    static glm::vec2 last_pos{width / 2.0f, height / 2.0f};
    static float sensitivity = 0.1f;

    if (glfwGetMouseButton((GLFWwindow*)window->getNativeWindow(), GLFW_MOUSE_BUTTON_RIGHT) != GLFW_PRESS) {
      firstMouse = true;
      return false;
    }

    if (firstMouse) {
      last_pos = {xpos, ypos};
      firstMouse = false;
    }

    glm::vec2 delta = {xpos - last_pos.x, last_pos.y - ypos};
    last_pos = {xpos, ypos};

    camera.rotate(delta * sensitivity);

    return true;
  });

  window->onMouseButton([&](int button, int action, int mods) -> bool {
    if (button == GLFW_MOUSE_BUTTON_RIGHT) {
      if (action == GLFW_PRESS) {
        glfwSetInputMode((GLFWwindow*)window->getNativeWindow(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
      } else if (action == GLFW_RELEASE) {
        glfwSetInputMode((GLFWwindow*)window->getNativeWindow(), GLFW_CURSOR, GLFW_CURSOR_NORMAL);
      }
      return true;
    }
    return false;
  });

  window->onKey([&shader, &window](int key, int scancode, int action, int mods) -> bool {
    if (action == GLFW_PRESS) {
      switch (key) {
        case GLFW_KEY_R:
          shader.compile();
          return true;
        case GLFW_KEY_ESCAPE:
          window->shouldClose = true;
          return true;
      }
    }
    return false;
  });

  glEnable(GL_DEPTH_TEST);
  glDisable(GL_CULL_FACE);
  glViewport(0, 0, width, height);

  Planet sun("Sun", Color{1.0f, 1.0f, 1.0f, 1.0f});
  sun.addTexture("assets/textures/sun.jpg");

  Planet earth("Earth", Color{0.0f, 1.0f, 0.0f, 1.0f}, &sun, 3.0f);
  earth.addTexture("assets/textures/earth.jpg");

  float dt = 0.0f;
  float lastFrame = 0.0f;
  float simulation_speed = 1.0f;

  window->whileOpen([&]() {
    float currentFrame = glfwGetTime();
    dt = currentFrame - lastFrame;
    lastFrame = currentFrame;

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    ImGui::Begin("Settings");
    ImGui::Text("Simulation speed");
    ImGui::SameLine();
    ImGui::SliderFloat("##simulation_speed", &simulation_speed, 0.0f, 1000.0f);
    ImGui::End();

    ImGui::Begin("Planets");
    sun.drawUI();
    earth.drawUI();
    ImGui::End();

    sun.update(dt, simulation_speed);
    earth.update(dt, simulation_speed);

    sun.render(shader, camera.getView(), camera.getProjection(width, height));
    earth.render(shader, camera.getView(), camera.getProjection(width, height));

    moveCamera(camera, (GLFWwindow*)window->getNativeWindow());
  });
}
