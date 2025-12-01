//     Universidade Federal do Rio Grande do Sul
//             Instituto de Informática
//       Departamento de Informática Aplicada
//
//    INF01047 Fundamentos de Computação Gráfica
//               Prof. Eduardo Gastal
//
//                   Trabalho final
//

// Arquivos "headers" padrões de C podem ser incluídos em um
// programa C++, sendo necessário somente adicionar o caractere
// "c" antes de seu nome, e remover o sufixo ".h". Exemplo:
//    #include <stdio.h> // Em C
//  vira
//    #include <cstdio> // Em C++
//
#include <cmath>
#include <cstdio>
#include <cstdlib>

// Headers abaixo são específicos de C++
#include <map>
#include <string>
#include <limits>
#include <fstream>
#include <sstream>
#include <vector>

// Headers das bibliotecas OpenGL
#include <glad/glad.h>   // Criação de contexto OpenGL 3.3
#include <GLFW/glfw3.h>  // Criação de janelas do sistema operacional

// Headers da biblioteca GLM: criação de matrizes e vetores.
#include <glm/mat4x4.hpp>
#include <glm/vec4.hpp>
#include <glm/gtc/type_ptr.hpp>

// Headers locais, definidos na pasta "include/"
#include "utils.h"
#include "matrices.h"
#include "collisions.h"
#include "scene_builder.h" // Contém BuildTriangles(), SceneObject e g_VirtualScene

// Headers da biblioteca para carregar modelos obj
#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"

#define STB_IMAGE_IMPLEMENTATION 
#include <stb_image.h>

// Declaração de várias funções utilizadas em main().  Essas estão definidas
// logo após a definição de main() neste arquivo.
void LoadShadersFromFiles(); // Carrega os shaders de vértice e fragmento, criando um programa de GPU
GLuint LoadShader_Vertex(const char* filename);   // Carrega um vertex shader
GLuint LoadShader_Fragment(const char* filename); // Carrega um fragment shader
void LoadShader(const char* filename, GLuint shader_id); // Função utilizada pelas duas acima
GLuint CreateGpuProgram(GLuint vertex_shader_id, GLuint fragment_shader_id); // Cria um programa de GPU
GLuint LoadTextureImage(const char* filename); // Função que carrega imagens de textura

// Declaração de funções auxiliares para renderizar texto dentro da janela
// OpenGL. Estas funções estão definidas no arquivo "textrendering.cpp".
void TextRendering_Init();
float TextRendering_LineHeight(GLFWwindow* window);
float TextRendering_CharWidth(GLFWwindow* window);
void TextRendering_PrintString(GLFWwindow* window, const std::string &str, float x, float y, float scale = 1.0f);
void TextRendering_PrintMatrix(GLFWwindow* window, glm::mat4 M, float x, float y, float scale = 1.0f);
void TextRendering_PrintVector(GLFWwindow* window, glm::vec4 v, float x, float y, float scale = 1.0f);
void TextRendering_PrintMatrixVectorProduct(GLFWwindow* window, glm::mat4 M, glm::vec4 v, float x, float y, float scale = 1.0f);
void TextRendering_PrintMatrixVectorProductMoreDigits(GLFWwindow* window, glm::mat4 M, glm::vec4 v, float x, float y, float scale = 1.0f);
void TextRendering_PrintMatrixVectorProductDivW(GLFWwindow* window, glm::mat4 M, glm::vec4 v, float x, float y, float scale = 1.0f);

// Funções abaixo renderizam como texto na janela OpenGL algumas matrizes e
// outras informações do programa. Definidas após main().
void TextRendering_ShowModelViewProjection(GLFWwindow* window, glm::mat4 projection, glm::mat4 view, glm::mat4 model, glm::vec4 p_model);
void TextRendering_ShowEulerAngles(GLFWwindow* window);
void TextRendering_ShowProjection(GLFWwindow* window);
void TextRendering_ShowFramesPerSecond(GLFWwindow* window);

// Funções callback para comunicação com o sistema operacional e interação do
// usuário. Veja mais comentários nas definições das mesmas, abaixo.
void FramebufferSizeCallback(GLFWwindow* window, int width, int height);
void ErrorCallback(int error, const char* description);
void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mode);
void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
void CursorPosCallback(GLFWwindow* window, double xpos, double ypos);
void ScrollCallback(GLFWwindow* window, double xoffset, double yoffset);

// Estrutura ObjModel (Copiada do Lab 5)
struct ObjModel
{
    tinyobj::attrib_t                 attrib;
    std::vector<tinyobj::shape_t>     shapes;
    std::vector<tinyobj::material_t>  materials;

    ObjModel(const char* filename, const char* basepath = NULL, bool triangulate = true)
    {
        printf("Carregando objetos do arquivo \"%s\"...\n", filename);

        std::string fullpath(filename);
        std::string dirname;
        if (basepath == NULL)
        {
            auto i = fullpath.find_last_of("/");
            if (i != std::string::npos)
            {
                dirname = fullpath.substr(0, i+1);
                basepath = dirname.c_str();
            }
        }

        std::string warn;
        std::string err;
        bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, filename, basepath, triangulate);

        if (!err.empty())
            fprintf(stderr, "\n%s\n", err.c_str());

        if (!ret)
            throw std::runtime_error("Erro ao carregar modelo.");

        for (size_t shape = 0; shape < shapes.size(); ++shape)
        {
            if (shapes[shape].name.empty())
            {
                fprintf(stderr, "Erro: Objeto sem nome dentro do arquivo '%s'.\n", filename);
                throw std::runtime_error("Objeto sem nome.");
            }
            printf("- Objeto '%s'\n", shapes[shape].name.c_str());
        }
        printf("OK.\n");
    }
};

// Declaração das funções do Lab 5 que vamos adicionar
void ComputeNormals(ObjModel* model);
void BuildTrianglesAndAddToVirtualScene(ObjModel* model);

// Função que computa as normais (Copiada do Lab 5)
void ComputeNormals(ObjModel* model)
{
    if ( !model->attrib.normals.empty() )
        return;

    size_t num_vertices = model->attrib.vertices.size() / 3;

    std::vector<int> num_triangles_per_vertex(num_vertices, 0);
    std::vector<glm::vec4> vertex_normals(num_vertices, glm::vec4(0.0f,0.0f,0.0f,0.0f));

    for (size_t shape = 0; shape < model->shapes.size(); ++shape)
    {
        size_t num_triangles = model->shapes[shape].mesh.num_face_vertices.size();

        for (size_t triangle = 0; triangle < num_triangles; ++triangle)
        {
            glm::vec4  vertices[3];
            for (size_t vertex = 0; vertex < 3; ++vertex)
            {
                tinyobj::index_t idx = model->shapes[shape].mesh.indices[3*triangle + vertex];
                const float vx = model->attrib.vertices[3*idx.vertex_index + 0];
                const float vy = model->attrib.vertices[3*idx.vertex_index + 1];
                const float vz = model->attrib.vertices[3*idx.vertex_index + 2];
                vertices[vertex] = glm::vec4(vx,vy,vz,1.0);
            }

            const glm::vec4  a = vertices[0];
            const glm::vec4  b = vertices[1];
            const glm::vec4  c = vertices[2];

            const glm::vec4  n = crossproduct(b-a,c-a);

            for (size_t vertex = 0; vertex < 3; ++vertex)
            {
                tinyobj::index_t idx = model->shapes[shape].mesh.indices[3*triangle + vertex];
                num_triangles_per_vertex[idx.vertex_index] += 1;
                vertex_normals[idx.vertex_index] += n;
            }
        }
    }

    model->attrib.normals.resize( 3*num_vertices );

    for (size_t i = 0; i < vertex_normals.size(); ++i)
    {
        glm::vec4 n = vertex_normals[i] / (float)num_triangles_per_vertex[i];
        n /= norm(n);
        model->attrib.normals[3*i + 0] = n.x;
        model->attrib.normals[3*i + 1] = n.y;
        model->attrib.normals[3*i + 2] = n.z;
    }
}

