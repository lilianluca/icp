#pragma once

#include "settings.hpp"
#include "window.hpp"

class App {
public:
  App(); // konstruktor: zavolá se automaticky při vytvoření objektu

  int run(); // hlavní smyčka, vrací návratový kód programu

private:
  // nejdříve konfigurace, pak knihovny, pak zdroje
  Settings settings; // nastavení musí vzniknout před window
  GlfwLibrary glfw;  // glfw musí vzniknout před window
  Window window;
};