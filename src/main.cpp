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

// Declaração de várias funções utilizadas em main().  Essas estão definidas
// logo após a definição de main() neste arquivo.
GLuint BuildTriangles(); // Constrói triângulos para renderização
void LoadShadersFromFiles(); // Carrega os shaders de vértice e fragmento, criando um programa de GPU
GLuint LoadShader_Vertex(const char* filename);   // Carrega um vertex shader
GLuint LoadShader_Fragment(const char* filename); // Carrega um fragment shader
void LoadShader(const char* filename, GLuint shader_id); // Função utilizada pelas duas acima
GLuint CreateGpuProgram(GLuint vertex_shader_id, GLuint fragment_shader_id); // Cria um programa de GPU

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

// Definimos uma estrutura que armazenará dados necessários para renderizar
// cada objeto da cena virtual.
struct SceneObject
{
    const char*  name;        // Nome do objeto
    void*        first_index; // Índice do primeiro vértice dentro do vetor indices[] definido em BuildTriangles()
    int          num_indices; // Número de índices do objeto dentro do vetor indices[] definido em BuildTriangles()
    GLenum       rendering_mode; // Modo de rasterização (GL_TRIANGLES, GL_TRIANGLE_STRIP, etc.)
};

// Abaixo definimos variáveis globais utilizadas em várias funções do código.

// A cena virtual é uma lista de objetos nomeados, guardados em um dicionário
// (map).  Veja dentro da função BuildTriangles() como que são incluídos
// objetos dentro da variável g_VirtualScene, e veja na função main() como
// estes são acessados.
std::map<const char*, SceneObject> g_VirtualScene;

// Razão de proporção da janela (largura/altura). Veja função FramebufferSizeCallback().
float g_ScreenRatio = 1.0f;

// Ângulos de Euler que controlam a rotação de um dos cubos da cena virtual
float g_AngleX = 0.0f;
float g_AngleY = 0.0f;
float g_AngleZ = 0.0f;

// "g_LeftMouseButtonPressed = true" se o usuário está com o botão esquerdo do mouse
// pressionado no momento atual. Veja função MouseButtonCallback().
bool g_LeftMouseButtonPressed = false;

// Variáveis que definem a câmera em coordenadas esféricas, controladas pelo
// usuário através do mouse (veja função CursorPosCallback()). A posição
// efetiva da câmera é calculada dentro da função main(), dentro do loop de
// renderização.
float g_CameraTheta = 0.0f; // Ângulo no plano ZX em relação ao eixo Z
float g_CameraPhi = 0.0f;   // Ângulo em relação ao eixo Y
float g_CameraDistance = 2.5f; // Distância da câmera para a origem

glm::vec4 g_CameraPosition     = glm::vec4(-4.0f, 0.0f, -4.0f, 1.0f); // Posição inicial da câmera
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
float g_SwingAnimationTime = 0.0f; // Controla o progresso da animação (em radianos)

// Para animação e física independentes de framerate
float g_DeltaTime = 0.0f;
float g_LastFrameTime = 0.0f;

// Enum para os valores possíveis do mapa
enum MapType
{
    EMPTY = 0,        // Vazio
    WALL = 1,         // Parede sólida
    DAMAGED_WALL = 2, // Parede danificada (batida uma vez)
    DIAMOND = 3       // Diamante
};

// Tamanho do mapa
const int map_width = 10;
const int map_height = 10;
// Mapa do labirinto. 0 = vazio, 1 = bloco de parede
int maze_map[map_height][map_width] = {
    {WALL,WALL,WALL,WALL,WALL,WALL,WALL,WALL,WALL,WALL},
    {WALL,EMPTY,EMPTY,WALL,EMPTY,EMPTY,WALL,EMPTY,EMPTY,WALL},
    {WALL,EMPTY,EMPTY,WALL,EMPTY,EMPTY,WALL,EMPTY,EMPTY,WALL},
    {WALL,EMPTY,EMPTY,WALL,EMPTY,EMPTY,WALL,EMPTY,EMPTY,WALL},
    {WALL,EMPTY,EMPTY,WALL,EMPTY,EMPTY,WALL,EMPTY,EMPTY,WALL},
    {WALL,EMPTY,EMPTY,WALL,EMPTY,EMPTY,WALL,EMPTY,EMPTY,WALL},
    {WALL,EMPTY,EMPTY,WALL,EMPTY,EMPTY,WALL,EMPTY,EMPTY,WALL},
    {WALL,EMPTY,EMPTY,WALL,EMPTY,EMPTY,WALL,EMPTY,EMPTY,WALL},
    {WALL,EMPTY,EMPTY,WALL,EMPTY,EMPTY,WALL,EMPTY,EMPTY,WALL},
    {WALL,WALL,WALL,WALL,WALL,WALL,WALL,WALL,WALL,WALL}
};

