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
  if (ImGui::IsKeyPressed(ImGuiKey_Space, false)) {
    game.drop();
  }
}

void App::draw_overlay() {
  ImGui::SetNextWindowPos(ImVec2(10.0f, 10.0f));
  ImGui::SetNextWindowBgAlpha(0.7f);
  ImGui::Begin("stats", nullptr,
               ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize |
                   ImGuiWindowFlags_NoInputs);
  ImGui::Text("FPS: %.0f | Frame: %.1f ms", fps.get(), fps.frame_time_ms());
  ImGui::TextUnformatted("Arrows: move | Space: drop");
  ImGui::TextUnformatted("Mouse: perspective | R: restart | Esc: exit");
  if (game.is_over()) {
    ImGui::TextUnformatted("Game over - press R to restart");
  }
  ImGui::End();
}

int App::run() {
  while (!glfwWindowShouldClose(window.get())) {
    glfwPollEvents();
    if (glfwWindowShouldClose(window.get())) {
      break;
    }
    int width, height;
    glfwGetFramebufferSize(window.get(), &width, &height);
    if (width == 0 || height == 0) {
      glfwWaitEvents();
      continue;
    }

    gui.begin_frame();
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

    // Volne misto nad scenou pro ovladani a stav hry, take na HiDPI displejich.
    const float scale = static_cast<float>(height) / window_height;
    const int overlay_height =
        static_cast<int>(((settings.gui_font_size + 4) * 4 + 20) * scale);
    const int scene_height = std::max(height - overlay_height, 1);
    glViewport(0, 0, width, height);
    glClearColor(0.02f, 0.02f, 0.04f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glViewport(0, 0, width, scene_height);
    renderer.draw(game, static_cast<float>(width) / scene_height, eye_offset);
    draw_overlay();
    gui.end_frame();
    glfwSwapBuffers(window.get());
    fps.update();
  }
  return EXIT_SUCCESS;
}
