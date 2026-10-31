#pragma once

#include <GL/glew.h> // GLEW musí být vždy před GLFW
#include <GLFW/glfw3.h>

class App {
public:
  App();  // konstruktor: zavolá se automaticky při vytvoření objektu
  ~App(); // destruktor: zavolá se automaticky při zániku objektu

  void init(); // inicializace (GLFW, okno, GLEW), při chybě vyhodí výjimku
  int run();   // hlavní smyčka, vrací návratový kód programu

private:
  GLFWwindow *window = nullptr; // okno; nullptr = "zatím žádné"
};