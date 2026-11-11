#include "gui.hpp"

#include <filesystem>
#include <stdexcept>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

Gui::Gui(GLFWwindow *window, const std::string &font_path, float font_size) {
  IMGUI_CHECKVERSION(); // kontrola, že hlavičky a knihovna mají stejnou verzi
  ImGui::CreateContext();
  ImGui::GetIO().IniFilename =
      nullptr; // neukládat imgui.ini do pracovní složky

  // znaky, které má font obsahovat:
  // 0x0020-0x00FF = základní latinka + západoevropské znaky
  // 0x0100-0x017F = Latin Extended-A (č, ř, ž, ě, ů, ...)
  // 0 na konci = konec seznamu
  static const ImWchar ranges[] = {0x0020, 0x00FF, 0x0100, 0x017F, 0};

  if (!std::filesystem::exists(font_path)) {
    ImGui::DestroyContext();
    throw std::runtime_error("Soubor s fontem: " + font_path + " neexistuje");
  }
  ImGui::GetIO().Fonts->AddFontFromFileTTF(font_path.c_str(), font_size,
                                           nullptr, ranges);
  // Velký font se načte přímo ve své velikosti pro ostré číslice počítadla.
  counter_font = ImGui::GetIO().Fonts->AddFontFromFileTTF(
      font_path.c_str(), 64.0f, nullptr, ranges);

  if (!ImGui_ImplGlfw_InitForOpenGL(window, true)) {
    ImGui::DestroyContext();
    throw std::runtime_error("Nepodařilo se inicializovat ImGui");
  }

  if (!ImGui_ImplOpenGL3_Init("#version 330 core")) {
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    throw std::runtime_error("Nepodařilo se inicializovat ImGui");
  }
}

Gui::~Gui() {
  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  ImGui::DestroyContext();
}

void Gui::begin_frame() {
  ImGui_ImplOpenGL3_NewFrame();
  ImGui_ImplGlfw_NewFrame();
  ImGui::NewFrame();
}

void Gui::end_frame() {
  ImGui::Render(); // sestaví, co se má nakreslit
  ImGui_ImplOpenGL3_RenderDrawData(
      ImGui::GetDrawData()); // nakreslí to přes OpenGL
}