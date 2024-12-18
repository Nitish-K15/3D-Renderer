#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <SDL.h>
#include "display.h"
#include "vector.h"
#include "triangle.h"
#include "mesh.h"
#include "matrix.h"
#include "light.h"
#include "array.h"

//vec3_t cube_points[N_POINTS];

//vec2_t projected_points[N_POINTS];

triangle_t* triangles_to_render = NULL;


bool is_running = false;
int previous_frame_time = 0;

vec3_t camera_position = { 0,0,0 };

mat4_t proj_matrix;


void setup(void)
{
	render_method = RENDER_WIRE;
	cull_method = CULL_BACKFACE;

	//Allocate the required memory in bytes to hold the color buffer
	color_buffer = (uint32_t*)malloc(sizeof(uint32_t) * window_width * window_height);

	//Creating an SDL texture that is used to display the color buffer
	color_buffer_texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, window_width, window_height);

	//Initialize projection matrix
	float fov = M_PI / 3.0; //same as 180/3 or 60deg
	float aspect = (float)window_height / (float)window_width;
	float znear = 0.1;
	float zfar = 100.0;
	proj_matrix = mat4t_make_perspective(fov, aspect, znear, zfar);

	load_cube_mesh_data();
	//load_obj_file_data("D:/VS/3DRenderer/assets/f22.obj");
}

void process_input(void)
{
	SDL_Event event;
	SDL_PollEvent(&event);

	switch (event.type)
	{
	case SDL_QUIT:
		is_running = false;
		break;
	case SDL_KEYDOWN:
		if (event.key.keysym.sym == SDLK_ESCAPE)
			is_running = false;
		if (event.key.keysym.sym == SDLK_1)
			render_method = RENDER_WIRE_VERTEX;
		if (event.key.keysym.sym == SDLK_2)
			render_method = RENDER_WIRE;
		if (event.key.keysym.sym == SDLK_3)
			render_method = RENDER_FILL_TRIANGLE;
		if (event.key.keysym.sym == SDLK_4)
			render_method = RENDER_FILL_TRIANGLE_WIRE;
		if (event.key.keysym.sym == SDLK_c)
			cull_method = CULL_BACKFACE;
		if (event.key.keysym.sym == SDLK_d)
			cull_method = CULL_NONE;
		break;
	}
}



void update(void)
{
	int time_to_wait = FRAME_TARGET_TIME - (SDL_GetTicks() - previous_frame_time);

	if (time_to_wait > 0 && time_to_wait < FRAME_TARGET_TIME)
	{
		SDL_Delay(time_to_wait);
	}

	//while (!SDL_TICKS_PASSED(SDL_GetTicks(), previous_frame_time + FRAME_TARGET_TIME));
	previous_frame_time = SDL_GetTicks();

	//Initialize array of triangles to render
	triangles_to_render = NULL;

	// Change the mesh scale, rotation, and translation values per animation frame
	  // mesh.rotation.x += 0.01;
	mesh.rotation.y += 0.01;
	// mesh.rotation.z += 0.01;
	// mesh.scale.x += 0.002;
	// mesh.scale.y += 0.001;
	// mesh.translation.x += 0.01;
	mesh.translation.z = 5.0;

	mat4_t scale_matrix = mat4_make_scale(mesh.scale.x, mesh.scale.y, mesh.scale.z);
	mat4_t translation_matrix = mat4_make_translation(mesh.translation.x, mesh.translation.y, mesh.translation.z);
	mat4_t rotation_matrix_x = mat4_make_rotation_x(mesh.rotation.x);
	mat4_t rotation_matrix_y = mat4_make_rotation_y(mesh.rotation.y);
	mat4_t rotation_matrix_z = mat4_make_rotation_z(mesh.rotation.z);

	//Loop all triangle faces of our mesh
	int num_faces = array_length(mesh.faces);
	for (int i = 0; i < num_faces; i++)
	{
		face_t mesh_face = mesh.faces[i];


		vec3_t face_vertices[3];

		face_vertices[0] = mesh.vertices[mesh_face.a - 1];
		face_vertices[1] = mesh.vertices[mesh_face.b - 1];
		face_vertices[2] = mesh.vertices[mesh_face.c - 1];

		vec4_t transformed_vertices[3];

		//Loop all three vertices of this current face and apply transformations
		for (int j = 0; j < 3; j++)
		{
			vec4_t transformed_vertex = vec4_from_vec3(face_vertices[j]);

			mat4_t world_matrix = mat4_identity();
			world_matrix = mat4_mul_mat4(scale_matrix, world_matrix);
			world_matrix = mat4_mul_mat4(rotation_matrix_z, world_matrix);
			world_matrix = mat4_mul_mat4(rotation_matrix_y, world_matrix);
			world_matrix = mat4_mul_mat4(rotation_matrix_x, world_matrix);
			world_matrix = mat4_mul_mat4(translation_matrix, world_matrix);

			transformed_vertex = mat4_mul_vec4(world_matrix, transformed_vertex);

			//Translate the vertex away from camera in Z
			//transformed_vertex.z += 5;

			transformed_vertices[j] = transformed_vertex;
		}

		//Calculate average depth of each face
		float avg_depth = (transformed_vertices[0].z + transformed_vertices[1].z + transformed_vertices[2].z) / 3.0;

		//Check backface culling
		vec3_t vector_a = vec3_from_vec4(transformed_vertices[0]);
		vec3_t vector_b = vec3_from_vec4(transformed_vertices[1]);
		vec3_t vector_c = vec3_from_vec4(transformed_vertices[2]);

		//Get vector b-a and c-a
		vec3_t vector_ab = vec3_sub(vector_b, vector_a);
		vec3_t vector_ac = vec3_sub(vector_c, vector_a);

		//Compute face normal
		vec3_t normal = vec3_cross(vector_ab, vector_ac);
		//Normalize the normal vector
		vec3_normalize(&normal);

		vec3_t camera_ray = vec3_sub(camera_position, vector_a); //from point to camera
		if (cull_method == CULL_BACKFACE)
		{

			//Bypass the triangles that are looking away from the camera
			if (vec3_dot(normal, camera_ray) < 0)
			{
				continue;
			}
		}


		vec4_t projected_points[3];

		//Project the vertices
		for (int j = 0; j < 3; j++)
		{
			//Project the current vertex
			projected_points[j] = mat4_mul_vec4_projection(proj_matrix, transformed_vertices[j]);


			//Scale into view
			projected_points[j].x *= window_width / 2.0;
			projected_points[j].y *= window_height / 2.0;

			//Scale and translate projected point to the middle of the screen
			projected_points[j].x += window_width / 2.0;
			projected_points[j].y += window_height / 2.0;

		}

		//Calculate shade intensity of light
		float light_intensity = -vec3_dot(normal, light.direction);

		//Calculate triangle color
		uint32_t triangle_color = light_apply_intensity(mesh_face.color, light_intensity);

		triangle_t projected_triangle = {
			.points = {
				{projected_points[0].x,projected_points[0].y},
				{ projected_points[1].x,projected_points[1].y },
				{projected_points[2].x,projected_points[2].y},
				},
			.color = triangle_color,
			.avg_depth = avg_depth
		};


		//Save the projected triangle in the array of triangles to render
		//triangles_to_render[i] = projected_triangle;
		array_push(triangles_to_render, projected_triangle);
	}

	int num_of_triangles = array_length(triangles_to_render);
	for (int i = 0; i < num_of_triangles; i++)
		for (int j = i; j < num_of_triangles; j++)
		{
			if (triangles_to_render[i].avg_depth < triangles_to_render[j].avg_depth)
			{
				triangle_t temp = triangles_to_render[i];
				triangles_to_render[i] = triangles_to_render[j];
				triangles_to_render[j] = temp;
			}
		}


	/*for (int i = 0; i < N_POINTS; i++)
	{
		vec3_t point = cube_points[i];

		vec3_t transformed_point = vec3_rotate_x(point, cube_rotation.x);
		transformed_point = vec3_rotate_y(transformed_point, cube_rotation.y);
		transformed_point = vec3_rotate_z(transformed_point, cube_rotation.z);

		//move point away from camera
		transformed_point.z -= camera_position.z;

		//Project the current point
		vec2_t projected_point = project(transformed_point);

		//Save the projected 2d vector in array of projected points
		projected_points[i] = projected_point;
	}*/
}


