#include "app.hpp"

#include <GLFW/glfw3.h>
#include <cstdlib>
#include <glm/gtc/matrix_transform.hpp>
#include <imgui.h>

App::App()
    : settings("config/settings.json"),
      window(settings.width, settings.height, settings.title.c_str()),
      gui(window.get(), settings.gui_font, settings.gui_font_size),
      shader("assets/shaders/triangle.vert", "assets/shaders/triangle.frag"),
      triangle({-0.5f, -0.5f, 0.0f, 0.5f, -0.5f, 0.0f, 0.0f, 0.5f, 0.0f}) {
  window.set_vsync(settings.vsync);

  // zapnutí průhlednosti (míchání barev)
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

int App::run() {
  // hlavní smyčka (1 obrátka = 1 snímek)
  while (!glfwWindowShouldClose(window.get())) {
    glfwPollEvents();

    int width, height;
    glfwGetFramebufferSize(window.get(), &width, &height);
    glViewport(0, 0, width, height);

    gui.begin_frame();

    if (glfwGetKey(window.get(), GLFW_KEY_ESCAPE) == GLFW_PRESS) {
      glfwSetWindowShouldClose(window.get(), GLFW_TRUE);
    }

    // popis GUI
    ImGui::SetNextWindowPos(ImVec2(10.0f, 10.0f)); // levý horní roh
    ImGui::SetNextWindowBgAlpha(0.35f);            // poloprůhledné pozadí
    ImGui::Begin(
        "stats", nullptr,
        ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize |
            ImGuiWindowFlags_NoInputs); // bez rámečku, titulku a klikání
    ImGui::Text("FPS: %.0f", fps.get());
    ImGui::Text("Doba snímku: %0.1f ms", fps.frame_time_ms());
    ImGui::End();

    glClearColor(1.0f, 1.0f, 0.0f, 1.0f);

    glClear(GL_COLOR_BUFFER_BIT);

    // poměr stran okna (ochrana proti dělení nulou při minimalizaci)
    float aspect = height > 0 ? static_cast<float>(width) / height : 1.0f;
    float angle = static_cast<float>(glfwGetTime());
    glm::mat4 model =
        glm::rotate(glm::mat4(1.0f), angle, glm::vec3(0.0f, 1.0f, 0.0f));
    glm::mat4 view = glm::lookAt(glm::vec3(0.0f, 0.0f, 3.0f),  // kde je kamera
                                 glm::vec3(0.0f, 0.0f, 0.0f),  // kam se dívá
                                 glm::vec3(0.0f, 1.0f, 0.0f)); // kde je nahoře
    glm::mat4 projection =
        glm::perspective(glm::radians(45.0f), aspect, // zorný úhel, poměr
                         0.1f, 100.0f);               // blízká a daleká hranice
    shader.use();
    shader.set_mat4("mvp", projection * view * model);

    triangle.draw();

    gui.end_frame();               // GUI přes scénu
    glfwSwapBuffers(window.get()); // zobrazit

    fps.update();
  }

  return EXIT_SUCCESS;
}
