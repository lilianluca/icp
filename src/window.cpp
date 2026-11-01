#include "window.hpp"

#include <stdexcept>

GlfwLibrary::GlfwLibrary() {
  if (!glfwInit()) {
    throw std::runtime_error("GLFW se nepodařilo inicializovat");
  }
}

GlfwLibrary::~GlfwLibrary() { glfwTerminate(); }

Window::Window(int width, int height, const char *title) {
  // povolíme jen moderní OpenGL (3.3)
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  handle = glfwCreateWindow(width, height, title, nullptr, nullptr);
  if (!handle) {
    throw std::runtime_error("Nepodařilo se vytvořit okno");
  }

  glfwMakeContextCurrent(handle);

  // inicializace GLEW (až teď, protože potřebuje kontext)
  if (glewInit() != GLEW_OK) {
    glfwDestroyWindow(handle);
    throw std::runtime_error("GLEW se nepodařilo inicializovat");
  }
}

Window::~Window() { glfwDestroyWindow(handle); }

GLFWwindow *Window::get() const { return handle; }

void Window::set_vsync(bool enabled) {
  // vertikální synchronizace (1 = zapnutá, 0 = vypnutá);
  glfwSwapInterval(enabled ? 1 : 0);
}