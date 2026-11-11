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

  bool try_move(Piece &piece, CellPosition new_position) const {
    Piece candidate = piece;
    candidate.set_position(new_position);
    if (!can_place(candidate)) {
      return false;
    }
    piece.set_position(new_position);
    return true;
  }

  bool try_lock(const Piece &piece) {
    if (!can_place(piece)) {
      return false;
    }
    for (const CellPosition &cell : piece.world_cells()) {
      occupy(cell.x, cell.y, cell.z);
    }
    return true;
  }

  int clear_full_layers() {
    int cleared = 0;
    // Od dna přesouváme celé zbývající vrstvy, včetně prázdných.
    int destination = depth - 1;
    for (int z = depth - 1; z >= 0; --z) {
      if (is_full(cells[z])) {
        ++cleared;
        continue;
      }
      if (destination != z) {
        cells[destination] = cells[z];
      }
      --destination;
    }
    while (destination >= 0) {
      cells[destination--] = {};
    }
    return cleared;
  }

private:
  using Row = std::array<bool, width>;
  using Layer = std::array<Row, height>;
  static bool is_full(const Layer &layer) {
    for (const Row &row : layer) {
      for (bool occupied : row) {
        if (!occupied) {
          return false;
        }
      }
    }
    return true;
  }

  std::array<Layer, depth> cells{};
};
