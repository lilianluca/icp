#pragma once

#include <GL/glew.h>
#include <GLFW/glfw3.h>

// Knihovna GLFW jako objekt (RAII):
// konstruktor ji inicializuje, destruktor ukončí
class GlfwLibrary {
public:
  GlfwLibrary();
  ~GlfwLibrary();

  // zákaz kopírování: dvě kopie by GLFW ukončily dvakrát
  GlfwLibrary(const GlfwLibrary &) = delete;
  GlfwLibrary &operator=(const GlfwLibrary &) = delete;
};

// Okno jako objekt (RAII):
// konstruktor ho vytvoří, destruktor zničí
class Window {
public:
  Window(int width, int height, const char *title);
  ~Window();

  // zákaz kopírování: dvě kopie by zničily stejné okno dvakrát
  Window(const Window &) = delete;
  Window &operator=(const Window &) = delete;

  // "surové" okno pro funkce GLFW, které ho potřebují
  // const znamená, že metoda objekt nemění, jen z něj čte
  GLFWwindow *get() const;
  void set_vsync(bool enabled);
  void toggle_fullscreen();

private:
  GLFWwindow *handle = nullptr;
  int windowed_x = 0;
  int windowed_y = 0;
  int windowed_width;
  int windowed_height;
  bool windowed_maximized = false;
};