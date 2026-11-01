#pragma once

#include <GL/glew.h>
#include <string>

// Shader program jako objekt (RAII):
// konstruktor ho přeloží, destruktor uvolní
class ShaderProgram {
public:
  ShaderProgram(const std::string &vertex_path,
                const std::string &fragment_path);
  ~ShaderProgram();

  // zákaz kopírování
  ShaderProgram(const ShaderProgram &) = delete;
  ShaderProgram &operator=(const ShaderProgram &) = delete;

  void use() const; // další kreslení použije tento program

private:
  GLuint id = 0;
};