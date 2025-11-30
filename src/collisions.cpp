// collisions.cpp
#include "collisions.h"
#include <cmath> 

// Definição do mapa
// 0 = espaço vazio, 1 = parede
int maze_map[MAP_HEIGHT][MAP_WIDTH] = {
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

// Implementação da função de colisão
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
            // Se for 1, é parede
            if (maze_map[grid_z][grid_x] == 1)
                return true;
        }
    }
    return false;
}