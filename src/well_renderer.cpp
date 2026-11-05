#include "well_renderer.hpp"

#include <algorithm>
#include <glm/gtc/matrix_transform.hpp>

namespace {
std::vector<float> well_vertices() {
  std::vector<float> vertices;
  const float x0 = -Playfield::width / 2.0f;
  const float x1 = Playfield::width / 2.0f;
  const float y0 = -Playfield::height / 2.0f;
  const float y1 = Playfield::height / 2.0f;
  const float bottom = -static_cast<float>(Playfield::depth);
  auto line = [&](glm::vec3 a, glm::vec3 b) {
    vertices.insert(vertices.end(), {a.x, a.y, a.z, b.x, b.y, b.z});
  };

  // Stejna mrizka sten a dna jako v prototypu, v jednotkovych bunkach.
  for (int z = 0; z <= Playfield::depth; ++z) {
    const float level = -static_cast<float>(z);
    line({x0, y0, level}, {x1, y0, level});
    line({x1, y0, level}, {x1, y1, level});
    line({x1, y1, level}, {x0, y1, level});
    line({x0, y1, level}, {x0, y0, level});
  }
  for (int x = 0; x <= Playfield::width; ++x) {
    const float column = x0 + x;
    line({column, y0, 0}, {column, y0, bottom});
    line({column, y1, 0}, {column, y1, bottom});
    line({column, y0, bottom}, {column, y1, bottom});
  }
  for (int y = 0; y <= Playfield::height; ++y) {
    const float row = y0 + y;
    line({x0, row, 0}, {x0, row, bottom});
    line({x1, row, 0}, {x1, row, bottom});
    line({x0, row, bottom}, {x1, row, bottom});
  }
  return vertices;
}
} // namespace

WellRenderer::WellRenderer()
    : shader("assets/shaders/well.vert", "assets/shaders/well.frag"),
      grid(well_vertices(), GL_LINES),
      cube({0,0,0, 1,0,0, 1,1,0, 0,0,0, 1,1,0, 0,1,0,
            0,0,-1, 1,1,-1, 1,0,-1, 0,0,-1, 0,1,-1, 1,1,-1,
            0,0,-1, 0,0,0, 0,1,0, 0,0,-1, 0,1,0, 0,1,-1,
            1,0,0, 1,0,-1, 1,1,-1, 1,0,0, 1,1,-1, 1,1,0,
            0,0,-1, 1,0,-1, 1,0,0, 0,0,-1, 1,0,0, 0,0,0,
            0,1,0, 1,1,0, 1,1,-1, 0,1,0, 1,1,-1, 0,1,-1}),
      edges({0,0,0, 1,0,0, 1,0,0, 1,1,0,
             1,1,0, 0,1,0, 0,1,0, 0,0,0,
             0,0,-1, 1,0,-1, 1,0,-1, 1,1,-1,
             1,1,-1, 0,1,-1, 0,1,-1, 0,0,-1,
             0,0,0, 0,0,-1, 1,0,0, 1,0,-1,
             1,1,0, 1,1,-1, 0,1,0, 0,1,-1}, GL_LINES) {}

void WellRenderer::draw_cube(CellPosition cell, glm::vec3 color,
                             const glm::mat4 &projection) {
  const glm::vec3 position(cell.x - Playfield::width / 2.0f,
                           cell.y - Playfield::height / 2.0f, -cell.z);
  const glm::mat4 model = glm::translate(glm::mat4(1.0f), position);
  shader.set_mat4("mvp", projection * model);
  shader.set_vec3("color", color * (1.0f - 0.55f * cell.z / Playfield::depth));
  glEnable(GL_POLYGON_OFFSET_FILL);
  glPolygonOffset(1.0f, 1.0f);
  cube.draw();
  glDisable(GL_POLYGON_OFFSET_FILL);
  shader.set_vec3("color", glm::vec3(0.025f));
  edges.draw();
}

void WellRenderer::draw(const Game &game, float aspect, glm::vec2 eye_offset) {
  // Pevny obdelnik okna a posun oka, stejne jako window_projection v prototypu.
  const float half_width = std::max(Playfield::width / 2.0f,
                                     Playfield::height / 2.0f * aspect) * 1.1f;
  const float half_height = half_width / aspect;
  const glm::vec3 eye(eye_offset.x, eye_offset.y, 12.0f);
  const glm::mat4 projection =
      glm::frustum((-half_width - eye.x) / eye.z,
                   (half_width - eye.x) / eye.z,
                   (-half_height - eye.y) / eye.z,
                   (half_height - eye.y) / eye.z, 1.0f, 24.0f) *
      glm::translate(glm::mat4(1.0f), -eye);

  shader.use();
  for (int z = 0; z < Playfield::depth; ++z) {
    for (int y = 0; y < Playfield::height; ++y) {
      for (int x = 0; x < Playfield::width; ++x) {
        if (game.board().is_occupied(x, y, z)) {
          draw_cube({x, y, z}, {0.9f, 0.45f, 0.2f}, projection);
        }
      }
    }
  }
  if (!game.is_over()) {
    for (const CellPosition &cell : game.active_piece().world_cells()) {
      draw_cube(cell, {0.45f, 0.85f, 0.35f}, projection);
    }
  }
  shader.set_mat4("mvp", projection);
  shader.set_vec3("color", {0.25f, 0.75f, 0.85f});
  grid.draw();
}
