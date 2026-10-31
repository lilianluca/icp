#pragma once

#include "window.hpp"

class App {
public:
  App(); // konstruktor: zavolá se automaticky při vytvoření objektu

  int run(); // hlavní smyčka, vrací návratový kód programu

private:
  // pořadí je důležité, glfw musí vzniknout před window
  GlfwLibrary glfw;
  Window window;
};