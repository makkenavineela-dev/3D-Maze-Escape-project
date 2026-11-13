#include "Shader.h"

#include <fstream>
#include <glm/gtc/type_ptr.hpp>
#include <sstream>
#include <stdexcept>
#include <vector>

namespace {

std::string readFile(const std::string& path) {
    std::ifstream file(path);
    if (!file) throw std::runtime_error("Cannot open shader file: " + path);
    std::stringstream text;
    text << file.rdbuf();
    return text.str();
}

GLuint compile(GLenum type, const std::string& path) {
    const std::string source = readFile(path);
    const char* text = source.c_str();
    const GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &text, nullptr);
    glCompileShader(shader);

    GLint ok = GL_FALSE;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &ok);
    if (!ok) {
        GLint length = 0;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &length);
        std::vector<char> log(length > 1 ? length : 1);
        glGetShaderInfoLog(shader, static_cast<GLsizei>(log.size()), nullptr, log.data());
        glDeleteShader(shader);
        throw std::runtime_error("Shader compile error in " + path + ":\n" + log.data());
    }
    return shader;
}

}  // namespace

Shader::Shader(const std::string& vertexPath, const std::string& fragmentPath) {
    const GLuint vertex = compile(GL_VERTEX_SHADER, vertexPath);
    GLuint fragment = 0;
    try {
        fragment = compile(GL_FRAGMENT_SHADER, fragmentPath);
    } catch (...) {
        glDeleteShader(vertex);
        throw;
    }

    program_ = glCreateProgram();
    glAttachShader(program_, vertex);
    glAttachShader(program_, fragment);
    glLinkProgram(program_);
    glDeleteShader(vertex);
    glDeleteShader(fragment);

    GLint ok = GL_FALSE;
    glGetProgramiv(program_, GL_LINK_STATUS, &ok);
    if (!ok) {
        GLint length = 0;
        glGetProgramiv(program_, GL_INFO_LOG_LENGTH, &length);
        std::vector<char> log(length > 1 ? length : 1);
        glGetProgramInfoLog(program_, static_cast<GLsizei>(log.size()), nullptr, log.data());
        glDeleteProgram(program_);
        throw std::runtime_error("Shader link error (" + vertexPath + " + " + fragmentPath + "):\n" + log.data());
    }
}

Shader::~Shader() {
    if (program_) glDeleteProgram(program_);
}

void Shader::use() const { glUseProgram(program_); }

GLint Shader::location(const std::string& name) const {
    auto it = locations_.find(name);
    if (it == locations_.end()) it = locations_.emplace(name, glGetUniformLocation(program_, name.c_str())).first;
    return it->second;   // -1 when the shader does not use this uniform; glUniform* then ignores it
}

void Shader::setBool(const std::string& name, bool value) const { glUniform1i(location(name), value ? 1 : 0); }
void Shader::setInt(const std::string& name, int value) const { glUniform1i(location(name), value); }
void Shader::setFloat(const std::string& name, float value) const { glUniform1f(location(name), value); }
void Shader::setVec3(const std::string& name, const glm::vec3& value) const {
    glUniform3fv(location(name), 1, glm::value_ptr(value));
}
void Shader::setVec4(const std::string& name, const glm::vec4& value) const {
    glUniform4fv(location(name), 1, glm::value_ptr(value));
}
void Shader::setMat3(const std::string& name, const glm::mat3& value) const {
    glUniformMatrix3fv(location(name), 1, GL_FALSE, glm::value_ptr(value));
}
void Shader::setMat4(const std::string& name, const glm::mat4& value) const {
    glUniformMatrix4fv(location(name), 1, GL_FALSE, glm::value_ptr(value));
}
