#pragma once

#include "playfield.hpp"

#include <cmath>
#include <utility>

class Game {
public:
  const Playfield &board() const { return field; }
  const Piece &active_piece() const { return active; }
  bool is_over() const { return over; }

  float fall_offset() const {
    if (over) {
      return 0.0f;
    }
    // Plynulý posun mezi vrstvami končí před dnem nebo obsazenou buňkou.
    Piece below = active;
    CellPosition target = active.get_position();
    ++target.z;
    below.set_position(target);
    if (!field.can_place(below)) {
      return 0.0f;
    }
    return static_cast<float>(fall_elapsed / fall_interval);
  }

  bool move(int dx, int dy) {
    if (over) {
      return false;
    }
    CellPosition target = active.get_position();
    target.x += dx;
    target.y += dy;
    const float offset = fall_offset();
    if (offset > 0.0f) {
      // Během pádu dílek částečně zasahuje i do následující vrstvy.
      Piece below = active;
      below.set_position({target.x, target.y, target.z + 1});
      if (!field.can_place(below)) {
        return false;
      }
    }
    if (!field.try_move(active, target)) {
      return false;
    }
    if (offset == 0.0f && fall_offset() > 0.0f) {
      // Po odsunutí z opory začíná plynulý pád z aktuální hloubky.
      fall_elapsed = 0.0;
    }
    return true;
  }

  void update(double elapsed_seconds) {
    if (over || !std::isfinite(elapsed_seconds) || elapsed_seconds <= 0.0) {
      return;
    }
    // Zbytek času přenášíme do dalšího snímku, aby rychlost nezávisela na FPS.
    fall_elapsed += elapsed_seconds;
    while (fall_elapsed >= fall_interval && !over) {
      fall_elapsed -= fall_interval;
      CellPosition target = active.get_position();
      ++target.z;
      if (!field.try_move(active, target)) {
        if (!lock_active_piece()) {
          return;
        }
      }
    }
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
    if (!lock_active_piece()) {
      return false;
    }
    // Nový dílek dostane po rychlém pádu celý interval do dalšího kroku.
    fall_elapsed = 0.0;
    return true;
  }

private:
  bool lock_active_piece() {
    Piece next = spawn_piece();
    if (!field.try_lock(active)) {
      return false;
    }
    active = std::move(next);
    over = !field.can_place(active);
    return true;
  }

  static Piece spawn_piece() {
    return Piece({{0, 0, 0}, {1, 0, 0}, {1, 1, 0}},
                 {Playfield::width / 2 - 1, Playfield::height / 2 - 1, 0});
  }

  static constexpr double fall_interval = 1.0;
  double fall_elapsed = 0.0;
  Playfield field;
  Piece active = spawn_piece();
  bool over = false;
};
