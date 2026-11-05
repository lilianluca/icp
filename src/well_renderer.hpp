#pragma once

#include "game.hpp"
#include "mesh.hpp"
#include "shader.hpp"

class WellRenderer {
public:
  WellRenderer();
  void draw(const Game &game, float aspect, glm::vec2 eye_offset);

private:
  void draw_cube(CellPosition cell, glm::vec3 color, const glm::mat4 &projection,
                 float depth_offset = 0.0f);

  ShaderProgram shader;
  Mesh grid;
  Mesh cube;
  Mesh edges;
};
