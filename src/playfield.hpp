#pragma once

#include "piece.hpp"

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

  bool can_place(const Piece &piece) const {
    for (const CellPosition &cell : piece.world_cells()) {
      if (cell.x < 0 || cell.x >= width || cell.y < 0 || cell.y >= height ||
          cell.z < 0 || cell.z >= depth ||
          is_occupied(cell.x, cell.y, cell.z)) {
        return false;
      }
    }
    return true;
  }

private:
  using Row = std::array<bool, width>;
  using Layer = std::array<Row, height>;
  std::array<Layer, depth> cells{};
};
