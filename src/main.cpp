#include <imgui.h>

#include <memory>

#include "Nexus.h"
#include "Nexus/Log.h"
#include "Nexus/Window/GLFWWindow.h"
#include "core/asset_manager.h"
#include "core/resource_manager.h"
#include "editor/editor.h"
#include "renderer/camera.h"
#include "renderer/material.h"
#include "scene/components.h"
#include "scene/scene.h"
#include "utils/utils.h"

int main() {
  Nexus::Logger::setLevel(Nexus::LogLevel::Trace);
  Nexus::Window::s_api = Nexus::Window::API::GLFW;
  auto window = Nexus::Window::create(Nexus::WindowProps("CAS Engine", 1280, 720));

  // glfwMaximizeWindow((GLFWwindow*)window->getNativeWindow());

  int width, height;
  glfwGetFramebufferSize((GLFWwindow*)window->getNativeWindow(), &width, &height);

  glEnable(GL_DEPTH_TEST);
  glDisable(GL_CULL_FACE);
  glViewport(0, 0, width, height);

  window->setVSync(true);
  window->onResize([&width, &height, &window](int new_width, int new_height) -> bool {
    glfwGetFramebufferSize((GLFWwindow*)window->getNativeWindow(), &width, &height);

    glViewport(0, 0, width, height);
    return false;
  });

  auto shader = ResourceManager::LoadShader("default", "assets/shaders/default.vert", "assets/shaders/default.frag");
  AssetManager::addMemoryOnlyAsset(shader, "default");
  auto basic_shader = ResourceManager::LoadShader("basic", "assets/shaders/basic.vert", "assets/shaders/basic.frag");
  AssetManager::addMemoryOnlyAsset(basic_shader, "basic");

  auto material = std::make_shared<Material>(shader);
  AssetManager::addMemoryOnlyAsset(material, "default");

  auto sphere = std::make_shared<Mesh>("assets/models/sphere.obj");
  AssetManager::addMemoryOnlyAsset(sphere, "sphere");

  auto cube = std::make_shared<Mesh>("assets/models/cube.obj");
  AssetManager::addMemoryOnlyAsset(cube, "cube");

  float dt = 0.0f;
  float lastFrame = 0.0f;
  float simulation_speed = 1.0f;

  Editor editor;
  editor.init();

  auto scene = std::make_shared<Scene>();
  editor.setContext(scene);

  auto entity = scene->createEntity("Test");
  auto& mat = entity.addComponent<MaterialComponent>();
  mat.material_handle = material->handle;
  auto& mc = entity.addComponent<MeshComponent>();
  mc.mesh_handle = sphere->handle;

  int framecount = 0;

  window->whileOpen([&]() {
    checkOpenGLError(std::format("Start render frame {}", framecount++));
    float currentFrame = glfwGetTime();
    dt = currentFrame - lastFrame;
    lastFrame = currentFrame;

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    editor.onUpdate(dt);
    checkOpenGLError(std::format("Start editor update frame {}", framecount));

    editor.onImGuiRender();
    checkOpenGLError(std::format("Start editor imgui frame {}", framecount));
    // int winWidth, winHeight;
    // glfwGetFramebufferSize((GLFWwindow*)window->getNativeWindow(), &winWidth, &winHeight);
    // glViewport(0, 0, winWidth, winHeight);
    // glBindFramebuffer(GL_FRAMEBUFFER, 0);
  });
}
