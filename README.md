# PICKAXEMAN - Relatório Final do Trabalho
## INF01047 - Fundamentos de Computação Gráfica - 2025/2

**Alunos:** 

Adão dos Antos Júnior - 00233028
Felipe Dal Ponte Bregalda - 00209037

**Repositório:** https://github.com/AdaoGum/PICKAXEMAN-AF-UFRGS-FCG

---

## 1. Descrição do Jogo

**PICKAXEMAN** é um jogo 3D de labirinto estilo Minecraft onde o jogador deve explorar um labirinto, coletar diamantes e escapar antes que o tempo acabe. O jogador possui uma picareta que pode ser usada para destruir paredes e criar novos caminhos.

### Objetivo
- Coletar **2 diamantes** antes que os **2 minutos** de tempo acabem
- O jogador pode destruir paredes para criar atalhos

### Controles
| Tecla | Ação |
|-------|------|
| W/A/S/D | Movimentação |
| Mouse | Rotação da câmera |
| Botão Esquerdo | Golpe de picareta |
| V | Alternar câmera (1ª/3ª pessoa) |
| C | Mostrar/esconder teto |
| H | Mostrar informações de debug |
| R | Reiniciar jogo |
| ENTER | Iniciar jogo (tela de título) |

### Mecânicas
- **Destruição de paredes**: Primeiro golpe danifica (fica vermelha), segundo golpe destrói
- **Sistema anti-cheat**: Se o jogador sair dos limites do mapa, tem 10 segundos para voltar
- **Pontuação**: 50 pontos por diamante + bônus do tempo restante ao vencer

---

## 2. Implementação dos Requisitos Técnicos

### 2.1 Malhas Poligonais Complexas ✅

O jogo utiliza **3 tipos de malhas poligonais**:

| Objeto | Origem | Arquivo |
|--------|--------|---------|
| **Cubo** | Gerado proceduralmente | `src/scene_builder.cpp` |
| **Picareta** | Modelo OBJ externo | `data/obj/pickaxe.obj` |
| **Diamante** | Modelo OBJ externo | `data/obj/diamond.obj` |

**Implementação:**
- O cubo é construído em `BuildTriangles()` com 8 vértices e 12 triângulos (linhas 21-59 de `scene_builder.cpp`)
- Os modelos OBJ são carregados usando **TinyObjLoader** (linhas 601-613 de `main.cpp`):
```cpp
ObjModel pickaxeModel("../../data/obj/pickaxe.obj");
ComputeNormals(&pickaxeModel);
BuildTrianglesAndAddToVirtualScene(&pickaxeModel);

ObjModel diamondModel("../../data/obj/diamond.obj");
ComputeNormals(&diamondModel);
BuildTrianglesAndAddToVirtualScene(&diamondModel);
```

---

### 2.2 Transformações Geométricas ✅

Utilizamos as **5 matrizes de transformação** implementadas em `include/matrices.h`:

| Matriz | Função | Uso no Jogo |
|--------|--------|-------------|
| `Matrix_Identity()` | Matriz identidade | Base para transformações |
| `Matrix_Translate(tx, ty, tz)` | Translação | Posicionar paredes, diamantes, picareta |
| `Matrix_Scale(sx, sy, sz)` | Escala | Ajustar tamanho dos diamantes (0.3x) |
| `Matrix_Rotate_X/Y/Z(angle)` | Rotação | Animação da picareta, rotação dos diamantes |

**Exemplo - Renderização do diamante (main.cpp, ~linha 1000):**
```cpp
model = Matrix_Translate(world_x, 0.3f, world_z) 
      * Matrix_Scale(0.3f, 0.3f, 0.3f) 
      * Matrix_Rotate_Y(diamond_rotation);
```

**Exemplo - Picareta com animação (main.cpp, ~linha 1240):**
```cpp
model = Matrix_Translate(0.5f, -0.5f, -1.0f)
      * animation_rotation  // Vem da curva de Bézier
      * Matrix_Rotate_Y(glm::radians(180.0f))
      * Matrix_Rotate_X(glm::radians(270.0f))
      * Matrix_Rotate_Z(glm::radians(180.0f))
      * Matrix_Scale(1.0f, 1.0f, 1.0f);
```

---

### 2.3 Controle de Câmeras ✅

O jogo implementa **duas câmeras distintas** controladas pela variável `g_UseFreeCamera`:

