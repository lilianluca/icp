#pragma once

#include <array>

class Playfield {
public:
  static constexpr int width = 8;
  static constexpr int height = 5;
  static constexpr int depth = 10;

  bool is_occupied(int x, int y, int z) const {
    return cells.at(z).at(y).at(x);
  }

  void occupy(int x, int y, int z) {
    cells.at(z).at(y).at(x) = true;
  }

private:
  using Row = std::array<bool, width>;
  using Layer = std::array<Row, height>;
  std::array<Layer, depth> cells{};
};
