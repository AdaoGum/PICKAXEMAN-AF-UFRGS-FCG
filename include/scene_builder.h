// scene_builder.h
// Este arquivo contém a função BuildTriangles() e estruturas relacionadas
// para construção da geometria básica da cena (cubo, chão, eixos).

#ifndef SCENE_BUILDER_H
#define SCENE_BUILDER_H

#include <map>
#include <string>
#include <glad/glad.h>
#include <glm/vec3.hpp>

// Estrutura que representa um objeto geométrico carregado a partir de um
// arquivo ".obj". Veja https://en.wikipedia.org/wiki/Wavefront_.obj_file .
struct SceneObject
{
    std::string  name;              // Nome do objeto
    size_t       first_index;       // Índice do primeiro vértice
    size_t       num_indices;       // Número de índices
    GLenum       rendering_mode;    // Modo de rasterização
    GLuint       vertex_array_object_id; // ID do VAO
    glm::vec3    bbox_min;          // Bounding Box min
    glm::vec3    bbox_max;          // Bounding Box max
};

// Mapa atualizado para usar string como chave
// Armazena todos os objetos da cena virtual
extern std::map<std::string, SceneObject> g_VirtualScene;

// Constrói triângulos para futura renderização
// Retorna o ID do VAO criado
GLuint BuildTriangles();

#endif // SCENE_BUILDER_H
