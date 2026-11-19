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
  if (ImGui::IsKeyPressed(ImGuiKey_F11, false)) {
    window.toggle_fullscreen();
    return;
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
  if (ImGui::IsKeyPressed(ImGuiKey_P, false)) {
    game.toggle_pause();
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
  ImGui::TextUnformatted("P: pause | F2: layer clear demo");
  ImGui::TextUnformatted("F11: fullscreen");
  ImGui::TextUnformatted("Mouse: perspective | R: restart | Esc: exit");
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
  if (game.is_paused() || game.is_over()) {
    // Stav pauzy nebo konce hry je vidět ve středu okna.
    const bool game_over = game.is_over();
    const char *title = game_over ? "Game over" : "Paused";
    const char *restart_hint = "R - new game";
    const float hint_width = game_over ? ImGui::CalcTextSize(restart_hint).x : 0.0f;
    ImGui::SetNextWindowPos(ImVec2(ImGui::GetIO().DisplaySize.x * 0.5f,
                                  ImGui::GetIO().DisplaySize.y * 0.5f),
                            ImGuiCond_Always, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowBgAlpha(0.7f);
    ImGui::Begin("game_state", nullptr,
                 ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize |
                     ImGuiWindowFlags_NoInputs);
    ImGui::PushFont(gui.get_counter_font());
    const float title_width = ImGui::CalcTextSize(title).x;
    const float content_width = std::max(title_width, hint_width);
    const float padding = ImGui::GetStyle().WindowPadding.x;
    ImGui::SetCursorPosX(padding + (content_width - title_width) * 0.5f);
    ImGui::TextUnformatted(title);
    ImGui::PopFont();
    if (game_over) {
      ImGui::SetCursorPosX(padding + (content_width - hint_width) * 0.5f);
      ImGui::TextUnformatted(restart_hint);
    }
    ImGui::End();
  }
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
    // Přepnutí fullscreen může během ovládání změnit velikost framebufferu.
    glfwGetFramebufferSize(window.get(), &width, &height);
    if (width == 0 || height == 0) {
      ImGui::EndFrame(); // uzavřít rozpracovaný GUI snímek
      continue;
    }

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
