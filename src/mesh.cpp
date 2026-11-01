#include "mesh.hpp"

Mesh::Mesh(const std::vector<float> &vertices) {
  vertex_count = static_cast<GLsizei>(vertices.size() / 3);

  glGenVertexArrays(1, &vao); // vytvoří VAO, ID uloží do vao
  glBindVertexArray(vao);     // "teď pracuji s tímto VAO"

  glGenBuffers(1, &vbo);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferData(GL_ARRAY_BUFFER,                 // nahraje data do karty
               vertices.size() * sizeof(float), // velikost v bajtech
               vertices.data(),                 // ukazatel na data
               GL_STATIC_DRAW);                 // data se nebudou měnit

  // popis dat pro vstup "location = 0" ve vertex shaderu
  glVertexAttribPointer(0,                 // location
                        3,                 // 3 čísla na vrchol
                        GL_FLOAT,          // typ čísel
                        GL_FALSE,          // nenormalizovat
                        3 * sizeof(float), // vzdálenost mezi vrcholy v bajtech
                        nullptr);          // začátek v bufferu
  glEnableVertexAttribArray(0);
}

Mesh::~Mesh() {
  glDeleteBuffers(1, &vbo);
  glDeleteVertexArrays(1, &vao);
}

void Mesh::draw() const {
  glBindVertexArray(vao);
  glDrawArrays(GL_TRIANGLES, 0, vertex_count);
}