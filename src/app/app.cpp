#include "app.hpp"

#include <algorithm>
#include <cstdlib>
#include <imgui.h>

App::App()
    : settings("config/settings.json"),
      window(settings.width, settings.height, settings.title.c_str()),
      gui(window.get(), settings.gui_font, settings.gui_font_size) {
  window.set_vsync(settings.vsync);
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_MULTISAMPLE); // vyhlazení hran při pohybu kamery
  // zapnutí průhlednosti (míchání barev)
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void App::handle_input() {
  if (ImGui::IsKeyPressed(ImGuiKey_Escape, false)) {
    glfwSetWindowShouldClose(window.get(), GLFW_TRUE);
  }
  if (ImGui::GetIO().WantCaptureKeyboard) {
    return;
  }
  if (ImGui::IsKeyPressed(ImGuiKey_F2, false)) {
    game = Game::layer_clear_demo();
    return;
  }
  if (ImGui::IsKeyPressed(ImGuiKey_R, false)) {
    game = Game{};
    return;
  }
  const int dx = static_cast<int>(ImGui::IsKeyPressed(ImGuiKey_RightArrow)) -
                 static_cast<int>(ImGui::IsKeyPressed(ImGuiKey_LeftArrow));
  const int dy = static_cast<int>(ImGui::IsKeyPressed(ImGuiKey_UpArrow)) -
                 static_cast<int>(ImGui::IsKeyPressed(ImGuiKey_DownArrow));
  if (dx != 0 || dy != 0) {
    game.move(dx, dy);
  }
  if (ImGui::IsKeyPressed(ImGuiKey_Q, false)) {
    game.rotate(Rotation::ZPositive);
  }
  if (ImGui::IsKeyPressed(ImGuiKey_E, false)) {
    game.rotate(Rotation::ZNegative);
  }
  if (ImGui::IsKeyPressed(ImGuiKey_Space, false)) {
    game.drop();
  }
}

void App::draw_overlay() {
  // popis GUI
  ImGui::SetNextWindowPos(ImVec2(10.0f, 10.0f)); // levý horní roh
  ImGui::SetNextWindowBgAlpha(0.7f); // poloprůhledné pozadí
  ImGui::Begin("stats", nullptr,
               ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize |
                   ImGuiWindowFlags_NoInputs); // bez rámečku, titulku a klikání
  ImGui::Text("FPS: %.0f | Frame: %.1f ms", fps.get(), fps.frame_time_ms());
  ImGui::TextUnformatted("Arrows: move | Space: drop");
  ImGui::TextUnformatted("Q/E: rotate around Z");
  ImGui::TextUnformatted("F2: layer clear demo");
  ImGui::TextUnformatted("Mouse: perspective | R: restart | Esc: exit");
  if (game.is_over()) {
    ImGui::TextUnformatted("Game over - press R to restart");
  }
  ImGui::End();

  // Počítadlo sleduje horní střed okna i při změně jeho velikosti.
  ImGui::SetNextWindowPos(ImVec2(ImGui::GetIO().DisplaySize.x * 0.5f, 16.0f),
                          ImGuiCond_Always, ImVec2(0.5f, 0.0f));
  ImGui::PushFont(gui.get_counter_font());
  ImGui::Begin("layer_counter", nullptr,
               ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize |
                   ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoBackground);
  ImGui::Text("%d", game.cleared_layers());
  ImGui::End();
  ImGui::PopFont();
}

int App::run() {
  double last_frame_time = glfwGetTime();
  // hlavní smyčka (1 obrátka = 1 snímek)
  while (!glfwWindowShouldClose(window.get())) {
    glfwPollEvents();
    if (glfwWindowShouldClose(window.get())) {
      break;
    }
    int width, height;
    glfwGetFramebufferSize(window.get(), &width, &height);
    // ochrana proti dělení nulou při minimalizaci
    if (width == 0 || height == 0) {
      glfwWaitEvents();
      last_frame_time = glfwGetTime(); // po obnovení okna nedohánět dobu minimalizace
      continue;
    }

    // Čas od předchozího snímku pro pád nezávislý na rychlosti vykreslování.
    const double now = glfwGetTime();
    const double elapsed_seconds = now - last_frame_time;
    last_frame_time = now;

    gui.begin_frame();
    game.update(elapsed_seconds);
    handle_input();

    int window_width, window_height;
    glfwGetWindowSize(window.get(), &window_width, &window_height);
    double mouse_x, mouse_y;
    glfwGetCursorPos(window.get(), &mouse_x, &mouse_y);
    const glm::vec2 eye_offset(
        std::clamp(static_cast<float>(mouse_x / window_width) * 2.0f - 1.0f,
                   -1.0f, 1.0f) * Playfield::width * 0.5f,
        -std::clamp(static_cast<float>(mouse_y / window_height) * 2.0f - 1.0f,
                    -1.0f, 1.0f) * Playfield::height * 0.5f);

    // Scéna vyplňuje celé okno, GUI se vykreslí přes ni.
    glViewport(0, 0, width, height);
    glClearColor(0.02f, 0.02f, 0.04f, 1.0f);
    // vymazat barvu i hloubku před kreslením dalšího snímku
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    // poměr stran oblasti pro vykreslování scény
    renderer.draw(game, static_cast<float>(width) / height, eye_offset);
    draw_overlay();
    gui.end_frame(); // GUI přes scénu
    glfwSwapBuffers(window.get()); // zobrazit
    fps.update();
  }
  return EXIT_SUCCESS;
}