// Contador para fazer o impacto da picareta na parede.
// TODO: Duas maneiras de fazer o quebra parede.
// Ou com o maze_map mudando de valor, acho que este é mais fácil.
// Ou com o hit_wall_counts contando até 2 e aí removendo o cubo da parede.
// int hit_wall_counts[map_height][map_width] = {0};

// Função para detectar colisão com as paredes
bool CheckCollision(float x, float z)
{
    // Raio do corpo do personagem
    float collision_radius = 0.2f; 

    // Precisamos verificar uma pequena área ao redor do ponto para evitar atravessar
    // Verificamos 4 pontos ao redor da posição (frente, trás, esquerda, direita)
    float check_points[4][2] = {
        { x + collision_radius, z },
        { x - collision_radius, z },
        { x, z + collision_radius },
        { x, z - collision_radius }
    };

    for (int i = 0; i < 4; i++)
    {
        float px = check_points[i][0];
        float pz = check_points[i][1];

        // Conversão de Coordenada de Mundo para Índice da Matriz
        // Inverso da lógica usada no desenho: pos_x = x - width/2
        // Logo: grid_x = world_x + width/2
        // O +0.5f é para arredondar para o índice inteiro correto
        int grid_x = (int)(px + (float)map_width / 2.0f + 0.5f);
        int grid_z = (int)(pz + (float)map_height / 2.0f + 0.5f);

        // Verifica se está dentro dos limites do mapa
        if (grid_x >= 0 && grid_x < map_width && grid_z >= 0 && grid_z < map_height)
        {
            // Se for 1, é parede!
            if (maze_map[grid_z][grid_x] == 1)
                return true;
        }
    }
    return false;
}


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
            float swing_speed = 10.0f; // Velocidade da animação (radianos/seg)
            g_SwingAnimationTime += swing_speed * g_DeltaTime;

            // Usamos PI (3.141592) porque a função sin(x) de 0 a PI
            // faz um movimento de "ida e volta" (0 -> 1 -> 0).
            if (g_SwingAnimationTime >= 3.141592f)
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
        glm::vec4 forward_vector = glm::vec4(g_CameraViewVector.x, 0.0f, g_CameraViewVector.z, 0.0f);
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

        glm::mat4 view = Matrix_Camera_View(g_CameraPosition, g_CameraViewVector, g_CameraUpVector);


        // Agora computamos a matriz de Projeção.
        glm::mat4 projection;

        // Note que, no sistema de coordenadas da câmera, os planos near e far
        // estão no sentido negativo! Veja slides 176-204 do documento Aula_09_Projecoes.pdf.
        float nearplane = -0.1f;  // Posição do "near plane"
        float farplane  = -10.0f; // Posição do "far plane"

        if (g_UsePerspectiveProjection)
        {
            // Projeção Perspectiva.
            // Para definição do field of view (FOV), veja slides 205-215 do documento Aula_09_Projecoes.pdf.
            float field_of_view = 3.141592 / 3.0f;
            projection = Matrix_Perspective(field_of_view, g_ScreenRatio, nearplane, farplane);
        }
        else
        {
            // Projeção Ortográfica.
            // Para definição dos valores l, r, b, t ("left", "right", "bottom", "top"),
            // PARA PROJEÇÃO ORTOGRÁFICA veja slides 219-224 do documento Aula_09_Projecoes.pdf.
            // Para simular um "zoom" ortográfico, computamos o valor de "t"
            // utilizando a variável g_CameraDistance.
            float t = 1.5f*g_CameraDistance/2.5f;
            float b = -t;
            float r = t*g_ScreenRatio;
            float l = -r;
            projection = Matrix_Orthographic(l, r, b, t, nearplane, farplane);
        }

        // Enviamos as matrizes "view" e "projection" para a placa de vídeo
        // (GPU). Veja o arquivo "shader_vertex.glsl", onde estas são
        // efetivamente aplicadas em todos os pontos.
        glUniformMatrix4fv(view_uniform       , 1 , GL_FALSE , glm::value_ptr(view));
        glUniformMatrix4fv(projection_uniform , 1 , GL_FALSE , glm::value_ptr(projection));


        the_projection = projection;
        the_view = view;

        // Desenha o chão
        {
            glm::mat4 model = Matrix_Identity();
            the_model = model; // Salva para o texto

            glUniformMatrix4fv(model_uniform, 1, GL_FALSE, glm::value_ptr(model));
            glUniform1i(render_as_black_uniform, false);

            glDrawElements(
                g_VirtualScene["floor"].rendering_mode,
                g_VirtualScene["floor"].num_indices,
                GL_UNSIGNED_INT,
                (void*)g_VirtualScene["floor"].first_index
            );
        }

        // Desenha o labirinto com base no maze_map
        for (int z = 0; z < map_height; ++z)
        {
            for (int x = 0; x < map_width; ++x)
            {
                // Se for uma parede (WALL ou DAMAGED_WALL)
                if (maze_map[z][x] == WALL || maze_map[z][x] == DAMAGED_WALL)
                {
                    // Os cubos têm 1x1x1 (de -0.5 a 0.5).
                    // O chão está em y = -0.5.
                    // Queremos que o cubo fique "em cima" do chão.
                    // Um translate(x, 0.0, z) posiciona o *centro* do cubo em y=0.0,
                    // fazendo-o ocupar de y=-0.5 a y=0.5. Perfeito.

                    // Calcula a posição no mundo, centrando o labirinto em (0,0)
                    float pos_x = (float)x - (float)map_width / 2.0f;
                    float pos_z = (float)z - (float)map_height / 2.0f;

                    glm::mat4 model = Matrix_Translate(pos_x, 0.0f, pos_z);

                    // Envia a matriz "model" para a placa de vídeo (GPU).
                    glUniformMatrix4fv(model_uniform, 1, GL_FALSE, glm::value_ptr(model));
                    
                    // Desenha as faces coloridas
                    glUniform1i(render_as_black_uniform, false);

                    // Decide qual cubo desenhar
                    if (maze_map[z][x] == DAMAGED_WALL)
                    {
                        // Desenha o cubo vermelho
                        glDrawElements(
                            g_VirtualScene["damaged_cube_faces"].rendering_mode,
                            g_VirtualScene["damaged_cube_faces"].num_indices,
                            GL_UNSIGNED_INT,
                            (void*)g_VirtualScene["damaged_cube_faces"].first_index
                        );
                    }
                    else // Se for WALL
                    {
                        // Desenha o cubo azul
                        glDrawElements(
                            g_VirtualScene["cube_faces"].rendering_mode,
                            g_VirtualScene["cube_faces"].num_indices,
                            GL_UNSIGNED_INT,
                            (void*)g_VirtualScene["cube_faces"].first_index
                        );
                    }

                    // Desenha as arestas pretas para ambos os cubos
                    glLineWidth(4.0f);
                    glUniform1i(render_as_black_uniform, true);
                    glDrawElements(
                        g_VirtualScene["cube_edges"].rendering_mode,
                        g_VirtualScene["cube_edges"].num_indices,
                        GL_UNSIGNED_INT,
                        (void*)g_VirtualScene["cube_edges"].first_index
                    );
                }
            }
        }

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
            // Limpa o Z-buffer para garantir que a picareta sempre apareça na frente
            glClear(GL_DEPTH_BUFFER_BIT);

            // Usamos uma matriz View identidade para desenhar em "Camera Space"
            glm::mat4 pickaxe_view = Matrix_Identity();
            glUniformMatrix4fv(view_uniform, 1, GL_FALSE, glm::value_ptr(pickaxe_view));


            // Calcular a rotação da animação
            float swing_angle_rad = 0.0f;
            if (g_IsSwinging)
            {
                // sin(0) = 0, sin(pi/2) = 1, sin(pi) = 0.
                // Isso dá um movimento suave de "ida e volta".
                // Multiplicamos por um ângulo máximo (ex: 45 graus)
                float max_swing_angle = glm::radians(-90.0f);
                swing_angle_rad = sin(g_SwingAnimationTime) * max_swing_angle;
            }
            // Criamos uma matriz de rotação para a animação (em torno do eixo X)
            glm::mat4 animation_rotation = Matrix_Rotate_X(swing_angle_rad);

            // Modelo do Cabo da Picareta
            glm::mat4 handle_model = Matrix_Translate(0.4f, -0.3f, -1.0f) // Posição na tela
                                   * animation_rotation // Animação
                                   * Matrix_Rotate_Y(glm::radians(90.0f))
                                   * Matrix_Rotate_Z(glm::radians(-30.0f)) // Rotação
                                   * Matrix_Scale(0.08f, 0.6f, 0.08f);    // Tamanho (fino e longo)

            glUniformMatrix4fv(model_uniform, 1, GL_FALSE, glm::value_ptr(handle_model));

            // Desenha faces do cabo
            glUniform1i(render_as_black_uniform, false);
            glDrawElements(
                g_VirtualScene["cube_faces"].rendering_mode,
                g_VirtualScene["cube_faces"].num_indices,
                GL_UNSIGNED_INT,
                (void*)g_VirtualScene["cube_faces"].first_index
            );
            // Desenha arestas do cabo
            glUniform1i(render_as_black_uniform, true);
            glDrawElements(
                g_VirtualScene["cube_edges"].rendering_mode,
                g_VirtualScene["cube_edges"].num_indices,
                GL_UNSIGNED_INT,
                (void*)g_VirtualScene["cube_edges"].first_index
            );

            // Modelo da Cabeça da Picareta
            glm::mat4 head_model = Matrix_Translate(0.4f, -0.3f, -1.0f)      // Mesma Posição
                                 * animation_rotation // Animação
                                 * Matrix_Rotate_Y(glm::radians(90.0f))
                                 * Matrix_Rotate_Z(glm::radians(-30.0f))    // Mesma Rotação
                                 * Matrix_Translate(0.0f, 0.25f, 0.0f)      // Desloca para a ponta do cabo
                                 * Matrix_Scale(0.8f, 0.08f, 0.03f);       // Tamanho (largo e fino)

            glUniformMatrix4fv(model_uniform, 1, GL_FALSE, glm::value_ptr(head_model));

            // Desenha faces da cabeça
            glUniform1i(render_as_black_uniform, false);
            glDrawElements(
                g_VirtualScene["cube_faces"].rendering_mode,
                g_VirtualScene["cube_faces"].num_indices,
                GL_UNSIGNED_INT,
                (void*)g_VirtualScene["cube_faces"].first_index
            );
            // Desenha arestas da cabeça
            glUniform1i(render_as_black_uniform, true);
            glDrawElements(
                g_VirtualScene["cube_edges"].rendering_mode,
                g_VirtualScene["cube_edges"].num_indices,
                GL_UNSIGNED_INT,
                (void*)g_VirtualScene["cube_edges"].first_index
            );
        }




        // "Desligamos" o VAO, evitando assim que operações posteriores venham a
        // alterar o mesmo. Isso evita bugs.
        glBindVertexArray(0);

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