#### Câmera Livre (1ª Pessoa - FPS)
- **Arquivo:** `main.cpp`, linhas 912-919
- Posição acompanha o jogador (`g_CameraPosition`)
- Direção controlada por ângulos Yaw/Pitch via mouse
- Implementação:
```cpp
if (g_UseFreeCamera) {
    glm::vec4 camera_eye = g_CameraPosition + glm::vec4(0.0f, 0.0f, 0.0f, 0.0f);
    view = Matrix_Camera_View(camera_eye, g_CameraViewVector, g_CameraUpVector);
}
```

#### Câmera Look-At (3ª Pessoa)
- **Arquivo:** `main.cpp`, linhas 922-938
- Usa coordenadas esféricas para orbitar ao redor do jogador
- Distância ajustável via scroll do mouse (`g_CameraDistance`)
- Implementação:
```cpp
else {
    float r = g_CameraDistance;
    float y = r * sin(g_CameraPhi);
    float z = r * cos(g_CameraPhi) * cos(g_CameraTheta);
    float x = r * cos(g_CameraPhi) * sin(g_CameraTheta);
    
    glm::vec4 camera_lookat_point = g_CameraPosition + glm::vec4(0.0f, 1.0f, 0.0f, 0.0f);
    glm::vec4 camera_view_point = camera_lookat_point + glm::vec4(x, y, z, 0.0f);
    glm::vec4 view_vector = camera_lookat_point - camera_view_point;
    
    view = Matrix_Camera_View(camera_view_point, view_vector, g_CameraUpVector);
}
```

---

### 2.4 Instâncias de Objetos ✅

Utilizamos **instanciação** para renderizar múltiplos objetos com a mesma geometria:

- **Paredes**: O mesmo VAO do cubo é renderizado ~200 vezes com diferentes matrizes Model
- **Diamantes**: 7 instâncias do mesmo modelo OBJ em posições diferentes

**Implementação (main.cpp, linhas 1030-1080):**
```cpp
// Desenha todas as paredes do labirinto
for (int z = 0; z < MAP_HEIGHT; ++z) {
    for (int x = 0; x < MAP_WIDTH; ++x) {
        if (maze_map[z][x] == WALL || maze_map[z][x] == DAMAGED_WALL) {
            float world_x = (float)x - (float)MAP_WIDTH / 2.0f;
            float world_z = (float)z - (float)MAP_HEIGHT / 2.0f;
            
            model = Matrix_Translate(world_x, 0.0f, world_z);
            glUniformMatrix4fv(model_uniform, 1, GL_FALSE, glm::value_ptr(model));
            glDrawElements(...); // Mesmo VAO, matriz Model diferente
        }
    }
}
```

---

### 2.5 Testes de Intersecção ✅

Implementamos **3 tipos de teste de colisão** em `src/collisions.cpp`:

#### 1. Colisão Jogador-Parede (Ponto-Cubo)
- **Função:** `CheckCollision(float x, float z)`
- Verifica 4 pontos ao redor do jogador (raio de colisão = 0.2)
- Impede atravessar paredes
```cpp
bool CheckCollision(float x, float z) {
    float collision_radius = 0.2f;
    float check_points[4][2] = {
        { x + collision_radius, z },
        { x - collision_radius, z },
        { x, z + collision_radius },
        { x, z - collision_radius }
    };
    // Verifica cada ponto contra o mapa
}
```

#### 2. Colisão Jogador-Diamante (Ponto-Célula)
- **Função:** `CheckDiamondCollision(glm::vec4 camera_position)`
- Verifica se o jogador está na mesma célula do mapa que um diamante
- Remove diamante ao coletar

#### 3. Colisão Picareta-Parede (Raio-Célula)
- **Função:** `CheckMapCollisionAndBreak(glm::vec4 camera_position, glm::vec4 view_vector)`
- Projeta um raio na direção que o jogador olha
- Alcance de 1.5 unidades
- Detecta e danifica/destrói parede à frente

---

### 2.6 Modelos de Iluminação ✅

Implementamos o modelo **Blinn-Phong** com 3 componentes em `shader_fragment.glsl`:

#### Componente Ambiente
```glsl
vec3 Ka = Kd * 0.5;  // 50% da cor difusa
```

#### Componente Difusa (Lambert)
```glsl
float lambert = max(0, dot(n, l));
```

#### Componente Especular (Blinn-Phong)
```glsl
vec4 h = normalize(l + v);  // Half-vector
float specular_term = pow(max(0, dot(n, h)), q);
if (lambert <= 0.0) specular_term = 0.0;  // Sem especular em sombra
```

**Configuração por objeto:**
| Objeto | Ks (especular) | q (expoente) |
|--------|----------------|--------------|
| Paredes/Chão/Teto | 0.1 | 10 |
| Diamante | 0.5 | 10 |
| Picareta | 0.3 | 20 |

