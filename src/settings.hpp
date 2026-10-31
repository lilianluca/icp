#pragma once

#include <string>

// Nastavení aplikace načtené ze souboru JSON
struct Settings {
  // načte nastavení ze souboru, při chybě vyhodí výjimku
  explicit Settings(const std::string &path);

  std::string title;
  int width = 0;
  int height = 0;
  bool vsync = true;
};