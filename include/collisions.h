// collisions.h
#ifndef COLLISIONS_H
#define COLLISIONS_H

// Definição das dimensões do mapa
#define MAP_WIDTH 10
#define MAP_HEIGHT 10

// Declaração da matriz do mapa (extern avisa que a variável existe em outro arquivo)
extern int maze_map[MAP_HEIGHT][MAP_WIDTH];

// Declaração da função de colisão
bool CheckCollision(float x, float z);

#endif // COLLISIONS_H