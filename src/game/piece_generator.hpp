#pragma once

#include "piece.hpp"

#include <algorithm>
#include <array>
#include <cstddef>
#include <random>
#include <vector>

class PieceGenerator {
public:
  Piece next() {
    if (next_index == order.size()) {
      // Každá skupina obsahuje všechny tvary v náhodném pořadí.
      std::shuffle(order.begin(), order.end(), random_engine);
      next_index = 0;
    }
    Piece piece(shapes[order[next_index]]);
    ++next_index;
    return piece;
  }

private:
  inline static const std::array<std::vector<CellPosition>, 5> shapes{{
      {{0, 0, 0}},                                  // jedna kostka
      {{0, 0, 0}, {1, 0, 0}},                       // domino
      {{0, 0, 0}, {1, 0, 0}, {2, 0, 0}},            // přímý dílek
      {{0, 0, 0}, {1, 0, 0}, {1, 1, 0}},            // L dílek
      {{0, 0, 0}, {1, 0, 0}, {0, 1, 0}, {1, 1, 0}} // čtverec
  }};

  std::mt19937 random_engine{std::random_device{}()};
  std::array<std::size_t, 5> order{0, 1, 2, 3, 4};
  std::size_t next_index = order.size();
};
