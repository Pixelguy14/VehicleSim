#pragma once

#include <glad/gl.h>

namespace vs {

/// Programa de shader compilado y enlazado.
///
/// Falla lanzando std::runtime_error con el log del driver: un shader mal
/// escrito es el error mas comun al empezar con OpenGL y el log de NVIDIA o
/// Mesa dice exactamente en que linea esta el problema. Devolver un bool
/// obligaria a cada llamador a inventarse un mensaje peor.
class Shader {
public:
    Shader(const char* vertexSource, const char* fragmentSource);
    ~Shader();

    Shader(const Shader&) = delete;
    Shader& operator=(const Shader&) = delete;

    Shader(Shader&& other) noexcept;
    Shader& operator=(Shader&& other) noexcept;

    void Bind() const;

    /// Localizacion de un uniform. Se consulta una vez al construir y se cachea:
    /// glGetUniformLocation es una llamada al driver por nombre, y hacerla cada
    /// frame por cada uniform es tirar tiempo sin motivo.
    [[nodiscard]] GLint Uniform(const char* name) const;

    [[nodiscard]] GLuint Id() const { return program_; }

private:
    GLuint program_ = 0;
};

}  // namespace vs
