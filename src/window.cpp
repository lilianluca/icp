#include "window.hpp"

#include <stdexcept>

GlfwLibrary::GlfwLibrary() {
  if (!glfwInit()) {
    throw std::runtime_error("GLFW se nepodařilo inicializovat");
  }
}

GlfwLibrary::~GlfwLibrary() { glfwTerminate(); }

Window::Window(int width, int height, const char *title) {
  handle = glfwCreateWindow(width, height, title, nullptr, nullptr);
  if (!handle) {
    throw std::runtime_error("Nepodařilo se vytvořit okno");
  }
}

Window::~Window() { glfwDestroyWindow(handle); }

GLFWwindow *Window::get() const { return handle; }