#include "settings.hpp"

#include <fstream>
#include <stdexcept>

#include <nlohmann/json.hpp>

Settings::Settings(const std::string &path) {
  std::ifstream file(path);
  if (!file) {
    throw std::runtime_error("Nelze otevřít soubor s nastavením: " + path);
  }

  try {
    nlohmann::json j = nlohmann::json::parse(file);

    title = j.at("title").get<std::string>();
    width = j.at("default_resolution").at("x").get<int>();
    height = j.at("default_resolution").at("y").get<int>();
    vsync = j.value("vsync", true);
    gui_font = j.value("gui_font", "assets/fonts/LiberationSans-Regular.ttf");
    gui_font_size = j.value("gui_font_size", 18.0f);
  } catch (const nlohmann::json::exception &e) {
    throw std::runtime_error("Chyba v souboru " + path + ": " + e.what());
  }
}