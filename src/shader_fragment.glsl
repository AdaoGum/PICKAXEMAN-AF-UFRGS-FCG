#version 330 core

in vec4 position_world;
in vec4 normal;
in vec4 position_model;
in vec2 texcoords;

// Fatores de iluminação interpolados (calculados por vértice no vertex shader)
in float gouraud_diffuse;
in float gouraud_specular;
in float gouraud_ambient;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

// Identificadores
#define OBJ_CUBE_WALL  0
#define OBJ_CUBE_FLOOR 1
#define OBJ_PICKAXE    2
#define OBJ_CEILING    3
#define OBJ_DIAMOND    4

uniform int object_id;
uniform bool render_as_black;
// Flag para paredes danificadas 0 = intacta, 1 = danificada
uniform int is_damaged;

// Texturas
uniform sampler2D TextureImage0; // Pedra
uniform sampler2D TextureImage1; // Grama
uniform sampler2D TextureImage2; // Madeira
uniform sampler2D TextureImage3; // Teto (graystones)
uniform sampler2D TextureImage4; // Diamante

// Iluminação
uniform vec4 bbox_min;
uniform vec4 bbox_max;

out vec4 color;

void main()
{
    if (render_as_black)
    {
        color = vec4(0.0, 0.0, 0.0, 1.0);
        return;
    }

    // --- 1. Configuração da Câmera ---
    vec4 origin = vec4(0.0, 0.0, 0.0, 1.0);
    vec4 camera_position = inverse(view) * origin;
    vec4 p = position_world;

    // --- CORREÇÃO MÁGICA DAS NORMAIS ---
    // Para o labirinto (cubos), calculamos a normal real da face (Flat Shading)
    // usando derivadas (dFdx, dFdy). Isso corrige os riscos nas paredes.
    // Para a picareta (OBJ), usamos a normal suave original do arquivo.
    vec4 n;
    if (object_id == OBJ_PICKAXE)
    {
        n = normalize(normal);
    }
    else
    {
        vec3 dx = dFdx(p.xyz);
        vec3 dy = dFdy(p.xyz);
        n = vec4(normalize(cross(dx, dy)), 0.0);
    }

    vec4 v = normalize(camera_position - p);
    vec4 l = normalize(vec4(0.5, 1.0, 0.5, 0.0));

    // --- 2. Mapeamento de Textura (U, V) ---
    float U = 0.0;
    float V = 0.0;

    // Fator de repetição (aumente se quiser ladrilhos menores)
    float tiling = 1.0;

    if (object_id == OBJ_CUBE_WALL)
    {
        // Mapeamento Planar Automático (Triplanar)
        // Decide qual eixo usar baseado na Normal calculada acima
        vec3 absN = abs(n.xyz);

        if (absN.x > absN.y && absN.x > absN.z)
        {
            // Paredes laterais (X)
            U = position_world.z;
            V = position_world.y;
        }
        else if (absN.y > absN.x && absN.y > absN.z)
        {
            // Topo/Baixo (Y)
            U = position_world.x;
            V = position_world.z;
        }
        else
        {
            // Paredes frente/trás (Z)
            U = position_world.x;
            V = position_world.y;
        }
    }
    else if (object_id == OBJ_CUBE_FLOOR)
    {
        // Chão
        U = position_world.x;
        V = position_world.z;
        tiling = 4.0; // Exemplo: Repete a textura 4x mais no chão
    }
    else if (object_id == OBJ_CEILING)
    {
        // Teto - usa coordenadas X/Z como o chão
         U = position_world.x;
        V = position_world.z;
        tiling = 4.0;
    }
    else if (object_id == OBJ_DIAMOND)
    {
        // Diamante - usa mapeamento triplanar (será processado depois)
        U = 0.0;
        V = 0.0;
        tiling = 0.05; // Escala para o tamanho do diamante
    }
    else
    {
        // Picareta
        U = texcoords.x;
        V = texcoords.y;
    }

    // --- 3. Cor e Iluminação ---
    vec3 Kd;
    if (object_id == OBJ_CUBE_WALL)
        Kd = texture(TextureImage0, vec2(U, V) * tiling).rgb;
    else if (object_id == OBJ_CUBE_FLOOR)
        Kd = texture(TextureImage1, vec2(U, V) * tiling).rgb;
    else if (object_id == OBJ_CEILING)
        Kd = texture(TextureImage3, vec2(U, V) * tiling).rgb;
    else if (object_id == OBJ_DIAMOND)
    {
        // Mapeamento Triplanar para o diamante
        vec3 blending = abs(n.xyz);
        blending = normalize(max(blending, 0.00001));
        float b = (blending.x + blending.y + blending.z);
        blending /= b;

        // Amostra a textura de 3 direções
        vec3 xaxis = texture(TextureImage4, position_model.yz * tiling).rgb;
        vec3 yaxis = texture(TextureImage4, position_model.xz * tiling).rgb;
        vec3 zaxis = texture(TextureImage4, position_model.xy * tiling).rgb;

        // Mistura baseado na normal
        Kd = xaxis * blending.x + yaxis * blending.y + zaxis * blending.z;
    }
    else
        Kd = texture(TextureImage2, vec2(U, V)).rgb;

    // Iluminação Blinn-Phong
    vec3 Ks = vec3(0.1, 0.1, 0.1);
    // Para o diamante, mais brilho especular
    if (object_id == OBJ_DIAMOND)
        Ks = vec3(0.5, 0.5, 0.5);
    float q = 10.0;
    vec3 Ka = Kd * 0.5; // Ambiente
    vec3 I = vec3(1.0, 1.0, 1.0);

    float lambert = max(0, dot(n, l));
    vec4 h = normalize(l + v);
    float specular_term = pow(max(0, dot(n, h)), q);

    if (lambert <= 0.0) specular_term = 0.0;

    // Se for picareta, usa modelo de Gouraud, senão Phong
    if (object_id == OBJ_PICKAXE)
    {
        // ========== MODELO DE GOURAUD ==========
        // Iluminação foram calculados por vertice no vertex shader e INTERPOLADOS para cada pixel. Aqui aplicamos esses fatores à TEXTURA.
        vec3 Ks_pickaxe = vec3(0.3, 0.3, 0.3);
        color.rgb = (Kd * gouraud_ambient) + (Kd * gouraud_diffuse * I) + (Ks_pickaxe * gouraud_specular * I);
    }
    else
    {
        // ========== MODELO DE PHONG ==========
        // Iluminação calculada por pixel
        color.rgb = Ka + (Kd * lambert * I) + (Ks * specular_term * I);
    }

    // Se a parede está danificada, mistura com cor vermelha
    if (is_damaged == 1 && object_id == OBJ_CUBE_WALL)
    {
        // Vermelho
        vec3 damaged_color = vec3(0.8, 0.2, 0.2);

        // 50% de mistura com vermelho
        color.rgb = mix(color.rgb, damaged_color, 0.5);
    }

    color.a = 1.0;
    color.rgb = pow(color.rgb, vec3(1.0/2.2));
}
