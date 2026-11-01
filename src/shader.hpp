#pragma once

#include <GL/glew.h>
#include <glm/glm.hpp>
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
  void set_mat4(const std::string &name, const glm::mat4 &value) const;

private:
  GLuint id = 0;
};