// Constrói triângulos e adiciona à cena virtual (Adaptada do Lab 5)
void BuildTrianglesAndAddToVirtualScene(ObjModel* model)
{
    GLuint vertex_array_object_id;
    glGenVertexArrays(1, &vertex_array_object_id);
    glBindVertexArray(vertex_array_object_id);

    std::vector<GLuint> indices;
    std::vector<float>  model_coefficients;
    std::vector<float>  normal_coefficients;
    std::vector<float>  texture_coefficients;

    for (size_t shape = 0; shape < model->shapes.size(); ++shape)
    {
        size_t first_index = indices.size();
        size_t num_triangles = model->shapes[shape].mesh.num_face_vertices.size();

        const float minval = std::numeric_limits<float>::min();
        const float maxval = std::numeric_limits<float>::max();

        glm::vec3 bbox_min = glm::vec3(maxval,maxval,maxval);
        glm::vec3 bbox_max = glm::vec3(minval,minval,minval);

        for (size_t triangle = 0; triangle < num_triangles; ++triangle)
        {
            for (size_t vertex = 0; vertex < 3; ++vertex)
            {
                tinyobj::index_t idx = model->shapes[shape].mesh.indices[3*triangle + vertex];

                indices.push_back(first_index + 3*triangle + vertex);

                const float vx = model->attrib.vertices[3*idx.vertex_index + 0];
                const float vy = model->attrib.vertices[3*idx.vertex_index + 1];
                const float vz = model->attrib.vertices[3*idx.vertex_index + 2];
                
                model_coefficients.push_back( vx ); // X
                model_coefficients.push_back( vy ); // Y
                model_coefficients.push_back( vz ); // Z
                model_coefficients.push_back( 1.0f ); // W

                bbox_min.x = std::min(bbox_min.x, vx);
                bbox_min.y = std::min(bbox_min.y, vy);
                bbox_min.z = std::min(bbox_min.z, vz);
                bbox_max.x = std::max(bbox_max.x, vx);
                bbox_max.y = std::max(bbox_max.y, vy);
                bbox_max.z = std::max(bbox_max.z, vz);

                // Se existirem normais, carregamos elas
                if ( idx.normal_index != -1 )
                {
                    const float nx = model->attrib.normals[3*idx.normal_index + 0];
                    const float ny = model->attrib.normals[3*idx.normal_index + 1];
                    const float nz = model->attrib.normals[3*idx.normal_index + 2];
                    normal_coefficients.push_back( nx ); // X
                    normal_coefficients.push_back( ny ); // Y
                    normal_coefficients.push_back( nz ); // Z
                    normal_coefficients.push_back( 0.0f ); // W
                }
            }
        }

        SceneObject theobject;
        theobject.name           = model->shapes[shape].name;
        theobject.first_index    = first_index; 
        theobject.num_indices    = indices.size() - first_index; 
        theobject.rendering_mode = GL_TRIANGLES;
        theobject.vertex_array_object_id = vertex_array_object_id;
        theobject.bbox_min = bbox_min;
        theobject.bbox_max = bbox_max;

        g_VirtualScene[model->shapes[shape].name] = theobject;
    }

    GLuint VBO_model_coefficients_id;
    glGenBuffers(1, &VBO_model_coefficients_id);
    glBindBuffer(GL_ARRAY_BUFFER, VBO_model_coefficients_id);
    glBufferData(GL_ARRAY_BUFFER, model_coefficients.size() * sizeof(float), NULL, GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, model_coefficients.size() * sizeof(float), model_coefficients.data());
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(0);

    if ( !normal_coefficients.empty() )
    {
        GLuint VBO_normal_coefficients_id;
        glGenBuffers(1, &VBO_normal_coefficients_id);
        glBindBuffer(GL_ARRAY_BUFFER, VBO_normal_coefficients_id);
        glBufferData(GL_ARRAY_BUFFER, normal_coefficients.size() * sizeof(float), NULL, GL_STATIC_DRAW);
        glBufferSubData(GL_ARRAY_BUFFER, 0, normal_coefficients.size() * sizeof(float), normal_coefficients.data());
        
        // ADAPTAÇÃO: Usamos location = 1 para as normais
        // Isso permite que o shader atual use as normais como "cor"
        glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 0, 0);
        glEnableVertexAttribArray(1);
    }

    GLuint indices_id;
    glGenBuffers(1, &indices_id);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indices_id);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), NULL, GL_STATIC_DRAW);
    glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, indices.size() * sizeof(GLuint), indices.data());

    glBindVertexArray(0);
}

GLuint g_NumLoadedTextures = 0;

