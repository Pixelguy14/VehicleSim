#include "graphics/Shader.h"

#include <cstddef>
#include <stdexcept>
#include <string>
#include <utility>

namespace vs {
namespace {

/// Log de un shader o de un programa. Devuelve cadena vacia si el driver no
/// tiene nada que decir (longitud 0), en lugar de un std::string con basura.
std::string InfoLog(GLuint object, bool isProgram) {
    GLint length = 0;
    if (isProgram) {
        glGetProgramiv(object, GL_INFO_LOG_LENGTH, &length);
    } else {
        glGetShaderiv(object, GL_INFO_LOG_LENGTH, &length);
    }

    if (length <= 0) {
        return {};
    }

    std::string log(static_cast<std::size_t>(length), '\0');
    GLsizei written = 0;
    if (isProgram) {
        glGetProgramInfoLog(object, length, &written, log.data());
    } else {
        glGetShaderInfoLog(object, length, &written, log.data());
    }

    log.resize(static_cast<std::size_t>(written));
    return log;
}

GLuint CompileStage(GLenum stage, const char* source, const char* stageName) {
    const GLuint shader = glCreateShader(stage);
    glShaderSource(shader, 1, &source, nullptr);
    glCompileShader(shader);

    GLint compiled = GL_FALSE;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
    if (compiled == GL_FALSE) {
        const std::string log = InfoLog(shader, false);
        glDeleteShader(shader);
        throw std::runtime_error(std::string("Error al compilar el shader ") + stageName + ":\n" +
                                 log);
    }

    return shader;
}

}  // namespace

Shader::Shader(const char* vertexSource, const char* fragmentSource) {
    if (vertexSource == nullptr || fragmentSource == nullptr) {
        throw std::runtime_error("Shader: codigo fuente nulo");
    }

    const GLuint vertex = CompileStage(GL_VERTEX_SHADER, vertexSource, "de vertices");

    // Si el fragment shader falla hay que soltar el de vertices a mano: no hay
    // RAII para un GLuint suelto y fugarlo en cada error de desarrollo acaba
    // agotando recursos tras editar el shader veinte veces.
    GLuint fragment = 0;
    try {
        fragment = CompileStage(GL_FRAGMENT_SHADER, fragmentSource, "de fragmentos");
    } catch (...) {
        glDeleteShader(vertex);
        throw;
    }

    program_ = glCreateProgram();
    glAttachShader(program_, vertex);
    glAttachShader(program_, fragment);
    glLinkProgram(program_);

    // Los shaders ya estan dentro del programa: se pueden borrar y el programa
    // sigue funcionando. Es lo que recomienda la especificacion.
    glDeleteShader(vertex);
    glDeleteShader(fragment);

    GLint linked = GL_FALSE;
    glGetProgramiv(program_, GL_LINK_STATUS, &linked);
    if (linked == GL_FALSE) {
        const std::string log = InfoLog(program_, true);
        glDeleteProgram(program_);
        program_ = 0;
        throw std::runtime_error("Error al enlazar el programa de shader:\n" + log);
    }
}

Shader::~Shader() {
    if (program_ != 0) {
        glDeleteProgram(program_);
    }
}

Shader::Shader(Shader&& other) noexcept : program_(std::exchange(other.program_, 0)) {}

Shader& Shader::operator=(Shader&& other) noexcept {
    if (this != &other) {
        if (program_ != 0) {
            glDeleteProgram(program_);
        }
        program_ = std::exchange(other.program_, 0);
    }
    return *this;
}

void Shader::Bind() const {
    glUseProgram(program_);
}

GLint Shader::Uniform(const char* name) const {
    return glGetUniformLocation(program_, name);
}

}  // namespace vs
