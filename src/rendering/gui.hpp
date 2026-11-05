#pragma once

#include <string>

struct GLFWwindow; // dopředná deklarace: "existuje typ GLFWwindow"

// Knihovna Dear ImGui jako objekt (RAII):
// konstruktor ji inicializuje, destruktor ukončí
class Gui {
public:
  Gui(GLFWwindow *window, const std::string &font_path, float font_size);
  ~Gui();

  // zákaz kopírování
  Gui(const Gui &) = delete;
  Gui &operator=(const Gui &) = delete;

  void begin_frame(); // na začátku snímku, před voláním ImGui::...
  void end_frame();   // na konci snímku: vykreslí GUI (přes scénu)
};