// Função que carrega uma imagem para ser utilizada como textura
GLuint LoadTextureImage(const char* filename)
{
    printf("Carregando imagem \"%s\"... ", filename);

    // Primeiro fazemos a leitura da imagem do disco
    stbi_set_flip_vertically_on_load(true);
    int width;
    int height;
    int channels;
    unsigned char *data = stbi_load(filename, &width, &height, &channels, 3);

    if ( data == NULL )
    {
        fprintf(stderr, "ERROR: Cannot open image file \"%s\".\n", filename);
        std::exit(EXIT_FAILURE);
    }

    printf("OK (%dx%d).\n", width, height);

    // Agora criamos objetos na GPU com OpenGL para armazenar a textura
    GLuint texture_id;
    GLuint sampler_id;
    glGenTextures(1, &texture_id);
    glGenSamplers(1, &sampler_id);

    // Veja slides 95-96 do documento Aula_20_Mapeamento_de_Texturas.pdf
    glSamplerParameteri(sampler_id, GL_TEXTURE_WRAP_S, GL_REPEAT); 
    glSamplerParameteri(sampler_id, GL_TEXTURE_WRAP_T, GL_REPEAT);

    // Parâmetros de amostragem da textura.
    glSamplerParameteri(sampler_id, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glSamplerParameteri(sampler_id, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // Agora enviamos a imagem lida do disco para a GPU
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
    glPixelStorei(GL_UNPACK_SKIP_PIXELS, 0);
    glPixelStorei(GL_UNPACK_SKIP_ROWS, 0);

    GLuint textureunit = g_NumLoadedTextures;
    glActiveTexture(GL_TEXTURE0 + textureunit);
    glBindTexture(GL_TEXTURE_2D, texture_id);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_SRGB8, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);
    glBindSampler(textureunit, sampler_id);

    stbi_image_free(data);

    g_NumLoadedTextures += 1;

    return texture_id; 
}


// Razão de proporção da janela (largura/altura). Veja função FramebufferSizeCallback().
float g_ScreenRatio = 1.0f;

// Ângulos de Euler que controlam a rotação de um dos cubos da cena virtual
float g_AngleX = 0.0f;
float g_AngleY = 0.0f;
float g_AngleZ = 0.0f;

// "g_LeftMouseButtonPressed = true" se o usuário está com o botão esquerdo do mouse
// pressionado no momento atual. Veja função MouseButtonCallback().
bool g_LeftMouseButtonPressed = false;

bool g_UseFreeCamera = true;
bool g_ShowCeiling = true; // Controla visibilidade do teto

// Variáveis que definem a câmera em coordenadas esféricas, controladas pelo
// usuário através do mouse (veja função CursorPosCallback()). A posição
// efetiva da câmera é calculada dentro da função main(), dentro do loop de
// renderização.
float g_CameraTheta = 0.0f; // Ângulo no plano ZX em relação ao eixo Z
float g_CameraPhi = 0.0f;   // Ângulo em relação ao eixo Y
float g_CameraDistance = 2.5f; // Distância da câmera para a origem

glm::vec4 g_CameraPosition     = glm::vec4(-5.0f, 0.0f, -5.0f, 1.0f); // Posição inicial da câmera
glm::vec4 g_CameraViewVector   = glm::vec4(0.0f, 0.0f, -1.0f, 0.0f); // Vetor "view", para onde a câmera está virada
glm::vec4 g_CameraUpVector     = glm::vec4(0.0f, 1.0f, 0.0f, 0.0f); // Vetor "up"

// Ângulos para controle do mouse
float g_CameraYaw   = -90.0f; // Yaw é rotação em torno do eixo Y. -90 graus para apontar para -Z.
float g_CameraPitch = 0.0f;   // Pitch é rotação em torno do eixo X.

// Variáveis para controle de movimento
bool g_W_pressed = false;
bool g_S_pressed = false;
bool g_A_pressed = false;
bool g_D_pressed = false;

// Variável que controla o tipo de projeção utilizada: perspectiva ou ortográfica.
bool g_UsePerspectiveProjection = true;

// Variável que controla se o texto informativo será mostrado na tela.
bool g_ShowInfoText = true;

// Variáveis que definem um programa de GPU (shaders). Veja função LoadShadersFromFiles().
GLuint g_GpuProgramID = 0;

// Variáveis para animação da picareta
bool g_IsSwinging = false;         // Controla se a animação está ativa
float g_SwingAnimationTime = 0.0f; // Controla o progresso da animação (t de 0 a 1)

// Função para calcular ponto em curva de Bézier Cúbica
// c(t) = (1-t)^3 * P0 + 3t(1-t)^2 * P1 + 3t^2(1-t) * P2 + t^3 * P3
glm::vec3 BezierCubic(float t, glm::vec3 P0, glm::vec3 P1, glm::vec3 P2, glm::vec3 P3)
{
    glm::vec3 point = (1.0f - t) * (1.0f - t) * (1.0f - t) * P0;              // (1-t)^3 * P0
    point += 3.0f * t * (1.0f - t) * (1.0f - t) * P1;             // 3t(1-t)^2 * P1
    point += 3.0f * t * t * (1.0f - t) * P2;             // 3t^2(1-t) * P2
    point += t * t * t * P3;                        // t^3 * P3
    
    return point;
}

// Pontos de controle para a animação da picareta (x = ângulo X, y = ângulo Y, z = posição Z)
// P0 - repouso, P1 começa, P2 continua movimento, P3 posição final
const glm::vec3 g_BezierP0 = glm::vec3(0.0f, 0.0f, 0.0f);       // Repouso
const glm::vec3 g_BezierP1 = glm::vec3(-30.0f, 10.0f, 0.1f);   // Levanta e puxa para trás
const glm::vec3 g_BezierP2 = glm::vec3(-70.0f, -5.0f, -0.1f);  // Prepara o golpe
const glm::vec3 g_BezierP3 = glm::vec3(-90.0f, 0.0f, -0.2f);   // Golpe final

// Para animação e física independentes de framerate
float g_DeltaTime = 0.0f;
float g_LastFrameTime = 0.0f;

GLuint g_TextureIdStone = 0;
GLuint g_TextureIdGrass = 0;
GLuint g_TextureIdWood = 0;
GLuint g_TextureIdCobblestone = 0;
GLuint g_TextureIdGravelstones = 0;
GLuint g_textureIdGraystonse = 0;
GLuint g_TextureIdDiamond = 0; // Textura do diamante


int main()
{
    // Inicializamos a biblioteca GLFW, utilizada para criar uma janela do
    // sistema operacional, onde poderemos renderizar com OpenGL.
    int success = glfwInit();
    if (!success)
    {
        fprintf(stderr, "ERROR: glfwInit() failed.\n");
        std::exit(EXIT_FAILURE);
    }

    // Definimos o callback para impressão de erros da GLFW no terminal
    glfwSetErrorCallback(ErrorCallback);

    // Pedimos para utilizar OpenGL versão 3.3 (ou superior)
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);

    #ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    #endif

    // Pedimos para utilizar o perfil "core", isto é, utilizaremos somente as
    // funções modernas de OpenGL.
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Criamos uma janela do sistema operacional, com 800 colunas e 800 linhas
    // de pixels, e com título "INF01047 ...".
    GLFWwindow* window;
    window = glfwCreateWindow(800, 800, "INF01047 - Trabalho Final - Felipe e Adão", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        fprintf(stderr, "ERROR: glfwCreateWindow() failed.\n");
        std::exit(EXIT_FAILURE);
    }

    // Definimos a função de callback que será chamada sempre que o usuário
    // pressionar alguma tecla do teclado ...
    glfwSetKeyCallback(window, KeyCallback);
    // ... ou clicar os botões do mouse ...
    glfwSetMouseButtonCallback(window, MouseButtonCallback);
    // ... ou movimentar o cursor do mouse em cima da janela ...
    glfwSetCursorPosCallback(window, CursorPosCallback);
    // ... ou rolar a "rodinha" do mouse.
    glfwSetScrollCallback(window, ScrollCallback);

    // Definimos a função de callback que será chamada sempre que a janela for
    // redimensionada, por consequência alterando o tamanho do "framebuffer"
    // (região de memória onde são armazenados os pixels da imagem).
    glfwSetFramebufferSizeCallback(window, FramebufferSizeCallback);
    glfwSetWindowSize(window, 800, 800); // Forçamos a chamada do callback acima, para definir g_ScreenRatio.

	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // Indicamos que as chamadas OpenGL deverão renderizar nesta janela
    glfwMakeContextCurrent(window);

    // Carregamento de todas funções definidas por OpenGL 3.3, utilizando a
    // biblioteca GLAD.
    gladLoadGLLoader((GLADloadproc) glfwGetProcAddress);

    // Imprimimos no terminal informações sobre a GPU do sistema
    const GLubyte *vendor      = glGetString(GL_VENDOR);
    const GLubyte *renderer    = glGetString(GL_RENDERER);
    const GLubyte *glversion   = glGetString(GL_VERSION);
    const GLubyte *glslversion = glGetString(GL_SHADING_LANGUAGE_VERSION);

    printf("GPU: %s, %s, OpenGL %s, GLSL %s\n", vendor, renderer, glversion, glslversion);

    // Carregamos os shaders de vértices e de fragmentos que serão utilizados
    // para renderização. Veja slides 180-200 do documento Aula_03_Rendering_Pipeline_Grafico.pdf.
    //
    LoadShadersFromFiles();

    // Construímos a representação de um triângulo
    GLuint vertex_array_object_id = BuildTriangles();

    // Inicializamos o código para renderização de texto.
    TextRendering_Init();

    // CARREGAMENTO DO OBJ (Picareta) USANDO CLASSE DO LAB 5
    ObjModel pickaxeModel("../../src/pickaxe.obj"); 
    ComputeNormals(&pickaxeModel);
    BuildTrianglesAndAddToVirtualScene(&pickaxeModel);

    // CARREGAMENTO DO OBJ (Diamante)
    ObjModel diamondModel("../../data/obj/diamond_obj.obj"); 
    ComputeNormals(&diamondModel);
    BuildTrianglesAndAddToVirtualScene(&diamondModel);
    
    // Debug: mostra os nomes dos objetos carregados
    printf("Objetos na cena virtual:\n");
    for (auto& pair : g_VirtualScene) {
        printf("  - '%s'\n", pair.first.c_str());
    }

    // Buscamos o endereço das variáveis definidas dentro do Vertex Shader.
    // Utilizaremos estas variáveis para enviar dados para a placa de vídeo
    // (GPU)! Veja arquivo "shader_vertex.glsl".
    GLint model_uniform           = glGetUniformLocation(g_GpuProgramID, "model"); // Variável da matriz "model"
    GLint view_uniform            = glGetUniformLocation(g_GpuProgramID, "view"); // Variável da matriz "view" em shader_vertex.glsl
    GLint projection_uniform      = glGetUniformLocation(g_GpuProgramID, "projection"); // Variável da matriz "projection" em shader_vertex.glsl
    GLint render_as_black_uniform = glGetUniformLocation(g_GpuProgramID, "render_as_black"); // Variável booleana em shader_vertex.glsl

    // Habilitamos o Z-buffer. Veja slides 104-116 do documento Aula_09_Projecoes.pdf.
    glEnable(GL_DEPTH_TEST);

    // Variáveis auxiliares utilizadas para chamada à função
    // TextRendering_ShowModelViewProjection(), armazenando matrizes 4x4.
    glm::mat4 the_projection;
    glm::mat4 the_model;
    glm::mat4 the_view;

    

    // Inicializa o g_LastFrameTime antes do loop
    g_LastFrameTime = (float)glfwGetTime();

    g_TextureIdStone = LoadTextureImage("../../data/425.jpg"); // Textura 0 (Paredes)
    g_TextureIdGrass = LoadTextureImage("../../data/grass.jpg"); // Textura 1 (Chão)
    g_TextureIdWood = LoadTextureImage("../../data/wood.jpg"); // Textura 2 (Picareta)
    g_TextureIdGravelstones = LoadTextureImage("../../data/gravelstones.jpg"); // Textura 3 (Chão de Pedra)
    g_textureIdGraystonse = LoadTextureImage("../../data/grayrocks.jpg"); // Textura 4 (Teto)
    g_TextureIdDiamond = LoadTextureImage("../../data/obj/diamond_obj.png"); // Textura 5 (Diamante)

    // Ficamos em um loop infinito, renderizando, até que o usuário feche a janela
    while (!glfwWindowShouldClose(window))
    {

        // Cálculo do Delta Time
        float currentFrameTime = (float)glfwGetTime();
        g_DeltaTime = currentFrameTime - g_LastFrameTime;
        g_LastFrameTime = currentFrameTime;

        // Atualização da Animação
        if (g_IsSwinging)
        {
            float swing_speed = 3.0f; // Velocidade da animação (unidades/seg)
            g_SwingAnimationTime += swing_speed * g_DeltaTime;

            // Animação vai de 0 a 2:
            // - De 0 a 1: ida (movimento de golpe)
            // - De 1 a 2: volta (retorno à posição inicial)
            if (g_SwingAnimationTime >= 2.0f)
            {
                g_IsSwinging = false;
                g_SwingAnimationTime = 0.0f;
            }
        }


        // Aqui executamos as operações de renderização

        // Definimos a cor do "fundo" do framebuffer como branco.  Tal cor é
        // definida como coeficientes RGBA: Red, Green, Blue, Alpha; isto é:
        // Vermelho, Verde, Azul, Alpha (valor de transparência).
        // Conversaremos sobre sistemas de cores nas aulas de Modelos de Iluminação.
        //
        //           R     G     B     A
        glClearColor(1.0f, 1.0f, 1.0f, 1.0f);

        // "Pintamos" todos os pixels do framebuffer com a cor definida acima,
        // e também resetamos todos os pixels do Z-buffer (depth buffer).
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Pedimos para a GPU utilizar o programa de GPU criado acima (contendo
        // os shaders de vértice e fragmentos).
        glUseProgram(g_GpuProgramID);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, g_TextureIdStone); // Unidade 0 = Pedra Parede

        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, g_TextureIdGravelstones); // Unidade 1 = Pedregulhos

        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, g_TextureIdWood);  // Unidade 2 = Madeira

        glActiveTexture(GL_TEXTURE3);
        glBindTexture(GL_TEXTURE_2D, g_textureIdGraystonse);  // Unidade 3 = Teto

        glActiveTexture(GL_TEXTURE4);
        glBindTexture(GL_TEXTURE_2D, g_TextureIdDiamond);  // Unidade 4 = Diamante

        // "Ligamos" o VAO. Informamos que queremos utilizar os atributos de
        // vértices apontados pelo VAO criado pela função BuildTriangles(). Veja
        // comentários detalhados dentro da definição de BuildTriangles().
        glBindVertexArray(vertex_array_object_id);

        // Computamos a posição da câmera utilizando coordenadas esféricas.  As
        // variáveis g_CameraDistance, g_CameraPhi, e g_CameraTheta são
        // controladas pelo mouse do usuário. Veja as funções CursorPosCallback()
        // e ScrollCallback().

        /*
        float r = g_CameraDistance;
        float y = r*sin(g_CameraPhi);
        float z = r*cos(g_CameraPhi)*cos(g_CameraTheta);
        float x = r*cos(g_CameraPhi)*sin(g_CameraTheta);

        // Abaixo definimos as varáveis que efetivamente definem a câmera virtual.
        // Veja slides 195-227 e 229-234 do documento Aula_08_Sistemas_de_Coordenadas.pdf.
        glm::vec4 camera_position_c  = glm::vec4(x,y,z,1.0f); // Ponto "c", centro da câmera
        glm::vec4 camera_lookat_l    = glm::vec4(0.0f,0.0f,0.0f,1.0f); // Ponto "l", para onde a câmera (look-at) estará sempre olhando
        glm::vec4 camera_view_vector = camera_lookat_l - camera_position_c; // Vetor "view", sentido para onde a câmera está virada
        glm::vec4 camera_up_vector   = glm::vec4(0.0f,1.0f,0.0f,0.0f); // Vetor "up" fixado para apontar para o "céu" (eito Y global)

        // Computamos a matriz "View" utilizando os parâmetros da câmera para
        // definir o sistema de coordenadas da câmera.  Veja slides 2-14, 184-190 e 236-242 do documento Aula_08_Sistemas_de_Coordenadas.pdf.
        glm::mat4 view = Matrix_Camera_View(camera_position_c, camera_view_vector, camera_up_vector);
        */

        //Lógica para camera livre
        //float camera_speed = 2.0f * 0.016f;
        //Usamos g_DeltaTime para velocidade constante
        float move_speed = 4.0f * g_DeltaTime;

        // Calcular vetores de direção SEM componente Y
        //glm::vec4 forward_vector = glm::vec4(g_CameraViewVector.x, 0.0f, g_CameraViewVector.z, 0.0f);
        
        glm::vec4 forward_vector;

        if (g_UseFreeCamera)
        {
            // Modo FPS: Usa o vetor de visão da câmera
            forward_vector = glm::vec4(g_CameraViewVector.x, 0.0f, g_CameraViewVector.z, 0.0f);
        }
        else
        {
            // Modo Look-At: Usa o ângulo Yaw do personagem
            // Isso garante que ele ande na direção que está virado
            float angle = glm::radians(g_CameraYaw);
            
            // Calculamos o vetor (X, Z) baseado no ângulo
            // Nota: Se o personagem andar de costas, inverta os sinais (-cos, -sin)
            forward_vector = glm::vec4(-cos(angle), 0.0f, sin(angle), 0.0f);
        }
        
        
        if (length(forward_vector) > 0.0f) // Evita divisão por zero
            forward_vector = normalize(forward_vector);
        
        // Vetor Direita (Right) - Já é paralelo ao chão geralmente, mas garantimos
        glm::vec4 right_vector = crossproduct(forward_vector, glm::vec4(0.0f, 1.0f, 0.0f, 0.0f));

        // Acumular a intenção de movimento
        glm::vec4 intended_movement = glm::vec4(0.0f, 0.0f, 0.0f, 0.0f);

        if (g_W_pressed) intended_movement += forward_vector;
        if (g_S_pressed) intended_movement -= forward_vector;
        if (g_D_pressed) intended_movement += right_vector;
        if (g_A_pressed) intended_movement -= right_vector;

        // Se houver movimento, aplicamos com verificação de colisão
        if (length(intended_movement) > 0.0f)
        {
            // Normaliza para que andar na diagonal não seja mais rápido
            intended_movement = normalize(intended_movement) * move_speed;

            // Tentativa de movimento no eixo X
            float new_x = g_CameraPosition.x + intended_movement.x;
            if (!CheckCollision(new_x, g_CameraPosition.z))
            {
                g_CameraPosition.x = new_x;
            }

            // Tentativa de movimento no eixo Z
            float new_z = g_CameraPosition.z + intended_movement.z;
            if (!CheckCollision(g_CameraPosition.x, new_z))
            {
                g_CameraPosition.z = new_z;
            }
            
            // Forçamos Y a ser sempre 0.0 (altura dos olhos ajustada se necessário)
            // Se quiser que a câmera fique na altura de uma pessoa (ex: 0.0 é o chão), 
            // você pode somar uma constante na View Matrix ou manter g_CameraPosition.y fixo aqui.
            g_CameraPosition.y = 0.0f; 
        }

        glm::mat4 view;        

        if (g_UseFreeCamera)
        {
            // Modo FPS: A câmera está na posição do jogador e olha para g_CameraViewVector
            // Adicionamos +0.5 no Y para simular a altura dos olhos do boneco
            glm::vec4 camera_eye = g_CameraPosition + glm::vec4(0.0f, 0.0f, 0.0f, 0.0f);
            
            view = Matrix_Camera_View(camera_eye, g_CameraViewVector, g_CameraUpVector);
        }
        else
        {
            // Modo Look-At: A câmera orbita ao redor do jogador
            float r = g_CameraDistance;
            float y = r * sin(g_CameraPhi);
            float z = r * cos(g_CameraPhi) * cos(g_CameraTheta);
            float x = r * cos(g_CameraPhi) * sin(g_CameraTheta);

            // Posição do alvo (Cabeça do jogador)
            glm::vec4 camera_lookat_point = g_CameraPosition + glm::vec4(0.0f, 1.0f, 0.0f, 0.0f); 

            // Posição da câmera (Deslocada em relação ao alvo)
            glm::vec4 camera_view_point = camera_lookat_point + glm::vec4(x, y, z, 0.0f);

            // Vetor View: De onde a câmera está -> para onde ela olha
            glm::vec4 view_vector = camera_lookat_point - camera_view_point;

            view = Matrix_Camera_View(camera_view_point, view_vector, g_CameraUpVector);
        }

        the_view = view;
        
        glm::mat4 projection;
        float nearplane = -0.1f; 
        float farplane  = -20.0f; 

        if (g_UsePerspectiveProjection)
        {
            float field_of_view = 3.141592 / 3.0f;
            projection = Matrix_Perspective(field_of_view, g_ScreenRatio, nearplane, farplane);
        }
        else
        {
            float t = 1.5f*g_CameraDistance/2.5f;
            float b = -t;
            float r = t*g_ScreenRatio;
            float l = -r;
            projection = Matrix_Orthographic(l, r, b, t, nearplane, farplane);
        }

        glUniformMatrix4fv(view_uniform       , 1 , GL_FALSE , glm::value_ptr(view));
        glUniformMatrix4fv(projection_uniform , 1 , GL_FALSE , glm::value_ptr(projection));

        // Desenha o chão
        {
            glm::mat4 model = Matrix_Identity();
            the_model = model; // Salva para o texto

            glUniformMatrix4fv(model_uniform, 1, GL_FALSE, glm::value_ptr(model));
            glUniform1i(glGetUniformLocation(g_GpuProgramID, "object_id"), 1); 

            glDrawElements(
                g_VirtualScene["floor"].rendering_mode,
                g_VirtualScene["floor"].num_indices,
                GL_UNSIGNED_INT,
                (void*)g_VirtualScene["floor"].first_index
            );
        }

        // Desenha o teto (apenas se g_ShowCeiling for true)
        if (g_ShowCeiling)
        {
            glm::mat4 model = Matrix_Identity();

            glUniformMatrix4fv(model_uniform, 1, GL_FALSE, glm::value_ptr(model));
            glUniform1i(glGetUniformLocation(g_GpuProgramID, "object_id"), 3); // ID 3 = Teto

            glDrawElements(
                g_VirtualScene["ceiling"].rendering_mode,
                g_VirtualScene["ceiling"].num_indices,
                GL_UNSIGNED_INT,
                (void*)g_VirtualScene["ceiling"].first_index
            );
        }

        // Desenha os diamantes baseado no mapa
        {
            // Rotação contínua baseada no tempo
            float diamond_rotation = (float)glfwGetTime() * 1.0f; // 1 rad/s

            for (int z = 0; z < MAP_HEIGHT; ++z)
            {
                for (int x = 0; x < MAP_WIDTH; ++x)
                {
                    if (maze_map[z][x] == DIAMOND)
                    {
                        // Calcula a posição no mundo, centrando o labirinto em (0,0)
                        float pos_x = (float)x - (float)MAP_WIDTH / 2.0f;
                        float pos_z = (float)z - (float)MAP_HEIGHT / 2.0f;

                        // Posição do diamante (um pouco acima do chão)
                        // Rotação X de 90 graus para ficar em pé
                        glm::mat4 model = Matrix_Translate(pos_x, 0.2f, pos_z)
                                        * Matrix_Rotate_Y(diamond_rotation)
                                        // Gira e deixa em pé
                                        * Matrix_Rotate_X(glm::radians(-90.0f))
                                        // Diminuiu o tamanho do diamante
                                        * Matrix_Scale(0.007f, 0.007f, 0.007f);

                        glUniformMatrix4fv(model_uniform, 1, GL_FALSE, glm::value_ptr(model));
                        glUniform1i(glGetUniformLocation(g_GpuProgramID, "object_id"), 4);
                        glUniform1i(render_as_black_uniform, false);

                        // Desenha as partes do diamante
                        // Procura pelo objeto do diamante na cena virtual
                        if (g_VirtualScene.count("rdmobj00")) 
                        {
                            SceneObject &obj = g_VirtualScene["rdmobj00"]; 
                            glBindVertexArray(obj.vertex_array_object_id);
                            glDrawElements(
                                obj.rendering_mode,
                                obj.num_indices,
                                GL_UNSIGNED_INT,
                                (void*)(obj.first_index * sizeof(GLuint))
                            );
                        }
                    }
                }
            }
            // Restaura o VAO do cenário
            glBindVertexArray(vertex_array_object_id);
        }

        // Desenha o labirinto com base no maze_map
        // (Substitui o loop for (int i = 1; i <= 3; ++i))
        for (int z = 0; z < MAP_HEIGHT; ++z)
        {
            for (int x = 0; x < MAP_WIDTH; ++x)
            {
                // Se for uma parede (WALL ou DAMAGED_WALL)
                if (maze_map[z][x] == WALL || maze_map[z][x] == DAMAGED_WALL)
                {
                    // Os cubos têm 1x1x1 (de -0.5 a 0.5).
                    // O chão está em y = -0.5.
                    // Queremos que o cubo fique "em cima" do chão.
                    // Um translate(x, 0.0, z) posiciona o *centro* do cubo em y=0.0,
                    // fazendo-o ocupar de y=-0.5 a y=0.5.

                    // Calcula a posição no mundo, centrando o labirinto em (0,0)
                    float pos_x = (float)x - (float)MAP_WIDTH / 2.0f;
                    float pos_z = (float)z - (float)MAP_HEIGHT / 2.0f;

                    // Loop para empilhar 3 blocos verticalmente
                    for (int y = 0; y < 3; ++y)
                    {
                        // Cada bloco tem altura 1.0, então empilhamos em y=0, y=1, y=2
                        float pos_y = (float)y;

                        glm::mat4 model = Matrix_Translate(pos_x, pos_y, pos_z);

                        // Envia a matriz "model" para a placa de vídeo (GPU).
                        glUniformMatrix4fv(model_uniform, 1, GL_FALSE, glm::value_ptr(model));

                        // Envia flag is_damaged para o shader (1 = danificada, 0 = intacta)
                        glUniform1i(glGetUniformLocation(g_GpuProgramID, "is_damaged"), 
                                    (maze_map[z][x] == DAMAGED_WALL) ? 1 : 0);

                        // Desenha as faces coloridas
                        glUniform1i(glGetUniformLocation(g_GpuProgramID, "object_id"), 0);
                        glDrawElements(
                            g_VirtualScene["cube_faces"].rendering_mode,
                            g_VirtualScene["cube_faces"].num_indices,
                            GL_UNSIGNED_INT,
                            (void*)g_VirtualScene["cube_faces"].first_index
                        );

                        // Desenha as arestas pretas
                        glLineWidth(4.0f);
                        glUniform1i(glGetUniformLocation(g_GpuProgramID, "object_id"), 0);
                        glDrawElements(
                            g_VirtualScene["cube_edges"].rendering_mode,
                            g_VirtualScene["cube_edges"].num_indices,
                            GL_UNSIGNED_INT,
                            (void*)g_VirtualScene["cube_edges"].first_index
                        );

                        // Reseta is_damaged para 0 após desenhar
                        glUniform1i(glGetUniformLocation(g_GpuProgramID, "is_damaged"), 0);
                    }
                }
            }
        }

        CheckDiamondCollision(g_CameraPosition);

        // Agora queremos desenhar os eixos XYZ de coordenadas GLOBAIS.
        // Para tanto, colocamos a matriz de modelagem igual à identidade.
        // Veja slides 2-14 e 184-190 do documento Aula_08_Sistemas_de_Coordenadas.pdf.
        glm::mat4 model = Matrix_Identity();

        // Enviamos a nova matriz "model" para a placa de vídeo (GPU). Veja o
        // arquivo "shader_vertex.glsl".
        glUniformMatrix4fv(model_uniform, 1, GL_FALSE, glm::value_ptr(model));

        // Pedimos para OpenGL desenhar linhas com largura de 10 pixels.
        glLineWidth(10.0f);

        // Informamos para a placa de vídeo (GPU) que a variável booleana
        // "render_as_black" deve ser colocada como "false". Veja o arquivo
        // "shader_vertex.glsl".
        glUniform1i(render_as_black_uniform, false);

        // Pedimos para a GPU rasterizar os vértices dos eixos XYZ
        // apontados pelo VAO como linhas. Veja a definição de
        // g_VirtualScene["axes"] dentro da função BuildTriangles(), e veja
        // a documentação da função glDrawElements() em
        // http://docs.gl/gl3/glDrawElements.
        glDrawElements(
            g_VirtualScene["axes"].rendering_mode,
            g_VirtualScene["axes"].num_indices,
            GL_UNSIGNED_INT,
            (void*)g_VirtualScene["axes"].first_index
        );

        // Desenha a picareta fixo na câmera
        {
            // Limpa o Z-buffer (opcional no modo Look-At, mas útil no FPS)
            glClear(GL_DEPTH_BUFFER_BIT);

            glm::mat4 pickaxe_view;
            glm::mat4 model;

            // Calcular a animação usando Curva de Bézier Cúbica
            glm::vec3 bezier_anim = glm::vec3(0.0f);
            if (g_IsSwinging)
            {
                float t;
                if (g_SwingAnimationTime <= 1.0f)
                {
                    // Ida: t vai de 0 a 1
                    t = g_SwingAnimationTime;
                    bezier_anim = BezierCubic(t, g_BezierP0, g_BezierP1, g_BezierP2, g_BezierP3);
                }
                else
                {
                    // Volta: t vai de 1 a 0 (invertido)
                    t = 2.0f - g_SwingAnimationTime;
                    bezier_anim = BezierCubic(t, g_BezierP0, g_BezierP1, g_BezierP2, g_BezierP3);
                }
            }
            
            // Aplica as transformações da curva de Bézier
            // bezier_anim.x = rotação em X (swing principal)
            // bezier_anim.y = rotação em Y (movimento lateral)
            // bezier_anim.z = deslocamento em Z (avança/recua)
            glm::mat4 animation_rotation = Matrix_Rotate_X(glm::radians(bezier_anim.x))
                                         * Matrix_Rotate_Y(glm::radians(bezier_anim.y))
                                         * Matrix_Translate(0.0f, 0.0f, bezier_anim.z);

            if (g_UseFreeCamera)
            {
                // === MODO FPS (1ª Pessoa) ===
                // A picareta fica fixa na tela.
                // A View é a identidade.
                pickaxe_view = Matrix_Identity();
                
                // Posição fixa na frente da "tela"
                model = Matrix_Translate(0.5f, -0.5f, -1.0f) 
                      * animation_rotation 
                      * Matrix_Rotate_Y(glm::radians(180.0f)) // Ajuste para a ponta ficar para frente
                      * Matrix_Rotate_X(glm::radians(270.0f))
                      * Matrix_Rotate_Z(glm::radians(180.0f))
                      * Matrix_Scale(1.0f, 1.0f, 1.0f);
            }
            else
            {
                // === MODO LOOK-AT (3ª Pessoa) ===
                // A picareta fica no mundo, na posição do jogador.
                // A View é a mesma do cenário (the_view).
                pickaxe_view = the_view;

                // Posição no mundo (g_CameraPosition)
                // Rotação: Usamos g_CameraYaw para alinhar com onde o jogador estava olhando
                model = Matrix_Translate(g_CameraPosition.x, g_CameraPosition.y-0.5f, g_CameraPosition.z) * Matrix_Rotate_Y(glm::radians(g_CameraYaw + 90.f)) 
                      * animation_rotation
                      * Matrix_Rotate_Y(glm::radians(180.0f))
                      * Matrix_Rotate_X(glm::radians(270.0f))
                      * Matrix_Rotate_Z(glm::radians(180.0f))
                      * Matrix_Scale(1.0f, 1.0f, 1.0f);
            }

            // Envia as matrizes calculadas
            glUniformMatrix4fv(view_uniform, 1, GL_FALSE, glm::value_ptr(pickaxe_view));
            glUniformMatrix4fv(model_uniform, 1, GL_FALSE, glm::value_ptr(model));
            glUniform1i(glGetUniformLocation(g_GpuProgramID, "object_id"), 2); // ID 2 = Picareta
            glUniform1i(render_as_black_uniform, false);

            // Desenha o objeto
                        
            if (g_VirtualScene.count("Cube002")) 
            {
                SceneObject &obj = g_VirtualScene["Cube002"]; 
                glBindVertexArray(obj.vertex_array_object_id);
                glDrawElements(
                    obj.rendering_mode,
                    obj.num_indices,
                    GL_UNSIGNED_INT,
                    (void*)(obj.first_index * sizeof(GLuint))
                );
            }

            glBindVertexArray(0);
        }

        // Pegamos um vértice com coordenadas de modelo (0.5, 0.5, 0.5, 1) e o
        // passamos por todos os sistemas de coordenadas armazenados nas
        // matrizes the_model, the_view, e the_projection; e escrevemos na tela
        // as matrizes e pontos resultantes dessas transformações.
        glm::vec4 p_model(0.5f, 0.5f, 0.5f, 1.0f);
        TextRendering_ShowModelViewProjection(window, the_projection, the_view, the_model, p_model);

        // Imprimimos na tela os ângulos de Euler que controlam a rotação do
        // terceiro cubo.
        TextRendering_ShowEulerAngles(window);

        // Imprimimos na informação sobre a matriz de projeção sendo utilizada.
        TextRendering_ShowProjection(window);

        // Imprimimos na tela informação sobre o número de quadros renderizados
        // por segundo (frames per second).
        TextRendering_ShowFramesPerSecond(window);

        // O framebuffer onde OpenGL executa as operações de renderização não
        // é o mesmo que está sendo mostrado para o usuário, caso contrário
        // seria possível ver artefatos conhecidos como "screen tearing". A
        // chamada abaixo faz a troca dos buffers, mostrando para o usuário
        // tudo que foi renderizado pelas funções acima.
        // Veja o link: https://en.wikipedia.org/w/index.php?title=Multiple_buffering&oldid=793452829#Double_buffering_in_computer_graphics
        glfwSwapBuffers(window);

        // Verificamos com o sistema operacional se houve alguma interação do
        // usuário (teclado, mouse, ...). Caso positivo, as funções de callback
        // definidas anteriormente usando glfwSet*Callback() serão chamadas
        // pela biblioteca GLFW.
        glfwPollEvents();
    }

    // Finalizamos o uso dos recursos do sistema operacional
    glfwTerminate();

    // Fim do programa
    return 0;
}

