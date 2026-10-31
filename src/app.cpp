#include "app.hpp"

#include <cstdlib>
#include <stdexcept>

App::App() : window(800, 600, "ICP") {
  glfwMakeContextCurrent(window.get());
  // vertikální synchronizace zapnutá, max. tolik snímků kolik zvládá monitor
  glfwSwapInterval(1);

  // inicializace GLEW (až teď, protože potřebuje kontext)
  if (glewInit() != GLEW_OK) {
    throw std::runtime_error("GLEW se nepodařilo inicializovat");
  }
}

int App::run() {
  // hlavní smyčka (1 obrátka = 1 snímek)
  while (!glfwWindowShouldClose(window.get())) {
    // vstup
    if (glfwGetKey(window.get(), GLFW_KEY_ESCAPE) == GLFW_PRESS) {
      glfwSetWindowShouldClose(window.get(), GLFW_TRUE);
    }

    // kreslení
    glClearColor(1.0f, 1.0f, 0.0f, 1.0f);

    glClear(GL_COLOR_BUFFER_BIT);
    glfwSwapBuffers(window.get());

    // události
    glfwPollEvents();
  }

  return EXIT_SUCCESS;
}
