#include <cstdlib>
#include <exception>
#include <iostream>

#include "app.hpp"

int main() {
  try {
    App app; // vytvoří objekt -> zavolá se konstruktor
    return app.run();
  } // tady app zaniká -> zavolá se destruktor (úklid)
  catch (const std::exception &e) {
    std::cerr << "Chyba: " << e.what() << "\n";
    return EXIT_FAILURE;
  }
}
