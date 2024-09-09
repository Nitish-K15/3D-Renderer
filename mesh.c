#include <stdio.h>
#include "string.h"
#include "mesh.h"



mesh_t mesh = {
	.vertices = NULL,
	.faces = NULL,
	.rotation = {0,0,0},
	.scale = {1,1,1},
	.translation = {0,0,0}
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
	{.a = 1, .b = 2, .c = 3,.color = 0xFFFF0000 },
	{.a = 1, .b = 3, .c = 4 ,.color = 0xFFFF0000},
	// right
{.a = 4, .b = 3, .c = 5, .color = 0xFF00FF00 },
{.a = 4, .b = 5, .c = 6, .color = 0xFF00FF00 },
// back
{.a = 6, .b = 5, .c = 7, .color = 0xFF0000FF },
{.a = 6, .b = 7, .c = 8, .color = 0xFF0000FF },
// left
{.a = 8, .b = 7, .c = 2 ,.color = 0xFFFFFF00},
{.a = 8, .b = 2, .c = 1 ,.color = 0xFFFFFF00},
// top
{.a = 2, .b = 7, .c = 5,.color = 0xFFFF00FF },
{.a = 2, .b = 5, .c = 3 ,.color = 0xFFFF00FF},
// bottom
{.a = 6, .b = 8, .c = 1,.color = 0xFF00FFFF },
{.a = 6, .b = 1, .c = 4 ,.color = 0xFF00FFFF}
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