// Carrega um Vertex Shader de um arquivo GLSL. Veja definição de LoadShader() abaixo.
GLuint LoadShader_Vertex(const char* filename)
{
    // Criamos um identificador (ID) para este shader, informando que o mesmo
    // será aplicado nos vértices.
    GLuint vertex_shader_id = glCreateShader(GL_VERTEX_SHADER);

    // Carregamos e compilamos o shader
    LoadShader(filename, vertex_shader_id);

    // Retorna o ID gerado acima
    return vertex_shader_id;
}

// Carrega um Fragment Shader de um arquivo GLSL . Veja definição de LoadShader() abaixo.
GLuint LoadShader_Fragment(const char* filename)
{
    // Criamos um identificador (ID) para este shader, informando que o mesmo
    // será aplicado nos fragmentos.
    GLuint fragment_shader_id = glCreateShader(GL_FRAGMENT_SHADER);

    // Carregamos e compilamos o shader
    LoadShader(filename, fragment_shader_id);

    // Retorna o ID gerado acima
    return fragment_shader_id;
}

// Função auxilar, utilizada pelas duas funções acima. Carrega código de GPU de
// um arquivo GLSL e faz sua compilação.
void LoadShader(const char* filename, GLuint shader_id)
{
    // Lemos o arquivo de texto indicado pela variável "filename"
    // e colocamos seu conteúdo em memória, apontado pela variável
    // "shader_string".
    std::ifstream file;
    try {
        file.exceptions(std::ifstream::failbit);
        file.open(filename);
    } catch ( std::exception& e ) {
        fprintf(stderr, "ERROR: Cannot open file \"%s\".\n", filename);
        std::exit(EXIT_FAILURE);
    }
    std::stringstream shader;
    shader << file.rdbuf();
    std::string str = shader.str();
    const GLchar* shader_string = str.c_str();
    const GLint   shader_string_length = static_cast<GLint>( str.length() );

    // Define o código do shader GLSL, contido na string "shader_string"
    glShaderSource(shader_id, 1, &shader_string, &shader_string_length);

    // Compila o código do shader GLSL (em tempo de execução)
    glCompileShader(shader_id);

    // Verificamos se ocorreu algum erro ou "warning" durante a compilação
    GLint compiled_ok;
    glGetShaderiv(shader_id, GL_COMPILE_STATUS, &compiled_ok);

    GLint log_length = 0;
    glGetShaderiv(shader_id, GL_INFO_LOG_LENGTH, &log_length);

    // Alocamos memória para guardar o log de compilação.
    // A chamada "new" em C++ é equivalente ao "malloc()" do C.
    GLchar* log = new GLchar[log_length];
    glGetShaderInfoLog(shader_id, log_length, &log_length, log);

    // Imprime no terminal qualquer erro ou "warning" de compilação
    if ( log_length != 0 )
    {
        std::string  output;

        if ( !compiled_ok )
        {
            output += "ERROR: OpenGL compilation of \"";
            output += filename;
            output += "\" failed.\n";
            output += "== Start of compilation log\n";
            output += log;
            output += "== End of compilation log\n";
        }
        else
        {
            output += "WARNING: OpenGL compilation of \"";
            output += filename;
            output += "\".\n";
            output += "== Start of compilation log\n";
            output += log;
            output += "== End of compilation log\n";
        }

        fprintf(stderr, "%s", output.c_str());
    }

    // A chamada "delete" em C++ é equivalente ao "free()" do C
    delete [] log;
}

