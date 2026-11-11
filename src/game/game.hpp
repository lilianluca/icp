#pragma once

#include "piece_generator.hpp"
#include "playfield.hpp"

#include <algorithm>
#include <cmath>
#include <utility>

class Game {
public:
  static Game layer_clear_demo() {
    Game demo;
    // Kontrolní scéna s L dílkem.
    demo.active = Piece({{0, 0, 0}, {1, 0, 0}, {1, 1, 0}},
                        {Playfield::width / 2 - 1, Playfield::height / 2 - 1, 0});
    const auto opening = demo.active.world_cells();
    // Na dně necháme otvor přesně pod výchozím dílkem.
    for (int y = 0; y < Playfield::height; ++y) {
      for (int x = 0; x < Playfield::width; ++x) {
        const bool in_opening = std::any_of(
            opening.begin(), opening.end(), [x, y](const CellPosition &cell) {
              return cell.x == x && cell.y == y;
            });
        if (!in_opening) {
          demo.field.occupy(x, y, Playfield::depth - 1);
        }
      }
    }
    // Tento blok po smazání plné vrstvy klesne o jednu vrstvu.
    demo.field.occupy(0, 0, Playfield::depth - 2);
    return demo;
  }

  const Playfield &board() const { return field; }
  const Piece &active_piece() const { return active; }
  bool is_over() const { return over; }
  bool is_paused() const { return paused; }

  void toggle_pause() {
    if (!over) {
      paused = !paused;
    }
  }

  int cleared_layers() const { return total_cleared_layers; }

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
    if (over || paused) {
      return false;
    }
    Piece candidate = active;
    CellPosition target = active.get_position();
    target.x += dx;
    target.y += dy;
    candidate.set_position(target);
    return try_transform(std::move(candidate));
  }

  bool rotate(Rotation rotation) {
    if (over || paused) {
      return false;
    }
    Piece candidate = active;
    if (!candidate.fit_rotation_frame(Playfield::width, Playfield::height)) {
      return false;
    }
    candidate.rotate(rotation);
    return try_transform(std::move(candidate));
  }

  void update(double elapsed_seconds) {
    // Pauza uchová i rozpracovaný pád mezi vrstvami bez přičítání času.
    if (over || paused || !std::isfinite(elapsed_seconds) ||
        elapsed_seconds <= 0.0) {
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
    if (over || paused) {
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
  bool try_transform(Piece candidate) {
    // Posun i otočení ověříme před změnou aktivního dílku.
    if (!field.can_place(candidate)) {
      return false;
    }
    const float offset = fall_offset();
    if (offset > 0.0f) {
      // Během pádu dílek částečně zasahuje i do následující vrstvy.
      Piece below = candidate;
      CellPosition target = candidate.get_position();
      ++target.z;
      below.set_position(target);
      if (!field.can_place(below)) {
        return false;
      }
    }
    active = std::move(candidate);
    if (offset == 0.0f && fall_offset() > 0.0f) {
      // Po odsunutí z opory začíná plynulý pád z aktuální hloubky.
      fall_elapsed = 0.0;
    }
    return true;
  }

  bool lock_active_piece() {
    Piece next = spawn_piece();
    if (!field.try_lock(active)) {
      return false;
    }
    total_cleared_layers += field.clear_full_layers();
    active = std::move(next);
    over = !field.can_place(active);
    return true;
  }

  Piece spawn_piece() {
    Piece piece = pieces.next();
    int rotation_size = 1;
    for (const CellPosition &cell : piece.world_cells()) {
      rotation_size = std::max(rotation_size, std::max(cell.x, cell.y) + 1);
    }
    // Vystředíme celý rotační čtverec, aby se dílek mohl otočit i po objevení.
    piece.set_position({(Playfield::width - rotation_size) / 2,
                        (Playfield::height - rotation_size) / 2, 0});
    return piece;
  }

  static constexpr double fall_interval = 1.0;
  double fall_elapsed = 0.0;
  Playfield field;
  PieceGenerator pieces; // musí vzniknout před prvním aktivním dílkem
  Piece active = spawn_piece();
  bool over = false;
  bool paused = false;
  int total_cleared_layers = 0;
};
