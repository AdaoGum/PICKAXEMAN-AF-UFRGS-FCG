#ifndef SCENE_BUILDER_H
#define SCENE_BUILDER_H

#include <glad/glad.h>
#include <glm/vec4.hpp>
#include <map>

// Definimos uma estrutura que armazenará dados necessários para renderizar
// cada objeto da cena virtual.
struct SceneObject
{
    const char*  name;        // Nome do objeto
    void*        first_index; // Índice do primeiro vértice dentro do vetor indices[]
    int          num_indices; // Número de índices do objeto dentro do vetor indices[]
    GLenum       rendering_mode; // Modo de rasterização (GL_TRIANGLES, GL_TRIANGLE_STRIP, etc.)
    glm::vec4    bbox_min;    // Bounding box: min
    glm::vec4    bbox_max;    // Bounding box: max
};

// A cena virtual é uma lista de objetos nomeados, guardados em um dicionário (map).
// A palavra "inline" aqui evita erros de "definição múltipla" do linker.
inline std::map<const char*, SceneObject> g_VirtualScene;

// A função inteira é colocada aqui, marcada como "inline".
// Isso permite que ela seja incluída diretamente em outros arquivos sem causar erros de linker.
inline GLuint BuildTriangles()
{
    // Primeiro, definimos os atributos de cada vértice.
    GLfloat model_coefficients[] = {
    // Vértices de um cubo
    //    X      Y     Z     W
        -0.5f,  0.5f,  0.5f, 1.0f, // posição do vértice 0
        -0.5f, -0.5f,  0.5f, 1.0f, // posição do vértice 1
         0.5f, -0.5f,  0.5f, 1.0f, // posição do vértice 2
         0.5f,  0.5f,  0.5f, 1.0f, // posição do vértice 3
        -0.5f,  0.5f, -0.5f, 1.0f, // posição do vértice 4
        -0.5f, -0.5f, -0.5f, 1.0f, // posição do vértice 5
         0.5f, -0.5f, -0.5f, 1.0f, // posição do vértice 6
         0.5f,  0.5f, -0.5f, 1.0f, // posição do vértice 7
    // Vértices para desenhar o eixo X
         0.0f,  0.0f,  0.0f, 1.0f, // posição do vértice 8
         1.0f,  0.0f,  0.0f, 1.0f, // posição do vértice 9
    // Vértices para desenhar o eixo Y
         0.0f,  0.0f,  0.0f, 1.0f, // posição do vértice 10
         0.0f,  1.0f,  0.0f, 1.0f, // posição do vértice 11
    // Vértices para desenhar o eixo Z
         0.0f,  0.0f,  0.0f, 1.0f, // posição do vértice 12
         0.0f,  0.0f,  1.0f, 1.0f, // posição do vértice 13
    // Vértices para o chão (y = -0.5)
      -50.0f, -0.5f, -50.0f, 1.0f, // vértice 14
       50.0f, -0.5f, -50.0f, 1.0f, // vértice 15
       50.0f, -0.5f,  50.0f, 1.0f, // vértice 16
      -50.0f, -0.5f,  50.0f, 1.0f,  // vértice 17
    // Vértices para o cubo vermelho (mesmas posições do cubo original)
        -0.5f,  0.5f,  0.5f, 1.0f, // posição do vértice 18
        -0.5f, -0.5f,  0.5f, 1.0f, // posição do vértice 19
         0.5f, -0.5f,  0.5f, 1.0f, // posição do vértice 20
         0.5f,  0.5f,  0.5f, 1.0f, // posição do vértice 21
        -0.5f,  0.5f, -0.5f, 1.0f, // posição do vértice 22
        -0.5f, -0.5f, -0.5f, 1.0f, // posição do vértice 23
         0.5f, -0.5f, -0.5f, 1.0f, // posição do vértice 24
         0.5f,  0.5f, -0.5f, 1.0f,  // posição do vértice 25
    // Vértices para o cubo preto usado na picareta
        -0.5f,  0.5f,  0.5f, 1.0f, // posição do vértice 26
        -0.5f, -0.5f,  0.5f, 1.0f, // posição do vértice 27
         0.5f, -0.5f,  0.5f, 1.0f, // posição do vértice 28
         0.5f,  0.5f,  0.5f, 1.0f, // posição do vértice 29
        -0.5f,  0.5f, -0.5f, 1.0f, // posição do vértice 30
        -0.5f, -0.5f, -0.5f, 1.0f, // posição do vértice 31
         0.5f, -0.5f, -0.5f, 1.0f, // posição do vértice 32
         0.5f,  0.5f, -0.5f, 1.0f  // posição do vértice 33
    };

    GLuint VBO_model_coefficients_id;
    glGenBuffers(1, &VBO_model_coefficients_id);

    GLuint vertex_array_object_id;
    glGenVertexArrays(1, &vertex_array_object_id);

    glBindVertexArray(vertex_array_object_id);
    glBindBuffer(GL_ARRAY_BUFFER, VBO_model_coefficients_id);
    glBufferData(GL_ARRAY_BUFFER, sizeof(model_coefficients), NULL, GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(model_coefficients), model_coefficients);

    GLuint location = 0;
    GLint  number_of_dimensions = 4;
    glVertexAttribPointer(location, number_of_dimensions, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(location);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    GLfloat color_coefficients[] = {
    // Cores para o cubo vermelho
        0.0f, 0.5f, 1.0f, 1.0f, // cor do vértice 0
        0.0f, 0.5f, 1.0f, 1.0f, // cor do vértice 1
        0.0f, 0.5f, 1.0f, 1.0f, // cor do vértice 2
        0.0f, 0.5f, 1.0f, 1.0f, // cor do vértice 3
        0.0f, 0.5f, 1.0f, 1.0f, // cor do vértice 4
        0.0f, 0.5f, 1.0f, 1.0f, // cor do vértice 5
        0.0f, 0.5f, 1.0f, 1.0f, // cor do vértice 6
        0.0f, 0.5f, 1.0f, 1.0f, // cor do vértice 7
    // Cores para desenhar o eixo X
        1.0f, 0.0f, 0.0f, 1.0f, // cor do vértice 8
        1.0f, 0.0f, 0.0f, 1.0f, // cor do vértice 9
    // Cores para desenhar o eixo Y
        0.0f, 1.0f, 0.0f, 1.0f, // cor do vértice 10
        0.0f, 1.0f, 0.0f, 1.0f, // cor do vértice 11
    // Cores para desenhar o eixo Z
        0.0f, 0.0f, 1.0f, 1.0f, // cor do vértice 12
        0.0f, 0.0f, 1.0f, 1.0f, // cor do vértice 13
    // Cores para o chão (verde)
        0.3f, 0.7f, 0.2f, 1.0f, // cor do vértice 14
        0.3f, 0.7f, 0.2f, 1.0f, // cor do vértice 15
        0.3f, 0.7f, 0.2f, 1.0f, // cor do vértice 16
        0.3f, 0.7f, 0.2f, 1.0f,  // cor do vértice 17
    // Cores para o cubo vermelho
        1.0f, 0.0f, 0.0f, 1.0f, // cor do vértice 18
        1.0f, 0.0f, 0.0f, 1.0f, // cor do vértice 19
        1.0f, 0.0f, 0.0f, 1.0f, // cor do vértice 20
        1.0f, 0.0f, 0.0f, 1.0f, // cor do vértice 21
        1.0f, 0.0f, 0.0f, 1.0f, // cor do vértice 22
        1.0f, 0.0f, 0.0f, 1.0f, // cor do vértice 23
        1.0f, 0.0f, 0.0f, 1.0f, // cor do vértice 24
        1.0f, 0.0f, 0.0f, 1.0f,  // cor do vértice 25
    // Cores para o cubo preto
        0.1f, 0.1f, 0.1f, 1.0f, // cor do vértice 26
        0.1f, 0.1f, 0.1f, 1.0f, // cor do vértice 27
        0.1f, 0.1f, 0.1f, 1.0f, // cor do vértice 28
        0.1f, 0.1f, 0.1f, 1.0f, // cor do vértice 29
        0.1f, 0.1f, 0.1f, 1.0f, // cor do vértice 30
        0.1f, 0.1f, 0.1f, 1.0f, // cor do vértice 31
        0.1f, 0.1f, 0.1f, 1.0f, // cor do vértice 32
        0.1f, 0.1f, 0.1f, 1.0f  // cor do vértice 33
    };
    GLuint VBO_color_coefficients_id;
    glGenBuffers(1, &VBO_color_coefficients_id);
    glBindBuffer(GL_ARRAY_BUFFER, VBO_color_coefficients_id);
    glBufferData(GL_ARRAY_BUFFER, sizeof(color_coefficients), NULL, GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(color_coefficients), color_coefficients);
    location = 1;
    number_of_dimensions = 4;
    glVertexAttribPointer(location, number_of_dimensions, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(location);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    GLuint indices[] = {
    // FACES de um cubo
        0, 1, 2, 7, 6, 5, 3, 2, 6, 4, 0, 3, 4, 5, 1, 1, 5, 6,
        0, 2, 3, 7, 5, 4, 3, 6, 7, 4, 3, 7, 4, 1, 0, 1, 6, 2,
    // FACES do chão
        14, 15, 16, 14, 16, 17,
    // ARESTAS de um cubo
        0, 1, 1, 2, 2, 3, 3, 0, 0, 4, 4, 7, 7, 6, 6, 2, 6, 5, 5, 4, 5, 1, 7, 3,
    // Eixos X, Y, Z
        8 , 9 , 10, 11, 12, 13,
    // FACES do cubo vermelho (com offset de 18)
        18, 19, 20, 25, 24, 23, 21, 20, 24, 22, 18, 21, 22, 23, 19, 19, 23, 24,
        18, 20, 21, 25, 23, 22, 21, 24, 25, 22, 21, 25, 22, 19, 18, 19, 24, 20,
    // FACES do cubo preto (com offset de 26)
        26, 27, 28, 33, 32, 31, 29, 28, 32, 30, 26, 29, 30, 31, 27, 27, 31, 32,
        26, 28, 29, 33, 31, 30, 29, 32, 33, 30, 29, 33, 30, 27, 26, 27, 32, 28
    };

    SceneObject cube_faces;
    cube_faces.name           = "Cubo (faces coloridas)";
    cube_faces.first_index    = (void*)0;
    cube_faces.num_indices    = 36;
    cube_faces.rendering_mode = GL_TRIANGLES;
    cube_faces.bbox_min       = glm::vec4(-0.5f, -0.5f, -0.5f, 1.0f);
    cube_faces.bbox_max       = glm::vec4( 0.5f,  0.5f,  0.5f, 1.0f);
    g_VirtualScene["cube_faces"] = cube_faces;

    SceneObject floor_faces;
    floor_faces.name           = "Chão";
    floor_faces.first_index    = (void*)(36*sizeof(GLuint));
    floor_faces.num_indices    = 6;
    floor_faces.rendering_mode = GL_TRIANGLES;
    floor_faces.bbox_min       = glm::vec4(-50.0f, -0.5f, -50.0f, 1.0f);
    floor_faces.bbox_max       = glm::vec4( 50.0f, -0.5f,  50.0f, 1.0f);
    g_VirtualScene["floor"] = floor_faces;

    SceneObject cube_edges;
    cube_edges.name           = "Cubo (arestas pretas)";
    cube_edges.first_index    = (void*)((36+6)*sizeof(GLuint));
    cube_edges.num_indices    = 24;
    cube_edges.rendering_mode = GL_LINES;
    cube_edges.bbox_min       = glm::vec4(-0.5f, -0.5f, -0.5f, 1.0f);
    cube_edges.bbox_max       = glm::vec4( 0.5f,  0.5f,  0.5f, 1.0f);
    g_VirtualScene["cube_edges"] = cube_edges;

    SceneObject axes;
    axes.name           = "Eixos XYZ";
    axes.first_index    = (void*)((36+6+24)*sizeof(GLuint));
    axes.num_indices    = 6;
    axes.rendering_mode = GL_LINES;
    axes.bbox_min       = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
    axes.bbox_max       = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
    g_VirtualScene["axes"] = axes;

    SceneObject damaged_cube_faces;
    damaged_cube_faces.name           = "Cubo Danificado (faces)";
    damaged_cube_faces.first_index    = (void*)((36+6+24+6)*sizeof(GLuint));
    damaged_cube_faces.num_indices    = 36;
    damaged_cube_faces.rendering_mode = GL_TRIANGLES;
    damaged_cube_faces.bbox_min       = glm::vec4(-0.5f, -0.5f, -0.5f, 1.0f);
    damaged_cube_faces.bbox_max       = glm::vec4( 0.5f,  0.5f,  0.5f, 1.0f);
    g_VirtualScene["damaged_cube_faces"] = damaged_cube_faces;

    // Adiciona o novo objeto para o cubo preto
    SceneObject black_cube_faces;
    black_cube_faces.name           = "Cubo Preto (faces)";
    // O offset é a soma de todos os índices anteriores
    black_cube_faces.first_index    = (void*)((36+6+24+6+36)*sizeof(GLuint));
    black_cube_faces.num_indices    = 36;
    black_cube_faces.rendering_mode = GL_TRIANGLES;
    black_cube_faces.bbox_min       = glm::vec4(-0.5f, -0.5f, -0.5f, 1.0f);
    black_cube_faces.bbox_max       = glm::vec4( 0.5f,  0.5f,  0.5f, 1.0f);
    g_VirtualScene["black_cube_faces"] = black_cube_faces;

    GLuint indices_id;
    glGenBuffers(1, &indices_id);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indices_id);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), NULL, GL_STATIC_DRAW);
    glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, sizeof(indices), indices);

    glBindVertexArray(0);

    return vertex_array_object_id;
}

#endif // SCENE_BUILDER_H