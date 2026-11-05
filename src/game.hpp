#pragma once

#include "playfield.hpp"

#include <utility>

class Game {
public:
  const Playfield &board() const { return field; }
  const Piece &active_piece() const { return active; }
  bool is_over() const { return over; }

  bool move(int dx, int dy) {
    if (over) {
      return false;
    }
    CellPosition target = active.get_position();
    target.x += dx;
    target.y += dy;
    return field.try_move(active, target);
  }

  bool drop() {
    if (over) {
      return false;
    }
    CellPosition target = active.get_position();
    ++target.z;
    while (field.try_move(active, target)) {
      ++target.z;
    }
    Piece next = spawn_piece();
    if (!field.try_lock(active)) {
      return false;
    }
    active = std::move(next);
    over = !field.can_place(active);
    return true;
  }

private:
  static Piece spawn_piece() {
    return Piece({{0, 0, 0}, {1, 0, 0}, {1, 1, 0}},
                 {Playfield::width / 2 - 1, Playfield::height / 2 - 1, 0});
  }

  Playfield field;
  Piece active = spawn_piece();
  bool over = false;
};
