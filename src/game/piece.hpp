#pragma once

#include <algorithm>
#include <utility>
#include <vector>

struct CellPosition {
  int x = 0;
  int y = 0;
  int z = 0;
};

enum class Rotation { ZPositive, ZNegative };

class Piece {
public:
  explicit Piece(std::vector<CellPosition> cells, CellPosition position = {}):
    local_cells(std::move(cells)), position(position) {
    if (local_cells.empty()) {
      return;
    }
    rotation_origin_x = local_cells.front().x;
    rotation_origin_y = local_cells.front().y;
    int max_x = rotation_origin_x;
    int max_y = rotation_origin_y;
    for (const CellPosition &cell : local_cells) {
      rotation_origin_x = std::min(rotation_origin_x, cell.x);
      rotation_origin_y = std::min(rotation_origin_y, cell.y);
      max_x = std::max(max_x, cell.x);
      max_y = std::max(max_y, cell.y);
    }
    // Pevný čtverec určuje střed otáčení pro všechny orientace dílku.
    rotation_size = std::max(max_x - rotation_origin_x, max_y - rotation_origin_y) + 1;
  }

  CellPosition get_position() const { return position; }

  void set_position(CellPosition new_position) {
    position = new_position;
  }

  bool fit_rotation_frame(int width, int height) {
    if (rotation_size > width || rotation_size > height) {
      return false;
    }
    // U stěn upravíme střed otáčení tak, aby se celý rotační čtverec vešel.
    rotation_origin_x = std::clamp(rotation_origin_x, -position.x,
                                   width - rotation_size - position.x);
    rotation_origin_y = std::clamp(rotation_origin_y, -position.y,
                                   height - rotation_size - position.y);
    return true;
  }

  void rotate(Rotation rotation) {
    // Otáčíme středy buněk o 90 stupňů kolem osy Z ve středu pevného čtverce.
    for (CellPosition &cell : local_cells) {
      const CellPosition original = cell;
      const int x = original.x - rotation_origin_x;
      const int y = original.y - rotation_origin_y;
      switch (rotation) {
      case Rotation::ZPositive:
        cell = {rotation_origin_x + rotation_size - 1 - y,
                rotation_origin_y + x, original.z};
        break;
      case Rotation::ZNegative:
        cell = {rotation_origin_x + y,
                rotation_origin_y + rotation_size - 1 - x, original.z};
        break;
      }
    }
  }

  std::vector<CellPosition> world_cells() const {
    std::vector<CellPosition> result;
    result.reserve(local_cells.size());
    for (const CellPosition &cell : local_cells) {
      result.push_back({cell.x + position.x, cell.y + position.y,
                        cell.z + position.z});
    }
    return result;
  }

private:
  std::vector<CellPosition> local_cells;
  CellPosition position;
  int rotation_origin_x = 0;
  int rotation_origin_y = 0;
  int rotation_size = 1;
};
