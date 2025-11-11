# Alterações feitas no arquivo original do Laboratório 2

Abaixo estão as modificações realizadas no arquivo do laboratório 2, organizadas por seções com trechos de código correspondentes.

---

## 1. Posição inicial da câmera

Posição inicial alterada para dentro do labirinto:

```cpp
glm::vec4 g_CameraPosition     = glm::vec4(-4.0f, 0.0f, -4.0f, 1.0f); // Posição inicial da câmera
glm::vec4 g_CameraViewVector   = glm::vec4(0.0f, 0.0f, -1.0f, 0.0f); // Vetor "view", para onde a câmera está virada
glm::vec4 g_CameraUpVector     = glm::vec4(0.0f, 1.0f, 0.0f, 0.0f); // Vetor "up"
```

---

## 2. Definição do mapa do labirinto

Mapa definido como uma matriz 10x10, onde 0 = espaço vazio e 1 = parede:

```cpp
// 0 = espaço vazio, 1 = parede 
const int map_width = 10;
const int map_height = 10;
int maze_map[map_height][map_width] = {
    {1,1,1,1,1,1,1,1,1,1},
    {1,0,0,0,0,0,0,0,0,1},
    {1,0,1,1,0,1,1,1,0,1},
    {1,0,1,0,0,0,1,0,0,1},
    {1,0,1,0,1,0,1,0,1,1},
    {1,0,0,0,1,0,0,0,0,1},
    {1,1,1,1,1,0,1,1,0,1},
    {1,0,0,0,0,0,0,1,0,1},
    {1,0,1,1,1,1,0,0,0,1},
    {1,1,1,1,1,1,1,1,1,1}
};
```

---

## 3. Desenha o chão

Trecho responsável pelo desenho do objeto "floor":

```cpp
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
```

---

## 4. Desenha o labirinto com base no `maze_map`

Loop que percorre o mapa e desenha cubos onde há paredes (valor 1):

```cpp
// (Substitui o loop for (int i = 1; i <= 3; ++i))
for (int z = 0; z < map_height; ++z)
{
    for (int x = 0; x < map_width; ++x)
    {
        // Se for uma parede (1)
        if (maze_map[z][x] == 1)
        {
            // Os cubos têm 1x1x1, e eles vão de -0.5 até 0.5                             
            // Calcula a posição no mundo, centrando o labirinto em (0,0)

            float pos_x = (float)x - (float)map_width / 2.0f;
            float pos_z = (float)z - (float)map_height / 2.0f;

            glm::mat4 model = Matrix_Translate(pos_x, 0.0f, pos_z);

            // Envia a matriz "model" para a placa de vídeo (GPU).
            glUniformMatrix4fv(model_uniform, 1, GL_FALSE, glm::value_ptr(model));

            // Desenha as faces coloridas
            glUniform1i(render_as_black_uniform, false);
            glDrawElements(
                g_VirtualScene["cube_faces"].rendering_mode,
                g_VirtualScene["cube_faces"].num_indices,
                GL_UNSIGNED_INT,
                (void*)g_VirtualScene["cube_faces"].first_index
            );

            // Desenha as arestas pretas
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
```

---

## 5. Desenha a picareta (HUD / item em frente à câmera)

A picareta é desenhada usando uma View identidade e garantindo que ela apareça sempre à frente (limpando o Z-buffer). O código desenha separadamente o cabo e a cabeça com suas transformações:

```cpp
{
    // Limpa o Z-buffer para garantir que a picareta sempre apareça na frente
    glClear(GL_DEPTH_BUFFER_BIT);

    // Usamos uma matriz View identidade para desenhar em "Camera Space"
    glm::mat4 pickaxe_view = Matrix_Identity();
    glUniformMatrix4fv(view_uniform, 1, GL_FALSE, glm::value_ptr(pickaxe_view));

    // Modelo do Cabo da Picareta
    glm::mat4 handle_model = Matrix_Translate(0.4f, -0.3f, -1.0f) // Posição na tela
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
                         * Matrix_Rotate_Z(glm::radians(-30.0f))    // Mesma Rotação
                         * Matrix_Translate(0.0f, 0.25f, 0.0f)      // Desloca para a ponta do cabo
                         * Matrix_Scale(0.3f, 0.08f, 0.08f);       // Tamanho (largo e fino)

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
```

---

## 6. Vértices para o chão (y = -0.5)

Vertices do chão (coordenadas X, Y, Z, W):