// Função que carrega os shaders de vértices e de fragmentos que serão
// utilizados para renderização. Veja slides 180-200 do documento Aula_03_Rendering_Pipeline_Grafico.pdf.
//
void LoadShadersFromFiles()
{
    // Note que o caminho para os arquivos "shader_vertex.glsl" e
    // "shader_fragment.glsl" estão fixados, sendo que assumimos a existência
    // da seguinte estrutura no sistema de arquivos:
    //
    //    + FCG_Lab_01/
    //    |
    //    +--+ bin/
    //    |  |
    //    |  +--+ Release/  (ou Debug/ ou Linux/)
    //    |     |
    //    |     o-- main.exe
    //    |
    //    +--+ src/
    //       |
    //       o-- shader_vertex.glsl
    //       |
    //       o-- shader_fragment.glsl
    //
    GLuint vertex_shader_id = LoadShader_Vertex("../../src/shader_vertex.glsl");
    GLuint fragment_shader_id = LoadShader_Fragment("../../src/shader_fragment.glsl");

    // Deletamos o programa de GPU anterior, caso ele exista.
    if ( g_GpuProgramID != 0 )
        glDeleteProgram(g_GpuProgramID);

    // Criamos um programa de GPU utilizando os shaders carregados acima.
    g_GpuProgramID = CreateGpuProgram(vertex_shader_id, fragment_shader_id);

    glUseProgram(g_GpuProgramID);
    
    // Diz ao shader que "TextureImage0" deve ler da GL_TEXTURE0
    glUniform1i(glGetUniformLocation(g_GpuProgramID, "TextureImage0"), 0);
    
    // Diz ao shader que "TextureImage1" deve ler da GL_TEXTURE1
    glUniform1i(glGetUniformLocation(g_GpuProgramID, "TextureImage1"), 1);
    
    // Diz ao shader que "TextureImage2" deve ler da GL_TEXTURE2
    glUniform1i(glGetUniformLocation(g_GpuProgramID, "TextureImage2"), 2);
    
    // Diz ao shader que "TextureImage3" deve ler da GL_TEXTURE3
    glUniform1i(glGetUniformLocation(g_GpuProgramID, "TextureImage3"), 3);
    
    // Diz ao shader que "TextureImage4" deve ler da GL_TEXTURE4
    glUniform1i(glGetUniformLocation(g_GpuProgramID, "TextureImage4"), 4);
    
    glUseProgram(0);

}

