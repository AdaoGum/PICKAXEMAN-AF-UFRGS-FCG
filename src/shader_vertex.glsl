#version 330 core

// Atributos do vértice (layout locations devem bater com o C++)
layout (location = 0) in vec4 model_coefficients;
layout (location = 1) in vec4 normal_coefficients;
layout (location = 2) in vec2 texture_coefficients;

// Matrizes
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

// Identificador do objeto (para Gouraud)
uniform int object_id;
#define OBJ_PICKAXE 2

// Saídas para o Fragment Shader
out vec4 position_world;
out vec4 position_model;
out vec4 normal;
out vec2 texcoords;

// Fatores de iluminação calculados POR VÉRTICE e interpolados para cada pixel
// Fator difuso (Lambert)
out float gouraud_diffuse;   
 // Fator especular (Blinn-Phong)
out float gouraud_specular; 
// Fator ambiente
out float gouraud_ambient;   

void main()
{
    gl_Position = projection * view * model * model_coefficients;

    position_world = model * model_coefficients;
    position_model = model_coefficients;
    
    // Importante: Normal precisa ser transformada com a inversa transposta se houver escala não uniforme
    // Para simplificar neste laboratório, usaremos a model mesmo.
    normal = model * normal_coefficients;
    normal.w = 0.0;

    texcoords = texture_coefficients;

    // Gouraud Calcula FATORES de iluminação por vértice (serão interpolados para pixels). A textura será amostrada no fragment shader e multiplicada por estes fatores
    if (object_id == OBJ_PICKAXE)
    {
        // Posição do vértice no mundo
        vec4 p = position_world;
        
        // Normal do vértice (normalizada)
        vec4 n = normalize(normal);
        
        // Posição da câmera
        vec4 origin = vec4(0.0, 0.0, 0.0, 1.0);
        vec4 camera_position = inverse(view) * origin;
        
        // Vetor view (do ponto para a câmera)
        vec4 v = normalize(camera_position - p);
        
        // Direção da luz (fixa, vindo de cima/diagonal)
        vec4 l = normalize(vec4(0.5, 1.0, 0.5, 0.0));
        
        // Fator ambiente
        gouraud_ambient = 0.4;
        
        // Componente difusa (Lambert) - calculada por vértice!
        gouraud_diffuse = max(0.0, dot(n, l));
        
        // Componente especular (Blinn-Phong) - calculada por vértice!
        vec4 h = normalize(l + v);
        float q = 20.0; // Expoente especular
        gouraud_specular = pow(max(0.0, dot(n, h)), q);
        if (gouraud_diffuse <= 0.0) gouraud_specular = 0.0;
    }
    else
    {
        // Para outros objetos, passa zero (iluminação calculada no fragment - Phong)
        gouraud_ambient = 0.0;
        gouraud_diffuse = 0.0;
        gouraud_specular = 0.0;
    }
}