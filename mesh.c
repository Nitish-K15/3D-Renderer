#include <stdio.h>
#include "string.h"
#include "mesh.h"



mesh_t mesh = {
	.vertices = NULL,
	.faces = NULL,
	.rotation = {0,0,0}
};

vec3_t cube_vertices[N_CUBE_VERTICES] = {
	{ -1, -1, -1 },
	{ -1,1,-1 },
	{ 1,1,-1 },
	{ 1,-1,-1 },
	{ 1,1,1 },
	{ 1,-1,1 },
	{ -1,1,1 },
	{ -1,-1,1 }
};

face_t cube_faces[N_CUBE_FACES] = {
	// front
	{.a = 1, .b = 2, .c = 3 },
	{.a = 1, .b = 3, .c = 4 },
	// right
{.a = 4, .b = 3, .c = 5 },
{.a = 4, .b = 5, .c = 6 },
// back
{.a = 6, .b = 5, .c = 7 },
{.a = 6, .b = 7, .c = 8 },
// left
{.a = 8, .b = 7, .c = 2 },
{.a = 8, .b = 2, .c = 1 },
// top
{.a = 2, .b = 7, .c = 5 },
{.a = 2, .b = 5, .c = 3 },
// bottom
{.a = 6, .b = 8, .c = 1 },
{.a = 6, .b = 1, .c = 4 }
};

void load_cube_mesh_data(void)
{
	for (int i = 0; i < N_CUBE_VERTICES; i++)
	{
		vec3_t cube_vertex = cube_vertices[i];
		array_push(mesh.vertices, cube_vertex);
	}
	for (int i = 0; i < N_CUBE_FACES; i++)
	{
		face_t cube_face = cube_faces[i];
		array_push(mesh.faces, cube_face);
	}
}

void load_obj_file_data(char* filepath)
{
	FILE* file = fopen(filepath, "r");
	if (!file) {
		perror("Failed to open file");
		return;
	}

	char line[512];

	while (fgets(line, sizeof(line), file)) {
		// Vertex information
		if (strncmp(line, "v ", 2) == 0) {
			vec3_t vertex;
			if (sscanf(line, "v %f %f %f", &vertex.x, &vertex.y, &vertex.z) == 3) {
				array_push(mesh.vertices, vertex);
			}
			else {
				fprintf(stderr, "Failed to parse vertex line: %s", line);
			}
		}

		// Face information
		if (strncmp(line, "f ", 2) == 0) {
			int vertexIndex[3], textureIndex[3], normalIndex[3];
			if (sscanf(line, "f %d/%d/%d %d/%d/%d %d/%d/%d",
				&vertexIndex[0], &textureIndex[0], &normalIndex[0],
				&vertexIndex[1], &textureIndex[1], &normalIndex[1],
				&vertexIndex[2], &textureIndex[2], &normalIndex[2]) == 9) {
				face_t face = { .a = vertexIndex[0], .b = vertexIndex[1], .c = vertexIndex[2] };
				array_push(mesh.faces, face);
			}
			else {
				fprintf(stderr, "Failed to parse face line: %s", line);
			}
		}
	}

	fclose(file);
}
