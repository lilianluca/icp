#include "app.hpp"

#include <cstdlib>
#include <stdexcept>

App::App()
    : settings("config/settings.json"),
      window(settings.width, settings.height, settings.title.c_str()) {
  glfwMakeContextCurrent(window.get());
  // vertikální synchronizace podle nastavení (1 = zapnutá, 0 = vypnutá)
  glfwSwapInterval(settings.vsync ? 1 : 0);

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
