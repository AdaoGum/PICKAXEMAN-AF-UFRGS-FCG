// collisions.cpp
#include "collisions.h"
#include <cmath> 
#include "collisions.h"
#include <cmath>
#include <cstdio>
#include <algorithm>
#include <glm/geometric.hpp> 

// Definição do mapa
// 0 = EMPTY (espaço vazio)
// 1 = WALL (parede intacta)
// 2 = DAMAGED_WALL (parede danificada - aparece vermelha)
// 3 = DIAMOND (diamante coletável)
const int original_maze_map[MAP_HEIGHT][MAP_WIDTH] = {
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
    {2,0,3,3,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,1,1,1,1,1,1,1,0,0,1,1,1,1,1,0,0,1,1,1},
    {1,1,1,0,1,1,1,0,0,0,0,0,0,0,1,0,0,1,3,1},
    {1,1,1,0,1,1,1,1,0,0,1,1,1,0,1,0,0,1,1,1},
    {1,1,1,1,1,1,1,0,0,0,1,0,0,0,1,0,0,0,0,1}, 
    {1,1,3,1,1,1,1,0,0,0,1,0,1,1,1,0,0,1,0,1},
    {1,1,1,1,1,1,1,0,0,0,1,0,1,1,1,0,0,0,0,1},
    {1,1,1,1,1,1,1,0,0,1,1,0,1,1,1,1,1,1,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,1},
    {1,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,1,0,1},
    {1,0,0,0,1,1,1,1,1,1,1,1,1,1,0,1,0,1,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,0,1},
    {1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,1,1,3,0,1},
    {1,1,1,1,0,0,1,1,1,0,0,1,1,1,1,1,1,1,1,1},
    {1,3,1,1,0,0,1,1,1,0,0,1,1,1,1,1,1,1,1,1},
    {1,1,1,1,0,0,0,0,0,0,0,1,0,0,0,0,0,1,3,1}, 
    {1,1,1,1,0,0,0,0,0,0,0,1,0,1,1,1,0,1,0,1},
    {1,1,1,1,1,1,1,1,1,0,0,1,1,1,1,1,1,1,1,1}
};

// Definição da matriz do mapa que será uma copia da original e poderá ser modificada
int maze_map[MAP_HEIGHT][MAP_WIDTH];

// Reinicia o mapa para o original
void ResetMap() {
    for (int z = 0; z < MAP_HEIGHT; z++) {
        for (int x = 0; x < MAP_WIDTH; x++) {
            maze_map[z][x] = original_maze_map[z][x];
        }
    }
}

// Implementação da função de colisão
// Retorna true se houver colisão com WALL (1) ou DAMAGED_WALL (2)
bool CheckCollision(float x, float z)
{
    // Raio do corpo do personagem
    float collision_radius = 0.2f; 

    // Verificamos 4 pontos ao redor da posição
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
        // A lógica é inversa ao desenho: grid_x = world_x + width/2
        int grid_x = (int)(px + (float)MAP_WIDTH / 2.0f + 0.5f);
        int grid_z = (int)(pz + (float)MAP_HEIGHT / 2.0f + 0.5f);

        // Verifica se está dentro dos limites do mapa
        if (grid_x >= 0 && grid_x < MAP_WIDTH && grid_z >= 0 && grid_z < MAP_HEIGHT)
        {
            int cell_value = maze_map[grid_z][grid_x];
            // Colide com WALL (1) ou DAMAGED_WALL (2)
            if (cell_value == 1 || cell_value==2)
                return true;
        }
    }
    return false;
}

void CheckMapCollisionAndBreak(glm::vec4 camera_position, glm::vec4 view_vector)
{
    // Distância de alcance da picareta
    float reach_distance = 1.5f; 

    // Direção que o jogador está olhando (sem componente Y para simplificar o acerto no grid)
    glm::vec4 look_direction = glm::vec4(view_vector.x, 0.0f, view_vector.z, 0.0f);
    
    if (glm::length(look_direction) > 0.0f)
        look_direction = glm::normalize(look_direction);

    // Posição do bloco à frente (Origem + Direção * Distância)
    float target_x = camera_position.x + look_direction.x * reach_distance;
    float target_z = camera_position.z + look_direction.z * reach_distance;

    // Converte coordenadas de mundo para índices do mapa
    int grid_x = (int)(target_x + (float)MAP_WIDTH / 2.0f + 0.5f);
    int grid_z = (int)(target_z + (float)MAP_HEIGHT / 2.0f + 0.5f);

    // Verifica se o índice é válido
    if (grid_x >= 0 && grid_x < MAP_WIDTH && grid_z >= 0 && grid_z < MAP_HEIGHT)
    {
        // Lógica de dano progressivo
        if (maze_map[grid_z][grid_x] == WALL)
        {
            maze_map[grid_z][grid_x] = DAMAGED_WALL;
            printf("Parede danificada em (%d, %d)!\n", grid_x, grid_z);
        }
        else if (maze_map[grid_z][grid_x] == DAMAGED_WALL)
        {
            maze_map[grid_z][grid_x] = EMPTY;
            printf("Parede destruida em (%d, %d)!\n", grid_x, grid_z);
        }
    }
}

// Verifica colisao com diamante
bool CheckDiamondCollision(glm::vec4 camera_position)
{
    // Converte posicao do mundo para indice da matriz
    int player_grid_x = (int)(camera_position.x + (float)MAP_WIDTH / 2.0f + 0.5f);
    int player_grid_z = (int)(camera_position.z + (float)MAP_HEIGHT / 2.0f + 0.5f);

    // Verifica limites do mapa
    if (player_grid_x >= 0 && player_grid_x < MAP_WIDTH && 
        player_grid_z >= 0 && player_grid_z < MAP_HEIGHT)
    {
        // Se o bloco onde o jogador está é um DIAMANTE
        if (maze_map[player_grid_z][player_grid_x] == DIAMOND)
        {
            // Remove o diamante do mapa
            maze_map[player_grid_z][player_grid_x] = EMPTY;
            
            // Diamante coletado!
            printf("Diamante coletado em (%d, %d)!\n", player_grid_x, player_grid_z);
            return true; 
        }
    }
    return false;
}