// Constrói triângulos para futura renderização
GLuint BuildTriangles()
{
    // Primeiro, definimos os atributos de cada vértice.

    // A posição de cada vértice é definida por coeficientes em um sistema de
    // coordenadas local de cada modelo geométrico. Note o uso de coordenadas
    // homogêneas.  Veja as seguintes referências:
    //
    //  - slides 35-48 do documento Aula_08_Sistemas_de_Coordenadas.pdf;
    //  - slides 184-190 do documento Aula_08_Sistemas_de_Coordenadas.pdf;
    //
    // Este vetor "model_coefficients" define a GEOMETRIA (veja slides 103-110 do documento Aula_04_Modelagem_Geometrica_3D.pdf).
    //
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
    //    X      Y     Z     W
         0.0f,  0.0f,  0.0f, 1.0f, // posição do vértice 8
         1.0f,  0.0f,  0.0f, 1.0f, // posição do vértice 9
    // Vértices para desenhar o eixo Y
    //    X      Y     Z     W
         0.0f,  0.0f,  0.0f, 1.0f, // posição do vértice 10
         0.0f,  1.0f,  0.0f, 1.0f, // posição do vértice 11
    // Vértices para desenhar o eixo Z
    //    X      Y     Z     W
         0.0f,  0.0f,  0.0f, 1.0f, // posição do vértice 12
         0.0f,  0.0f,  1.0f, 1.0f, // posição do vértice 13
    // Vértices para o chão (y = -0.5)
    //    X      Y     Z     W
      -50.0f, -0.5f, -50.0f, 1.0f, // vértice 14
       50.0f, -0.5f, -50.0f, 1.0f, // vértice 15
       50.0f, -0.5f,  50.0f, 1.0f, // vértice 16
      -50.0f, -0.5f,  50.0f, 1.0f,  // vértice 17
    // Vértices para o cubo vermelho (mesmas posições do cubo original)
    //    X      Y     Z     W
        -0.5f,  0.5f,  0.5f, 1.0f, // posição do vértice 18
        -0.5f, -0.5f,  0.5f, 1.0f, // posição do vértice 19
         0.5f, -0.5f,  0.5f, 1.0f, // posição do vértice 20
         0.5f,  0.5f,  0.5f, 1.0f, // posição do vértice 21
        -0.5f,  0.5f, -0.5f, 1.0f, // posição do vértice 22
        -0.5f, -0.5f, -0.5f, 1.0f, // posição do vértice 23
         0.5f, -0.5f, -0.5f, 1.0f, // posição do vértice 24
         0.5f,  0.5f, -0.5f, 1.0f  // posição do vértice 25
    };


    // Criamos o identificador (ID) de um Vertex Buffer Object (VBO).  Um VBO é
    // um buffer de memória que irá conter os valores de um certo atributo de
    // um conjunto de vértices; por exemplo: posição, cor, normais, coordenadas
    // de textura.  Neste exemplo utilizaremos vários VBOs, um para cada tipo de atributo.
    // Agora criamos um VBO para armazenarmos um atributo: posição.
    GLuint VBO_model_coefficients_id;
    glGenBuffers(1, &VBO_model_coefficients_id);

    // Criamos o identificador (ID) de um Vertex Array Object (VAO).  Um VAO
    // contém a definição de vários atributos de um certo conjunto de vértices;
    // isto é, um VAO irá conter ponteiros para vários VBOs.
    GLuint vertex_array_object_id;
    glGenVertexArrays(1, &vertex_array_object_id);

    // "Ligamos" o VAO ("bind"). Informamos que iremos atualizar o VAO cujo ID
    // está contido na variável "vertex_array_object_id".
    glBindVertexArray(vertex_array_object_id);

    // "Ligamos" o VBO ("bind"). Informamos que o VBO cujo ID está contido na
    // variável VBO_model_coefficients_id será modificado a seguir. A
    // constante "GL_ARRAY_BUFFER" informa que esse buffer é de fato um VBO, e
    // irá conter atributos de vértices.
    glBindBuffer(GL_ARRAY_BUFFER, VBO_model_coefficients_id);

    // Alocamos memória para o VBO "ligado" acima. Como queremos armazenar
    // nesse VBO todos os valores contidos no array "model_coefficients", pedimos
    // para alocar um número de bytes exatamente igual ao tamanho ("size")
    // desse array. A constante "GL_STATIC_DRAW" dá uma dica para o driver da
    // GPU sobre como utilizaremos os dados do VBO. Neste caso, estamos dizendo
    // que não pretendemos alterar tais dados (são estáticos: "STATIC"), e
    // também dizemos que tais dados serão utilizados para renderizar ou
    // desenhar ("DRAW").  Pense que:
    //
    //            glBufferData()  ==  malloc() do C  ==  new do C++.
    //
    glBufferData(GL_ARRAY_BUFFER, sizeof(model_coefficients), NULL, GL_STATIC_DRAW);

    // Finalmente, copiamos os valores do array model_coefficients para dentro do
    // VBO "ligado" acima.  Pense que:
    //
    //            glBufferSubData()  ==  memcpy() do C.
    //
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(model_coefficients), model_coefficients);

    // Precisamos então informar um índice de "local" ("location"), o qual será
    // utilizado no shader "shader_vertex.glsl" para acessar os valores
    // armazenados no VBO "ligado" acima. Também, informamos a dimensão (número de
    // coeficientes) destes atributos. Como em nosso caso são pontos em coordenadas
    // homogêneas, temos quatro coeficientes por vértice (X,Y,Z,W). Isso define
    // um tipo de dado chamado de "vec4" em "shader_vertex.glsl": um vetor com
    // quatro coeficientes. Finalmente, informamos que os dados estão em ponto
    // flutuante com 32 bits (GL_FLOAT).
    // Esta função também informa que o VBO "ligado" acima em glBindBuffer()
    // está dentro do VAO "ligado" acima por glBindVertexArray().
    // Veja https://www.khronos.org/opengl/wiki/Vertex_Specification#Vertex_Buffer_Object
    GLuint location = 0; // "(location = 0)" em "shader_vertex.glsl"
    GLint  number_of_dimensions = 4; // vec4 em "shader_vertex.glsl"
    glVertexAttribPointer(location, number_of_dimensions, GL_FLOAT, GL_FALSE, 0, 0);

    // "Ativamos" os atributos. Informamos que os atributos com índice de local
    // definido acima, na variável "location", deve ser utilizado durante o
    // rendering.
    glEnableVertexAttribArray(location);

    // "Desligamos" o VBO, evitando assim que operações posteriores venham a
    // alterar o mesmo. Isso evita bugs.
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // Agora repetimos todos os passos acima para atribuir um novo atributo a
    // cada vértice: uma cor (veja slides 109-112 do documento Aula_03_Rendering_Pipeline_Grafico.pdf e slide 111 do documento Aula_04_Modelagem_Geometrica_3D.pdf).
    // Tal cor é definida como coeficientes RGBA: Red, Green, Blue, Alpha;
    // isto é: Vermelho, Verde, Azul, Alpha (valor de transparência).
    // Conversaremos sobre sistemas de cores nas aulas de Modelos de Iluminação.
    GLfloat color_coefficients[] = {
    // Cores dos vértices do cubo
    //  R     G     B     A
        1.0f, 0.5f, 0.0f, 1.0f, // cor do vértice 0
        1.0f, 0.5f, 0.0f, 1.0f, // cor do vértice 1
        0.0f, 0.5f, 1.0f, 1.0f, // cor do vértice 2
        0.0f, 0.5f, 1.0f, 1.0f, // cor do vértice 3
        1.0f, 0.5f, 0.0f, 1.0f, // cor do vértice 4
        1.0f, 0.5f, 0.0f, 1.0f, // cor do vértice 5
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
    // Cores para o cubo vermelho (todos os vértices vermelhos)
    //  R     G     B     A
        1.0f, 0.0f, 0.0f, 1.0f, // cor do vértice 18
        1.0f, 0.0f, 0.0f, 1.0f, // cor do vértice 19
        1.0f, 0.0f, 0.0f, 1.0f, // cor do vértice 20
        1.0f, 0.0f, 0.0f, 1.0f, // cor do vértice 21
        1.0f, 0.0f, 0.0f, 1.0f, // cor do vértice 22
        1.0f, 0.0f, 0.0f, 1.0f, // cor do vértice 23
        1.0f, 0.0f, 0.0f, 1.0f, // cor do vértice 24
        1.0f, 0.0f, 0.0f, 1.0f  // cor do vértice 25
    };
    GLuint VBO_color_coefficients_id;
    glGenBuffers(1, &VBO_color_coefficients_id);
    glBindBuffer(GL_ARRAY_BUFFER, VBO_color_coefficients_id);
    glBufferData(GL_ARRAY_BUFFER, sizeof(color_coefficients), NULL, GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(color_coefficients), color_coefficients);
    location = 1; // "(location = 1)" em "shader_vertex.glsl"
    number_of_dimensions = 4; // vec4 em "shader_vertex.glsl"
    glVertexAttribPointer(location, number_of_dimensions, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(location);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // Vamos então definir polígonos utilizando os vértices do array
    // model_coefficients.
    //
    // Para referência sobre os modos de renderização, veja slides 182-188 do documento Aula_04_Modelagem_Geometrica_3D.pdf.
    //
    // Este vetor "indices" define a TOPOLOGIA (veja slides 103-110 do documento Aula_04_Modelagem_Geometrica_3D.pdf).
    //
    GLuint indices[] = {
    // Definimos os índices dos vértices que definem as FACES de um cubo
    // através de 12 triângulos que serão desenhados com o modo de renderização
    // GL_TRIANGLES.
        0, 1, 2, // triângulo 1
        7, 6, 5, // triângulo 2
        3, 2, 6, // triângulo 3
        4, 0, 3, // triângulo 4
        4, 5, 1, // triângulo 5
        1, 5, 6, // triângulo 6
        0, 2, 3, // triângulo 7
        7, 5, 4, // triângulo 8
        3, 6, 7, // triângulo 9
        4, 3, 7, // triângulo 10
        4, 1, 0, // triângulo 11
        1, 6, 2, // triângulo 12
    // Definimos os índices para as FACES do chão
        14, 15, 16, // triângulo 1 (chão)
        14, 16, 17, // triângulo 2 (chão)
    // Definimos os índices dos vértices que definem as ARESTAS de um cubo
    // através de 12 linhas que serão desenhadas com o modo de renderização
    // GL_LINES.
        0, 1, // linha 1
        1, 2, // linha 2
        2, 3, // linha 3
        3, 0, // linha 4
        0, 4, // linha 5
        4, 7, // linha 6
        7, 6, // linha 7
        6, 2, // linha 8
        6, 5, // linha 9
        5, 4, // linha 10
        5, 1, // linha 11
        7, 3, // linha 12
    // Definimos os índices dos vértices que definem as linhas dos eixos X, Y,
    // Z, que serão desenhados com o modo GL_LINES.
        8 , 9 , // linha 1
        10, 11, // linha 2
        12, 13 , // linha 3
    // Índices para as faces do cubo vermelho (com offset de 18)
        18, 19, 20, // triângulo 1
        25, 24, 23, // triângulo 2
        21, 20, 24, // triângulo 3
        22, 18, 21, // triângulo 4
        22, 23, 19, // triângulo 5
        19, 23, 24, // triângulo 6
        18, 20, 21, // triângulo 7
        25, 23, 22, // triângulo 8
        21, 24, 25, // triângulo 9
        22, 21, 25, // triângulo 10
        22, 19, 18, // triângulo 11
        19, 24, 20  // triângulo 12
    };

    // Criamos um primeiro objeto virtual (SceneObject) que se refere às faces
    // coloridas do cubo.
    SceneObject cube_faces;
    cube_faces.name           = "Cubo (faces coloridas)";
    cube_faces.first_index    = (void*)0; // Primeiro índice está em indices[0]
    cube_faces.num_indices    = 36;       // Último índice está em indices[35]; total de 36 índices.
    cube_faces.rendering_mode = GL_TRIANGLES; // Índices correspondem ao tipo de rasterização GL_TRIANGLES.

    // Adicionamos o objeto criado acima na nossa cena virtual (g_VirtualScene).
    g_VirtualScene["cube_faces"] = cube_faces;


    // Criamos um objeto para o chão
    SceneObject floor_faces;
    floor_faces.name           = "Chão";
    floor_faces.first_index    = (void*)(36*sizeof(GLuint)); // Começa depois das faces do cubo (índice 36)
    floor_faces.num_indices    = 6; // 2 triângulos = 6 índices
    floor_faces.rendering_mode = GL_TRIANGLES;
    g_VirtualScene["floor"] = floor_faces;

    // Criamos um segundo objeto virtual (SceneObject) que se refere às arestas
    // pretas do cubo.
    SceneObject cube_edges;
    cube_edges.name           = "Cubo (arestas pretas)";
    cube_edges.first_index    = (void*)((36+6)*sizeof(GLuint)); // Primeiro índice está em indices[36]
    cube_edges.num_indices    = 24; // Último índice está em indices[59]; total de 24 índices.
    cube_edges.rendering_mode = GL_LINES; // Índices correspondem ao tipo de rasterização GL_LINES.

    // Adicionamos o objeto criado acima na nossa cena virtual (g_VirtualScene).
    g_VirtualScene["cube_edges"] = cube_edges;

    // Criamos um terceiro objeto virtual (SceneObject) que se refere aos eixos XYZ.
    SceneObject axes;
    axes.name           = "Eixos XYZ";
    axes.first_index    = (void*)((36+6+24)*sizeof(GLuint)); // Primeiro índice está em indices[60]
    axes.num_indices    = 6; // Último índice está em indices[65]; total de 6 índices.
    axes.rendering_mode = GL_LINES; // Índices correspondem ao tipo de rasterização GL_LINES.

    // Adicionamos o objeto criado acima na nossa cena virtual (g_VirtualScene).
    g_VirtualScene["axes"] = axes;

    // Criamos um objeto para as faces do cubo vermelho
    SceneObject damaged_cube_faces;
    damaged_cube_faces.name           = "Cubo Danificado (faces)";
    // O offset é a soma de todos os índices anteriores (cubo, chão, arestas, eixos)
    damaged_cube_faces.first_index    = (void*)((36+6+24+6)*sizeof(GLuint));
    damaged_cube_faces.num_indices    = 36;
    damaged_cube_faces.rendering_mode = GL_TRIANGLES;

    // Adicionamos o objeto criado acima na nossa cena virtual (g_VirtualScene).
    g_VirtualScene["damaged_cube_faces"] = damaged_cube_faces;

    // Criamos um buffer OpenGL para armazenar os índices acima
    GLuint indices_id;
    glGenBuffers(1, &indices_id);

    // "Ligamos" o buffer. Note que o tipo agora é GL_ELEMENT_ARRAY_BUFFER.
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indices_id);

    // Alocamos memória para o buffer.
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), NULL, GL_STATIC_DRAW);

    // Copiamos os valores do array indices[] para dentro do buffer.
    glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, sizeof(indices), indices);

    // NÃO faça a chamada abaixo! Diferente de um VBO (GL_ARRAY_BUFFER), um
    // array de índices (GL_ELEMENT_ARRAY_BUFFER) não pode ser "desligado",
    // caso contrário o VAO irá perder a informação sobre os índices.
    //
    // glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0); // XXX Errado!
    //

    // "Desligamos" o VAO, evitando assim que operações posteriores venham a
    // alterar o mesmo. Isso evita bugs.
    glBindVertexArray(0);

    // Retornamos o ID do VAO. Isso é tudo que será necessário para renderizar
    // os triângulos definidos acima. Veja a chamada glDrawElements() em main().
    return vertex_array_object_id;
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
            g_IsSwinging = true;
            g_SwingAnimationTime = 0.0f; // Reseta o tempo da animação
        }

            // Lógica para quebrar o bloco
            // Alcance da Picareta
            float reach = 1.5f;
            // Posição do alvo da picareta. Pega a posição da câmera e avança na direção do vetor de visão.
            glm::vec4 target_pos = g_CameraPosition + g_CameraViewVector * reach;

            // Converte a posição do alvo para coordenadas do grid
            int grid_x = (int)(target_pos.x + (float)map_width / 2.0f + 0.5f);
            int grid_z = (int)(target_pos.z + (float)map_height / 2.0f + 0.5f);

            // Verifica se o alvo está dentro do mapa
            if (grid_x >= 0 && grid_x < map_width && grid_z >= 0 && grid_z < map_height)
            {
                // Verifica o tipo atual do bloco e aplica ação dependendo do tipo
                if (maze_map[grid_z][grid_x] == WALL || maze_map[grid_z][grid_x] == DAMAGED_WALL)
                {
                    switch (maze_map[grid_z][grid_x])
                    {
                        // Se for uma parede, vira uma parede danificada
                        case WALL:
                            maze_map[grid_z][grid_x] = DAMAGED_WALL;
                            break;
                            
                        // Se for uma parede danificada, vira espaço vazio
                        case DAMAGED_WALL:
                            maze_map[grid_z][grid_x] = EMPTY;
                            break;
                    }
                }
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

