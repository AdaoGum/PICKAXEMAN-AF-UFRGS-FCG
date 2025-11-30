#version 330 core

in vec4 position_world;
in vec4 normal;
in vec4 position_model;
in vec2 texcoords;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

// Identificadores
#define OBJ_CUBE_WALL  0
#define OBJ_CUBE_FLOOR 1
#define OBJ_PICKAXE    2

uniform int object_id;
uniform bool render_as_black;
// Flag para paredes danificadas 0 = intacta, 1 = danificada
uniform int is_damaged;

// Texturas
uniform sampler2D TextureImage0; // Pedra
uniform sampler2D TextureImage1; // Grama
uniform sampler2D TextureImage2; // Madeira

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
    else
        Kd = texture(TextureImage2, vec2(U, V)).rgb;

    // Iluminação Blinn-Phong
    vec3 Ks = vec3(0.1, 0.1, 0.1);
    float q = 10.0;
    vec3 Ka = Kd * 0.5; // Ambiente
    vec3 I = vec3(1.0, 1.0, 1.0); 

    float lambert = max(0, dot(n, l));
    vec4 h = normalize(l + v);
    float specular_term = pow(max(0, dot(n, h)), q);

    if (lambert <= 0.0) specular_term = 0.0;

    color.rgb = Ka + (Kd * lambert * I) + (Ks * specular_term * I);

    // Se a parede está danificada, mistura com cor vermelha
    if (is_damaged == 1 && object_id == OBJ_CUBE_WALL)
    {
        vec3 damaged_color = vec3(0.8, 0.2, 0.2); // Vermelho
        color.rgb = mix(color.rgb, damaged_color, 0.5); // 50% de mistura com vermelho
    }

    color.a = 1.0;
    color.rgb = pow(color.rgb, vec3(1.0/2.2));
}