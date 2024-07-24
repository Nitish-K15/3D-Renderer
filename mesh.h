#ifndef MESH_H
#define MESH_H

#include "vector.h"
#include "triangle.h"
#include "array.h"

#define N_CUBE_VERTICES 8

extern vec3_t cube_vertices[N_CUBE_VERTICES];

#define N_CUBE_FACES (6*2)
extern face_t cube_faces[N_CUBE_FACES];

void load_cube_mesh_data(void);

typedef struct {
	vec3_t* vertices; //dynamic array for vertices
	face_t* faces; //dynamic array for faces
	vec3_t rotation; //rotation with x,y,z values
}mesh_t;

extern mesh_t mesh;

#endif // !MESH_H