void render(void)
{
	//SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
	//SDL_RenderClear(renderer);

	draw_grid();

	int num_triangles = array_length(triangles_to_render);

	for (int i = 0; i < num_triangles; i++)
	{
		triangle_t triangle = triangles_to_render[i];
		/*	draw_rect(triangle.points[0].x, triangle.points[0].y, 3, 3, 0xFFFFFF00);
			draw_rect(triangle.points[1].x, triangle.points[1].y, 3, 3, 0xFFFFFF00);
			draw_rect(triangle.points[2].x, triangle.points[2].y, 3, 3, 0xFFFFFF00);*/
		if (render_method == RENDER_FILL_TRIANGLE || render_method == RENDER_FILL_TRIANGLE_WIRE)
			draw_filled_triangle(triangle.points[0].x, triangle.points[0].y, triangle.points[1].x, triangle.points[1].y, triangle.points[2].x, triangle.points[2].y, triangle.color);

		if (render_method == RENDER_WIRE || render_method == RENDER_WIRE_VERTEX || render_method == RENDER_FILL_TRIANGLE_WIRE)
			draw_triangle(triangle.points[0].x, triangle.points[0].y, triangle.points[1].x, triangle.points[1].y, triangle.points[2].x, triangle.points[2].y, 0xFFFFFFFF);

		if (render_method == RENDER_WIRE_VERTEX)
		{
			draw_rect(triangle.points[0].x, triangle.points[0].y, 3, 3, 0xFFFFFF00);
			draw_rect(triangle.points[1].x, triangle.points[1].y, 3, 3, 0xFFFFFF00);
			draw_rect(triangle.points[2].x, triangle.points[2].y, 3, 3, 0xFFFFFF00);
		}
	}

	//draw_pixel(10, 10, 0xFFFFFF00);
	//draw_rect(50, 50, 40, 100, 0xFFFF0000);
	/*for (int i = 0; i < N_POINTS; i++)
	{
		vec2_t projected_point = projected_points[i];
		draw_rect(projected_point.x + window_width / 2, projected_point.y + window_height / 2, 4, 4, 0xFFFFFF00);
	}*/


	//Clear the array of triangles to render every frame loop
	array_free(triangles_to_render);


	render_color_buffer();
	clear_color_buffer(0xFF000000);

	SDL_RenderPresent(renderer);
}

void free_resources(void)
{
	free(color_buffer);
	array_free(mesh.faces);
	array_free(mesh.vertices);

}


int main(int argc, char* args[])
{
	is_running = InitializeWindow();

	setup();

	while (is_running)
	{
		process_input();
		update();
		render();
	}

	free_resources();
	destroy_window();

	return 0;
}