// Esta função cria um programa de GPU, o qual contém obrigatoriamente um
// Vertex Shader e um Fragment Shader.
GLuint CreateGpuProgram(GLuint vertex_shader_id, GLuint fragment_shader_id)
{
    // Criamos um identificador (ID) para este programa de GPU
    GLuint program_id = glCreateProgram();

    // Definição dos dois shaders GLSL que devem ser executados pelo programa
    glAttachShader(program_id, vertex_shader_id);
    glAttachShader(program_id, fragment_shader_id);

    // Linkagem dos shaders acima ao programa
    glLinkProgram(program_id);

    // Verificamos se ocorreu algum erro durante a linkagem
    GLint linked_ok = GL_FALSE;
    glGetProgramiv(program_id, GL_LINK_STATUS, &linked_ok);

    // Imprime no terminal qualquer erro de linkagem
    if ( linked_ok == GL_FALSE )
    {
        GLint log_length = 0;
        glGetProgramiv(program_id, GL_INFO_LOG_LENGTH, &log_length);

        // Alocamos memória para guardar o log de compilação.
        // A chamada "new" em C++ é equivalente ao "malloc()" do C.
        GLchar* log = new GLchar[log_length];

        glGetProgramInfoLog(program_id, log_length, &log_length, log);

        std::string output;

        output += "ERROR: OpenGL linking of program failed.\n";
        output += "== Start of link log\n";
        output += log;
        output += "\n== End of link log\n";

        // A chamada "delete" em C++ é equivalente ao "free()" do C
        delete [] log;

        fprintf(stderr, "%s", output.c_str());
    }

    // Retornamos o ID gerado acima
    return program_id;
}

