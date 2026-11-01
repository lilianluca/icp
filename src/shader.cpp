#include "shader.hpp"

#include <fstream>
#include <glm/gtc/type_ptr.hpp>
#include <sstream>
#include <stdexcept>

namespace { // anonymní namespace - funkce viditelné jen v tomto souboru
std::string read_file(const std::string &path) {
  std::ifstream file(path);
  if (!file) {
    throw std::runtime_error("Nelze otevřít soubor " + path);
  }

  std::stringstream buffer;
  buffer << file.rdbuf(); // přesune celý obsah souboru do bufferu
  return buffer.str();
}

GLuint compile_shader(GLenum type, const std::string &path) {
  std::string source = read_file(path);
  const char *source_ptr = source.c_str();

  GLuint shader = glCreateShader(type);
  glShaderSource(shader, 1, &source_ptr, nullptr);
  glCompileShader(shader); // překlad za běhu programu!

  GLint success = GL_FALSE;
  glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
  if (!success) {
    char log[1024];
    glGetShaderInfoLog(shader, sizeof(log), nullptr, log);
    glDeleteShader(shader);
    throw std::runtime_error("Nepodařilo se zkompilovat shader: " + path + " " +
                             log);
  }
  return shader;
}
} // namespace

ShaderProgram::ShaderProgram(const std::string &vertex_path,
                             const std::string &fragment_path) {
  GLuint vertex = compile_shader(GL_VERTEX_SHADER, vertex_path);

  GLuint fragment = 0;
  try {
    fragment = compile_shader(GL_FRAGMENT_SHADER, fragment_path);
  } catch (...) { // chytí jakoukoli výjimku
    glDeleteShader(vertex);
    throw; // pošle stejnou výjimku dál
  }

  id = glCreateProgram();
  glAttachShader(id, vertex);
  glAttachShader(id, fragment);
  glLinkProgram(id); // spojí oba shadery do jednoho programu

  // uvolnění shaderů - po slinkování už nejsou potřeba
  glDeleteShader(vertex);
  glDeleteShader(fragment);

  GLint success = GL_FALSE;
  glGetProgramiv(id, GL_LINK_STATUS, &success);
  if (!success) {
    char log[1024];
    glGetProgramInfoLog(id, sizeof(log), nullptr, log);
    glDeleteProgram(id);
    throw std::runtime_error(
        std::string("Nepodařilo se slinkovat shader program: ") + log);
  }
}

ShaderProgram::~ShaderProgram() { glDeleteProgram(id); }

void ShaderProgram::use() const { glUseProgram(id); }

void ShaderProgram::set_mat4(const std::string &name,
                             const glm::mat4 &value) const {
  GLint location = glGetUniformLocation(id, name.c_str());
  if (location == -1) {
    throw std::runtime_error("Uniform location nenalezena " + name);
  }

  glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(value));
}