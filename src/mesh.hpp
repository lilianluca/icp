#pragma once

#include <GL/glew.h>
#include <vector>

// Geometrie v paměti grafické karty jako objekt (RAII)
class Mesh {
public:
  // vertices: souřadnice x, y, z pro každý vrchol
  explicit Mesh(const std::vector<float> &vertices);
  ~Mesh();

  // zákaz kopírování
  Mesh(const Mesh &) = delete;
  Mesh &operator=(const Mesh &) = delete;

  void draw() const;

private:
  GLuint vao = 0;
  GLuint vbo = 0;
  GLsizei vertex_count = 0;
};