// Definição da função que será chamada sempre que a janela do sistema
// operacional for redimensionada, por consequência alterando o tamanho do
// "framebuffer" (região de memória onde são armazenados os pixels da imagem).
void FramebufferSizeCallback(GLFWwindow* window, int width, int height)
{
    // Indicamos que queremos renderizar em toda região do framebuffer. A
    // função "glViewport" define o mapeamento das "normalized device
    // coordinates" (NDC) para "pixel coordinates".  Essa é a operação de
    // "Screen Mapping" ou "Viewport Mapping" vista em aula ({+ViewportMapping2+}).
    glViewport(0, 0, width, height);

    // Atualizamos também a razão que define a proporção da janela (largura /
    // altura), a qual será utilizada na definição das matrizes de projeção,
    // tal que não ocorra distorções durante o processo de "Screen Mapping"
    // acima, quando NDC é mapeado para coordenadas de pixels. Veja slides 205-215 do documento Aula_09_Projecoes.pdf.
    //
    // O cast para float é necessário pois números inteiros são arredondados ao
    // serem divididos!
    g_ScreenRatio = (float)width / height;
}

// Variáveis globais que armazenam a última posição do cursor do mouse, para
// que possamos calcular quanto que o mouse se movimentou entre dois instantes
// de tempo. Utilizadas no callback CursorPosCallback() abaixo.
double g_LastCursorPosX, g_LastCursorPosY;

// Função callback chamada sempre que o usuário aperta algum dos botões do mouse
void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
    {
        // Se o usuário pressionou o botão esquerdo do mouse, guardamos a
        // posição atual do cursor nas variáveis g_LastCursorPosX e
        // g_LastCursorPosY.  Também, setamos a variável
        // g_LeftMouseButtonPressed como true, para saber que o usuário está
        // com o botão esquerdo pressionado.
        glfwGetCursorPos(window, &g_LastCursorPosX, &g_LastCursorPosY);
        g_LeftMouseButtonPressed = true;
    }
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE)
    {
        // Quando o usuário soltar o botão esquerdo do mouse, atualizamos a
        // variável abaixo para false.
        g_LeftMouseButtonPressed = false;
    }
}

// Função callback chamada sempre que o usuário movimentar o cursor do mouse em
// cima da janela OpenGL.
void CursorPosCallback(GLFWwindow* window, double xpos, double ypos)
{
    // Abaixo executamos o seguinte: caso o botão esquerdo do mouse esteja
    // pressionado, computamos quanto que o mouse se movimento desde o último
    // instante de tempo, e usamos esta movimentação para atualizar os
    // parâmetros que definem a posição da câmera dentro da cena virtual.
    // Assim, temos que o usuário consegue controlar a câmera.

    //if (!g_LeftMouseButtonPressed)
    //{
    //    // Mesmo sem pressionar, atualiza a última posição para evitar saltos
    //    glfwGetCursorPos(window, &g_LastCursorPosX, &g_LastCursorPosY);
    //    return;
    //}

    static bool firstMouse = true;
    if (firstMouse)
    {
        g_LastCursorPosX = xpos;
        g_LastCursorPosY = ypos;
        firstMouse = false;
    }

    // Deslocamento do cursor do mouse em x e y de coordenadas de tela!
    float dx = xpos - g_LastCursorPosX;
    float dy = ypos - g_LastCursorPosY;

    if (g_UseFreeCamera)
    {
        /*
        // Atualizamos parâmetros da câmera com os deslocamentos
        g_CameraTheta -= 0.01f*dx;
        g_CameraPhi   += 0.01f*dy;

        // Em coordenadas esféricas, o ângulo phi deve ficar entre -pi/2 e +pi/2.
        float phimax = 3.141592f/2;
        float phimin = -phimax;

        if (g_CameraPhi > phimax)
            g_CameraPhi = phimax;

        if (g_CameraPhi < phimin)
            g_CameraPhi = phimin;
        */
        // Sensibilidade do mouse
        float sensitivity = 0.1f;
        dx *= sensitivity;
        dy *= sensitivity;

        // Atualizamos os ângulos de Yaw e Pitch
        g_CameraYaw   += dx;
        g_CameraPitch -= dy; // Invertido, pois coordenadas Y de tela crescem para baixo

        // Impomos limites ao Pitch para evitar que a câmera vire de cabeça para baixo
        if (g_CameraPitch > 89.0f)
            g_CameraPitch = 89.0f;
        if (g_CameraPitch < -89.0f)
            g_CameraPitch = -89.0f;

        // Calculamos o novo vetor de visão (direção) da câmera
        glm::vec4 front;
        front.x = cos(glm::radians(g_CameraYaw)) * cos(glm::radians(g_CameraPitch));
        front.y = sin(glm::radians(g_CameraPitch));
        front.z = sin(glm::radians(g_CameraYaw)) * cos(glm::radians(g_CameraPitch));
        front.w = 0.0f; // Vetor de direção, w=0
        g_CameraViewVector = normalize(front);
    }
    else
    {
        // --- LÓGICA DA CÂMERA LOOK-AT (ORBITAL) ---
        // Aqui movemos Theta e Phi para orbitar
        float sensitivity = 0.01f; 
        g_CameraTheta -= dx * sensitivity;
        g_CameraPhi   += dy * sensitivity;

        // Limites de Phi para não virar de ponta cabeça
        float phiMax = 3.141592f / 2.0f - 0.1f; // Quase 90 graus
        float phiMin = -3.141592f / 2.0f + 0.1f;

        if (g_CameraPhi > phiMax) g_CameraPhi = phiMax;
        if (g_CameraPhi < phiMin) g_CameraPhi = phiMin;

        g_CameraYaw = glm::degrees(g_CameraTheta) - 90.0f;
    }
    // Atualizamos as variáveis globais para armazenar a posição atual do
    // cursor como sendo a última posição conhecida do cursor.
    g_LastCursorPosX = xpos;
    g_LastCursorPosY = ypos;
}

// Função callback chamada sempre que o usuário movimenta a "rodinha" do mouse.
void ScrollCallback(GLFWwindow* window, double xoffset, double yoffset)
{
    /*
    // Atualizamos a distância da câmera para a origem utilizando a
    // movimentação da "rodinha", simulando um ZOOM.
    g_CameraDistance -= 0.1f*yoffset;

    // Uma câmera look-at nunca pode estar exatamente "em cima" do ponto para
    // onde ela está olhando, pois isto gera problemas de divisão por zero na
    // definição do sistema de coordenadas da câmera. Isto é, a variável abaixo
    // nunca pode ser zero. Versões anteriores deste código possuíam este bug,
    // o qual foi detectado pelo aluno Vinicius Fraga (2017/2).
    const float verysmallnumber = std::numeric_limits<float>::epsilon();
    if (g_CameraDistance < verysmallnumber)
        g_CameraDistance = verysmallnumber;
    */
}

