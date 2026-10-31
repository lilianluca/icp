#include "app.hpp"

#include <cstdlib>
#include <iostream>
#include <stdexcept>

App::App() {
  // zatím nic - všechno důležité dělá init()
}

void App::init() {
  // inicializace GLFW
  if (!glfwInit()) {
    throw std::runtime_error("GLFW se nepodařilo inicializovat");
  }

  // vytvoření okna
  window = glfwCreateWindow(800, 600, "ICP", nullptr, nullptr);
  if (!window) {
    throw std::runtime_error("Nepodařilo se vytvořit okno");
  }
  glfwMakeContextCurrent(window);
  // vertikální synchronizace zapnutá, max. tolik snímků kolik zvládá monitor
  glfwSwapInterval(1);

  // inicializace GLEW (až teď, protože potřebuje kontext)
  if (glewInit() != GLEW_OK) {
    throw std::runtime_error("GLEW se nepodařilo inicializovat");
  }
}

int App::run() {
  // hlavní smyčka (1 obrátka = 1 snímek)
  while (!glfwWindowShouldClose(window)) {
    // vstup
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
      glfwSetWindowShouldClose(window, GLFW_TRUE);
    }

    // kreslení
    glClearColor(1.0f, 1.0f, 0, 1.0f);

    glClear(GL_COLOR_BUFFER_BIT);
    glfwSwapBuffers(window);

    // události
    glfwPollEvents();
  }

  return EXIT_SUCCESS;
}

App::~App() {
  // zničení okna
  if (window) {
    glfwDestroyWindow(window);
  }
  glfwTerminate();
  std::cout << "Konec!\n";
}