---

### 2.7 Modelos de Interpolação ✅

Implementamos **dois modelos de interpolação** para demonstrar a diferença:

#### Interpolação de Phong (Por Pixel)
- **Usado em:** Paredes, Chão, Teto, Diamante
- **Arquivo:** `shader_fragment.glsl`, linhas 170-198
- Iluminação calculada no **Fragment Shader** para cada pixel
```glsl
// MODELO DE PHONG - Iluminação calculada por pixel
color.rgb = Ka + (Kd * lambert * I) + (Ks * specular_term * I);
```

#### Interpolação de Gouraud (Por Vértice)
- **Usado em:** **Picareta** (objeto com muitos vértices para visualizar a diferença)
- **Arquivo:** `shader_vertex.glsl`, linhas 47-77
- Iluminação calculada no **Vertex Shader** e interpolada

**Vertex Shader (cálculo por vértice):**
```glsl
if (object_id == OBJ_PICKAXE) {
    gouraud_ambient = 0.4;
    gouraud_diffuse = max(0.0, dot(n, l));
    
    vec4 h = normalize(l + v);
    gouraud_specular = pow(max(0.0, dot(n, h)), 20.0);
}
```

**Fragment Shader (aplicação dos valores interpolados):**
```glsl
if (object_id == OBJ_PICKAXE) {
    // GOURAUD: Usa fatores calculados por vértice e interpolados
    color.rgb = (Kd * gouraud_ambient) + (Kd * gouraud_diffuse * I) + (Ks * gouraud_specular * I);
}
```

---

### 2.8 Mapeamento de Texturas ✅

Implementamos **6 texturas** com diferentes técnicas de mapeamento:

| Objeto | Textura | Técnica de Mapeamento |
|--------|---------|----------------------|
| Paredes | 425.jpg | **Triplanar Simplificado** |
| Chão | gravelstones.jpg | Planar (XZ) com tiling 4x |
| Teto | grayrocks.jpg | Planar (XZ) com tiling 4x |
| Picareta | wood.jpg | Coordenadas UV do OBJ |
| Diamante | diamond_obj.png | **Triplanar Completo** |
| Tela Título | title_screen.png | UV direto |

#### Mapeamento Triplanar Simplificado (Paredes)
Escolhe a melhor projeção baseado na normal da face:
```glsl
vec3 absN = abs(n.xyz);
if (absN.x > absN.y && absN.x > absN.z) {
    U = position_world.z; V = position_world.y;  // Paredes laterais
} else if (absN.y > absN.x && absN.y > absN.z) {
    U = position_world.x; V = position_world.z;  // Topo/Baixo
} else {
    U = position_world.x; V = position_world.y;  // Frente/Trás
}
```

#### Mapeamento Triplanar Completo (Diamante)
Mistura 3 projeções baseado na normal para transições suaves:
```glsl
vec3 blending = abs(n.xyz);
blending = normalize(max(blending, 0.00001));
blending /= (blending.x + blending.y + blending.z);

vec3 xaxis = texture(TextureImage4, position_model.yz * tiling).rgb;
vec3 yaxis = texture(TextureImage4, position_model.xz * tiling).rgb;
vec3 zaxis = texture(TextureImage4, position_model.xy * tiling).rgb;

Kd = xaxis * blending.x + yaxis * blending.y + zaxis * blending.z;
```

---

### 2.9 Curvas de Bézier ✅

A animação da picareta utiliza uma **Curva de Bézier Cúbica** para movimento suave.

**Arquivo:** `main.cpp`, linhas 405-423

#### Fórmula Implementada
$$c(t) = (1-t)^3 P_0 + 3t(1-t)^2 P_1 + 3t^2(1-t) P_2 + t^3 P_3$$

```cpp
glm::vec3 BezierCubic(float t, glm::vec3 P0, glm::vec3 P1, glm::vec3 P2, glm::vec3 P3) {
    glm::vec3 point = (1.0f - t) * (1.0f - t) * (1.0f - t) * P0;
    point += 3.0f * t * (1.0f - t) * (1.0f - t) * P1;
    point += 3.0f * t * t * (1.0f - t) * P2;
    point += t * t * t * P3;
    return point;
}
```

#### Pontos de Controle
```cpp
const glm::vec3 g_BezierP0 = glm::vec3(0.0f, 0.0f, 0.0f);      // Repouso
const glm::vec3 g_BezierP1 = glm::vec3(-30.0f, 10.0f, 0.1f);   // Levanta e puxa
const glm::vec3 g_BezierP2 = glm::vec3(-70.0f, -5.0f, -0.1f);  // Prepara golpe
const glm::vec3 g_BezierP3 = glm::vec3(-90.0f, 0.0f, -0.2f);   // Golpe final
```