// Definição da função que será chamada sempre que o usuário pressionar alguma
// tecla do teclado. Veja http://www.glfw.org/docs/latest/input_guide.html#input_key
void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mod)
{
    // =======================
    // Não modifique este loop! Ele é utilizando para correção automatizada dos
    // laboratórios. Deve ser sempre o primeiro comando desta função KeyCallback().
    for (int i = 0; i < 10; ++i)
        if (key == GLFW_KEY_0 + i && action == GLFW_PRESS && mod == GLFW_MOD_SHIFT)
            std::exit(100 + i);
    // =======================

    // Se o usuário pressionar a tecla ESC, fechamos a janela.
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GL_TRUE);

    // O código abaixo implementa a seguinte lógica:
    //   Se apertar tecla X       então g_AngleX += delta;
    //   Se apertar tecla shift+X então g_AngleX -= delta;
    //   Se apertar tecla Y       então g_AngleY += delta;
    //   Se apertar tecla shift+Y então g_AngleY -= delta;
    //   Se apertar tecla Z       então g_AngleZ += delta;
    //   Se apertar tecla shift+Z então g_AngleZ -= delta;

    if (key == GLFW_KEY_C && action == GLFW_PRESS)
    {
        // Altera variáveis de controle da câmera e do teto
        g_UseFreeCamera = !g_UseFreeCamera;
        g_ShowCeiling = !g_ShowCeiling;
    }

    float delta = 3.141592 / 16; // 22.5 graus, em radianos.

    if (key == GLFW_KEY_X && action == GLFW_PRESS)
    {
        g_AngleX += (mod & GLFW_MOD_SHIFT) ? -delta : delta;
    }

    if (key == GLFW_KEY_Y && action == GLFW_PRESS)
    {
        g_AngleY += (mod & GLFW_MOD_SHIFT) ? -delta : delta;
    }
    if (key == GLFW_KEY_Z && action == GLFW_PRESS)
    {
        g_AngleZ += (mod & GLFW_MOD_SHIFT) ? -delta : delta;
    }

    // Se o usuário apertar a tecla espaço, resetamos os ângulos de Euler para zero.
    if (key == GLFW_KEY_SPACE && action == GLFW_PRESS)
    {
        g_AngleX = 0.0f;
        g_AngleY = 0.0f;
        g_AngleZ = 0.0f;
    }

    // Se o usuário apertar a tecla P, utilizamos projeção perspectiva.
    if (key == GLFW_KEY_P && action == GLFW_PRESS)
    {
        g_UsePerspectiveProjection = true;
    }

    // Se o usuário apertar a tecla O, utilizamos projeção ortográfica.
    if (key == GLFW_KEY_O && action == GLFW_PRESS)
    {
        g_UsePerspectiveProjection = false;
    }

    // Se o usuário apertar a tecla H, fazemos um "toggle" do texto informativo mostrado na tela.
    if (key == GLFW_KEY_H && action == GLFW_PRESS)
    {
        g_ShowInfoText = !g_ShowInfoText;
    }

    // Só começa uma nova animação se não houver uma em andamento
    if (key == GLFW_KEY_Q && action == GLFW_PRESS)
    {
        if (!g_IsSwinging)
        {
            // Controle da animação de balanço da picareta
            g_IsSwinging = true;
            // Reseta a animação
            g_SwingAnimationTime = 0.0f;

            CheckMapCollisionAndBreak(g_CameraPosition, g_CameraViewVector);
        }
    }

    if (key == GLFW_KEY_W)
    {
        if (action == GLFW_PRESS)
            g_W_pressed = true;
        if (action == GLFW_RELEASE)
            g_W_pressed = false;
    }

    if (key == GLFW_KEY_S)
    {
        if (action == GLFW_PRESS)
            g_S_pressed = true;
        if (action == GLFW_RELEASE)
            g_S_pressed = false;
    }

    if (key == GLFW_KEY_A)
    {
        if (action == GLFW_PRESS)
            g_A_pressed = true;
        if (action == GLFW_RELEASE)
            g_A_pressed = false;
    }

    if (key == GLFW_KEY_D)
    {
        if (action == GLFW_PRESS)
            g_D_pressed = true;
        if (action == GLFW_RELEASE)
            g_D_pressed = false;
    }



}

// Definimos o callback para impressão de erros da GLFW no terminal
void ErrorCallback(int error, const char* description)
{
    fprintf(stderr, "ERROR: GLFW: %s\n", description);
}

// Esta função recebe um vértice com coordenadas de modelo p_model e passa o
// mesmo por todos os sistemas de coordenadas armazenados nas matrizes model,
// view, e projection; e escreve na tela as matrizes e pontos resultantes
// dessas transformações.
void TextRendering_ShowModelViewProjection(
    GLFWwindow* window,
    glm::mat4 projection,
    glm::mat4 view,
    glm::mat4 model,
    glm::vec4 p_model
)
{
    if ( !g_ShowInfoText )
        return;

    glm::vec4 p_world = model*p_model;
    glm::vec4 p_camera = view*p_world;
    glm::vec4 p_clip = projection*p_camera;
    glm::vec4 p_ndc = p_clip / p_clip.w;

    float pad = TextRendering_LineHeight(window);

    TextRendering_PrintString(window, " Model matrix             Model     In World Coords.", -1.0f, 1.0f-pad, 1.0f);
    TextRendering_PrintMatrixVectorProduct(window, model, p_model, -1.0f, 1.0f-2*pad, 1.0f);

    TextRendering_PrintString(window, "                                        |  ", -1.0f, 1.0f-6*pad, 1.0f);
    TextRendering_PrintString(window, "                            .-----------'  ", -1.0f, 1.0f-7*pad, 1.0f);
    TextRendering_PrintString(window, "                            V              ", -1.0f, 1.0f-8*pad, 1.0f);

    TextRendering_PrintString(window, " View matrix              World     In Camera Coords.", -1.0f, 1.0f-9*pad, 1.0f);
    TextRendering_PrintMatrixVectorProduct(window, view, p_world, -1.0f, 1.0f-10*pad, 1.0f);

    TextRendering_PrintString(window, "                                        |  ", -1.0f, 1.0f-14*pad, 1.0f);
    TextRendering_PrintString(window, "                            .-----------'  ", -1.0f, 1.0f-15*pad, 1.0f);
    TextRendering_PrintString(window, "                            V              ", -1.0f, 1.0f-16*pad, 1.0f);

    TextRendering_PrintString(window, " Projection matrix        Camera                    In NDC", -1.0f, 1.0f-17*pad, 1.0f);
    TextRendering_PrintMatrixVectorProductDivW(window, projection, p_camera, -1.0f, 1.0f-18*pad, 1.0f);

    int width, height;
    glfwGetFramebufferSize(window, &width, &height);

    glm::vec2 a = glm::vec2(-1, -1);
    glm::vec2 b = glm::vec2(+1, +1);
    glm::vec2 p = glm::vec2( 0,  0);
    glm::vec2 q = glm::vec2(width, height);

    glm::mat4 viewport_mapping = Matrix(
        (q.x - p.x)/(b.x-a.x), 0.0f, 0.0f, (b.x*p.x - a.x*q.x)/(b.x-a.x),
        0.0f, (q.y - p.y)/(b.y-a.y), 0.0f, (b.y*p.y - a.y*q.y)/(b.y-a.y),
        0.0f , 0.0f , 1.0f , 0.0f ,
        0.0f , 0.0f , 0.0f , 1.0f
    );

    TextRendering_PrintString(window, "                                                       |  ", -1.0f, 1.0f-22*pad, 1.0f);
    TextRendering_PrintString(window, "                            .--------------------------'  ", -1.0f, 1.0f-23*pad, 1.0f);
    TextRendering_PrintString(window, "                            V                           ", -1.0f, 1.0f-24*pad, 1.0f);

    TextRendering_PrintString(window, " Viewport matrix           NDC      In Pixel Coords.", -1.0f, 1.0f-25*pad, 1.0f);
    TextRendering_PrintMatrixVectorProductMoreDigits(window, viewport_mapping, p_ndc, -1.0f, 1.0f-26*pad, 1.0f);
}

// Escrevemos na tela os ângulos de Euler definidos nas variáveis globais
// g_AngleX, g_AngleY, e g_AngleZ.
void TextRendering_ShowEulerAngles(GLFWwindow* window)
{
    if ( !g_ShowInfoText )
        return;

    float pad = TextRendering_LineHeight(window);

    char buffer[80];
    snprintf(buffer, 80, "Euler Angles rotation matrix = Z(%.2f)*Y(%.2f)*X(%.2f)\n", g_AngleZ, g_AngleY, g_AngleX);

    TextRendering_PrintString(window, buffer, -1.0f+pad/10, -1.0f+2*pad/10, 1.0f);
}

// Escrevemos na tela qual matriz de projeção está sendo utilizada.
void TextRendering_ShowProjection(GLFWwindow* window)
{
    if ( !g_ShowInfoText )
        return;

    float lineheight = TextRendering_LineHeight(window);
    float charwidth = TextRendering_CharWidth(window);

    if ( g_UsePerspectiveProjection )
        TextRendering_PrintString(window, "Perspective", 1.0f-13*charwidth, -1.0f+2*lineheight/10, 1.0f);
    else
        TextRendering_PrintString(window, "Orthographic", 1.0f-13*charwidth, -1.0f+2*lineheight/10, 1.0f);
}

// Escrevemos na tela o número de quadros renderizados por segundo (frames per
// second).
void TextRendering_ShowFramesPerSecond(GLFWwindow* window)
{
    if ( !g_ShowInfoText )
        return;

    // Variáveis estáticas (static) mantém seus valores entre chamadas
    // subsequentes da função!
    static float old_seconds = (float)glfwGetTime();
    static int   ellapsed_frames = 0;
    static char  buffer[20] = "?? fps";
    static int   numchars = 7;

    ellapsed_frames += 1;

    // Recuperamos o número de segundos que passou desde a execução do programa
    float seconds = (float)glfwGetTime();

    // Número de segundos desde o último cálculo do fps
    float ellapsed_seconds = seconds - old_seconds;

    if ( ellapsed_seconds > 1.0f )
    {
        numchars = snprintf(buffer, 20, "%.2f fps", ellapsed_frames / ellapsed_seconds);

        old_seconds = seconds;
        ellapsed_frames = 0;
    }

    float lineheight = TextRendering_LineHeight(window);
    float charwidth = TextRendering_CharWidth(window);

    TextRendering_PrintString(window, buffer, 1.0f-(numchars + 1)*charwidth, 1.0f-lineheight, 1.0f);
}

// set makeprg=cd\ ..\ &&\ make\ run\ >/dev/null
// vim: set spell spelllang=pt_br :

