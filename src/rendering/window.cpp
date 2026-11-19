#include "window.hpp"

#include <algorithm>
#include <stdexcept>

namespace {
GLFWmonitor *select_monitor(GLFWwindow *window) {
  int x, y, width, height;
  glfwGetWindowPos(window, &x, &y);
  glfwGetWindowSize(window, &width, &height);
  GLFWmonitor *selected = glfwGetPrimaryMonitor();
  long long largest_overlap = 0;
  int count = 0;
  GLFWmonitor **monitors = glfwGetMonitors(&count);
  // Vybereme monitor s největší částí okna hry.
  for (int i = 0; i < count; ++i) {
    const GLFWvidmode *mode = glfwGetVideoMode(monitors[i]);
    if (!mode) {
      continue;
    }
    int monitor_x, monitor_y;
    glfwGetMonitorPos(monitors[i], &monitor_x, &monitor_y);
    const int overlap_width = std::max(
        0, std::min(x + width, monitor_x + mode->width) - std::max(x, monitor_x));
    const int overlap_height = std::max(
        0, std::min(y + height, monitor_y + mode->height) - std::max(y, monitor_y));
    const long long overlap = static_cast<long long>(overlap_width) * overlap_height;
    if (overlap > largest_overlap) {
      selected = monitors[i];
      largest_overlap = overlap;
    }
  }
  return selected;
}
} // namespace

GlfwLibrary::GlfwLibrary() {
  if (!glfwInit()) {
    throw std::runtime_error("GLFW se nepodařilo inicializovat");
  }
}

GlfwLibrary::~GlfwLibrary() { glfwTerminate(); }

Window::Window(int width, int height, const char *title)
    : windowed_width(width), windowed_height(height) {
  // povolíme jen moderní OpenGL (3.3)
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  // Čtyři vzorky na pixel pro vyhlazení tenkých čar, stejně jako v prototypu.
  glfwWindowHint(GLFW_SAMPLES, 4);

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

void Window::toggle_fullscreen() {
  if (glfwGetWindowMonitor(handle)) {
    glfwSetWindowMonitor(handle, nullptr, windowed_x, windowed_y,
                         windowed_width, windowed_height, GLFW_DONT_CARE);
    if (windowed_maximized) {
      glfwMaximizeWindow(handle);
    }
    return;
  }

  GLFWmonitor *monitor = select_monitor(handle);
  if (!monitor) {
    return;
  }
  const GLFWvidmode *mode = glfwGetVideoMode(monitor);
  if (!mode) {
    return;
  }
  windowed_maximized = glfwGetWindowAttrib(handle, GLFW_MAXIMIZED) == GLFW_TRUE;
  if (windowed_maximized) {
    glfwRestoreWindow(handle);
  }
  // Uložíme rozměry běžného okna pro návrat z fullscreen režimu.
  glfwGetWindowPos(handle, &windowed_x, &windowed_y);
  glfwGetWindowSize(handle, &windowed_width, &windowed_height);
  glfwSetWindowMonitor(handle, monitor, 0, 0, mode->width, mode->height,
                       mode->refreshRate);
  if (windowed_maximized && !glfwGetWindowMonitor(handle)) {
    glfwMaximizeWindow(handle);
  }
}

Window::~Window() { glfwDestroyWindow(handle); }

GLFWwindow *Window::get() const { return handle; }

void Window::set_vsync(bool enabled) {
  // vertikální synchronizace (1 = zapnutá, 0 = vypnutá);
  glfwSwapInterval(enabled ? 1 : 0);
}