```text
//    X      Y     Z     W
-50.0f, -0.5f, -50.0f, 1.0f, // vértice 14
 50.0f, -0.5f, -50.0f, 1.0f, // vértice 15
 50.0f, -0.5f,  50.0f, 1.0f, // vértice 16
-50.0f, -0.5f,  50.0f, 1.0f  // vértice 17
```

---

## 7. Cores para o chão (verde)

Cores RGBA atribuídas aos vértices do chão:

```text
0.3f, 0.7f, 0.2f, 1.0f, // cor do vértice 14
0.3f, 0.7f, 0.2f, 1.0f, // cor do vértice 15
0.3f, 0.7f, 0.2f, 1.0f, // cor do vértice 16
0.3f, 0.7f, 0.2f, 1.0f  // cor do vértice 17
```

---

## 8. Índices das faces do chão

Índices que formam os dois triângulos do chão:

```text
14, 15, 16, // triângulo 1 (chão)
14, 16, 17, // triângulo 2 (chão)
```

---

## 9. Criação do objeto do chão na cena

Objeto `SceneObject` criado para o chão, com ajustes no primeiro índice e número de índices:

```cpp
SceneObject floor_faces;
floor_faces.name           = "Chão";
floor_faces.first_index    = (void*)(36*sizeof(GLuint)); // Começa depois das faces do cubo (índice 36)
floor_faces.num_indices    = 6; // 2 triângulos = 6 índices
floor_faces.rendering_mode = GL_TRIANGLES;
g_VirtualScene["floor"] = floor_faces;
```

---

## 10. Atualização dos índices iniciais (offsets)

Ajustes nos offsets dos índices para que as arestas e os eixos comecem após os blocos já definidos:

```cpp
cube_edges.first_index    = (void*)((36+6)*sizeof(GLuint)); // Começa em indices[42]
axes.first_index          = (void*)((36+6+24)*sizeof(GLuint)); // Começa em indices[66]
```

---

## 11. Atualiza ao não pressionar o botão esquerdo

Atualização da posição do cursor quando o botão esquerdo não está pressionado:

```cpp
glfwGetCursorPos(window, &g_LastCursorPosX, &g_LastCursorPosY);
return;
```

---

## 12. Movimento da Picareta

Agora vamos fazer a picareta se mexer

 Variáveis para animação da picareta
 
 ```cpp
bool g_IsSwinging = false;         // Controla se a animação está ativa
float g_SwingAnimationTime = 0.0f; // Controla o progresso da animação (em radianos)

float g_DeltaTime = 0.0f;
float g_LastFrameTime = 0.0f;

Apertar o botão de animação da picareta
    if (key == GLFW_KEY_Q && action == GLFW_PRESS)
    {
        // Só começa uma nova animação se não houver uma em andamento
        if (!g_IsSwinging)
        {
            g_IsSwinging = true;
            g_SwingAnimationTime = 0.0f; // Reseta o tempo da animação
        }
    }
```


Inicializa o timer da animação antes do loop

 ```cpp
g_LastFrameTime = (float)glfwGetTime();
```


Cálculo da animação e movimento da animação

 ```cpp
      float currentFrameTime = (float)glfwGetTime();
        g_DeltaTime = currentFrameTime - g_LastFrameTime;
        g_LastFrameTime = currentFrameTime;

        // Atualização da Animação
        if (g_IsSwinging)
        {
            float swing_speed = 10.0f; // Velocidade da animação (radianos/seg)
            g_SwingAnimationTime += swing_speed * g_DeltaTime;

            // Usamos PI porque a função sin(x) de 0 a PI (0 -> 1 -> 0).
            if (g_SwingAnimationTime >= 3.141592f)
            {
                g_IsSwinging = false;
                g_SwingAnimationTime = 0.0f;
            }
        }
 ```

Usamos g_DeltaTime para velocidade constante

 ```cpp
float camera_speed = 4.0f * g_DeltaTime; // 4.0 unidades por segundo

Cálculo da rotação da animação
            float swing_angle_rad = 0.0f;
            if (g_IsSwinging)
            {
                float max_swing_angle = glm::radians(45.0f);
                swing_angle_rad = sin(g_SwingAnimationTime) * max_swing_angle;
            }
            // Criamos uma matriz de rotação para a animação (em torno do eixo X)
            glm::mat4 animation_rotation = Matrix_Rotate_X(swing_angle_rad);
```

Multiplicamos nos modelos da cabeça e do cabo da picareta
