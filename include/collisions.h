#ifndef COLLISIONS_H
#define COLLISIONS_H

#include <glm/vec4.hpp> // Necessário para receber a posição da câmera

// Definição das dimensões do mapa
#define MAP_WIDTH 20
#define MAP_HEIGHT 20

// Tipos de células do mapa (Movido do main.cpp para cá para ser global)
enum MapType { EMPTY = 0, WALL = 1, DAMAGED_WALL = 2, DIAMOND = 3 };

// Declaração da matriz do mapa. É declarada aqui para ser acessível em outras partes do código.
extern int maze_map[MAP_HEIGHT][MAP_WIDTH];  
void ResetMap();

// Funções
bool CheckCollision(float x, float z); // Colisão ponto/colisão (andar)
void CheckMapCollisionAndBreak(glm::vec4 camera_position, glm::vec4 view_vector); // Ray (quebrar parede)
bool CheckDiamondCollision(glm::vec4 camera_position); // Colisão esfera-ponto (coletar diamante)

#endif // COLLISIONS_H