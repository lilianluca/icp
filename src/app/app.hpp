#pragma once

#include "fps_meter.hpp"
#include "rendering/gui.hpp"
#include "settings.hpp"
#include "rendering/well_renderer.hpp"
#include "rendering/window.hpp"

class App {
public:
  App(); // konstruktor: zavolá se automaticky při vytvoření objektu

  int run(); // hlavní smyčka, vrací návratový kód programu

private:
  void handle_input();
  void draw_overlay();

  // nejdříve konfigurace, pak knihovny, pak zdroje
  Settings settings; // nastavení musí vzniknout před window
  GlfwLibrary glfw;  // glfw musí vzniknout před window
  Window window;
  Gui gui; // gui potřebuji inicializované okno
  FpsMeter fps;
  Game game;
  WellRenderer renderer;
};