A animação usa os valores retornados como:
- `bezier_anim.x` → Rotação em X (swing principal)
- `bezier_anim.y` → Rotação em Y (movimento lateral)
- `bezier_anim.z` → Deslocamento em Z (avança/recua)

---

### 2.10 Animação Baseada em Tempo ✅

Toda movimentação e animação é **independente de framerate** usando Delta Time.

**Arquivo:** `main.cpp`, linhas 750-755

```cpp
// Cálculo do Delta Time
float currentFrameTime = (float)glfwGetTime();
g_DeltaTime = currentFrameTime - g_LastFrameTime;
g_LastFrameTime = currentFrameTime;
```

#### Aplicações:

**Movimento do jogador:**
```cpp
float move_speed = 4.0f * g_DeltaTime;  // 4 unidades por segundo
```

**Animação da picareta:**
```cpp
float swing_speed = 3.0f;
g_SwingAnimationTime += swing_speed * g_DeltaTime;
```

**Rotação dos diamantes:**
```cpp
float diamond_rotation = (float)glfwGetTime() * 1.0f;  // 1 rad/s
```

---

## 3. Estrutura do Projeto

```
PICKAXEMAN-AF-UFRGS-FCG/
├── src/
│   ├── main.cpp              # Loop principal, renderização, câmeras, input
│   ├── shader_vertex.glsl    # Vertex shader (transformações, Gouraud)
│   ├── shader_fragment.glsl  # Fragment shader (Phong, texturas, triplanar)
│   ├── collisions.cpp        # Sistema de colisão e mapa do labirinto
│   ├── scene_builder.cpp     # Construção de geometria (cubo, chão, teto)
│   └── textrendering.cpp     # Renderização de texto na tela
├── include/
│   ├── matrices.h            # Matrizes de transformação (Translate, Scale, Rotate)
│   ├── collisions.h          # Declarações do sistema de colisão
│   ├── scene_builder.h       # Declarações do construtor de cena
│   ├── tiny_obj_loader.h     # Biblioteca para carregar OBJ
│   └── stb_image.h           # Biblioteca para carregar texturas
├── data/
│   ├── obj/                  # Modelos 3D
│   │   ├── pickaxe.obj       # Picareta
│   │   └── diamond.obj       # Diamante
│   └── textures/             # Texturas
│       ├── 425.jpg           # Paredes
│       ├── gravelstones.jpg  # Chão
│       ├── grayrocks.jpg     # Teto
│       ├── wood.jpg          # Picareta
│       ├── diamond_obj.png   # Diamante
│       └── title_screen.png  # Tela de título
└── build/                    # Arquivos de compilação (CMake)
```

---

## 4. Compilação e Execução

### Requisitos
- CMake 3.10+
- Compilador C++ com suporte a C++11
- OpenGL 3.3+

### Windows (MinGW)
```bash
cd build
cmake ..
cmake --build . --config Release
cd ../bin/Release
./main.exe
```

### Linux
```bash
cd build
cmake ..
make
cd ../bin/Linux
./main
```

---

## 5. Contribuições

| Contribuições | Membro |
|------------------|--------------|
| Malhas poligonais complexas | Adão + Felipe |
| Transformações geométricas controladas pelo usuário | Felipe |
| Câmera livre e câmera look-at | Felipe |
| Instâncias de objetos | Adão + Felipe |
| Três tipos de testes de intersecção | Adão + Felipe |
| Modelos de Iluminação Difusa e Blinn-Phong | Adão + Felipe |
| Modelos de Interpolação de Phong e Gouraud | Adão |
| Mapeamento de texturas em todos os objetos | Adão + Felipe |
| Movimentação com curva Bézier cúbica | Adão |
| Animações baseadas no tempo (Δt) | Adão + Felipe |

---

## 6. Uso de IA Generativa

Utilizamos o GitHub Copilot e o Gemini para auxiliar em dúvidas no geral, debbugar e na estrutura de arquivos, algumas partes das colisões, na correção de bugs no mapeamento de texturas e A IA foi útil para explicar conceitos e sugerir soluções, mas todo o código foi revisado e adaptado para o contexto do projeto.

---

## 7. Referências

- Laboratórios da disciplina INF01047
- OpenGL Programming Guide
- Learn OpenGL (https://learnopengl.com)
- TinyObjLoader (https://github.com/tinyobjloader/tinyobjloader)
- STB Image (https://github.com/nothings/stb)
