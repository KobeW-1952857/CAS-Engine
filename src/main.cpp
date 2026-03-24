#include <imgui.h>

#include <memory>
#include <vector>

#include "Nexus.h"
#include "Nexus/Log.h"
#include "Nexus/Window/GLFWWindow.h"
#include "editor/editor.h"
#include "renderer/camera.h"

int main(int argc, char** argv) {
  Nexus::Logger::setLevel(Nexus::LogLevel::Trace);
  Nexus::Window::s_api = Nexus::Window::API::GLFW;

  auto window = Nexus::Window::create(Nexus::WindowProps("CAS Engine", 1280, 720));
  ImGui::GetIO().ConfigFlags &= ~ImGuiConfigFlags_ViewportsEnable;
  window->setVSync(true);

  int width, height;
  glfwGetFramebufferSize(static_cast<GLFWwindow*>(window->getNativeWindow()), &width, &height);

  glEnable(GL_DEPTH_TEST);
  glDisable(GL_CULL_FACE);
  glViewport(0, 0, width, height);

  window->onResize([&width, &height, &window](int new_width, int new_height) -> bool {
    glfwGetFramebufferSize(static_cast<GLFWwindow*>(window->getNativeWindow()), &width, &height);

    glViewport(0, 0, width, height);
    return false;
  });

  float dt = 0.0f;
  float lastFrame = 0.0f;
  std::vector<std::string> arguments(argv + 1, argv + argc);

  Editor editor(arguments);

  int framecount = 0;

  window->whileOpen([&]() {
    float currentFrame = glfwGetTime();
    dt = currentFrame - lastFrame;
    lastFrame = currentFrame;

    ImGui::ShowDemoWindow();

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    editor.onUpdate(dt);

    editor.onImGuiRender();
  });
}
