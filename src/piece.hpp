#pragma once

#include <utility>
#include <vector>

struct CellPosition {
  int x = 0;
  int y = 0;
  int z = 0;
};

class Piece {
public:
  explicit Piece(std::vector<CellPosition> cells, CellPosition position = {})
      : local_cells(std::move(cells)), position(position) {}

  void set_position(CellPosition new_position) {
    position = new_position;
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
};
