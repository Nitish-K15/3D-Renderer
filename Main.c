#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <SDL.h>
#include "display.h"
#include "vector.h"
#include "triangle.h"
#include "texture.h"
#include "mesh.h"
#include "matrix.h"
#include "light.h"
#include "array.h"
#include "camera.h"
#include "clipping.h"
//vec3_t cube_points[N_POINTS];

//vec2_t projected_points[N_POINTS];
#define MAX_TRIANGLES_PER_MESH 10000
triangle_t triangles_to_render[MAX_TRIANGLES_PER_MESH];

int num_triangles_to_render = 0;


bool is_running = false;
int previous_frame_time = 0;
float delta_time = 0;


mat4_t proj_matrix;
mat4_t view_matrix;
mat4_t world_matrix;


void setup(void)
{
	render_method = RENDER_WIRE;
	cull_method = CULL_BACKFACE;

	//Allocate the required memory in bytes to hold the color buffer
	color_buffer = (uint32_t*)malloc(sizeof(uint32_t) * window_width * window_height);

	z_buffer = (float*)malloc(sizeof(float) * window_width * window_height);

	//Creating an SDL texture that is used to display the color buffer
	color_buffer_texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STREAMING, window_width, window_height);

	//Initialize projection matrix
	float aspect_y = (float)window_height / (float)window_width;
	float aspect_x = (float)window_width / (float)window_height;
	float fov_y = 3.141592 / 3.0; // the same as 180/3, or 60deg
	float fov_x = atan(tan(fov_y / 2) * aspect_x) * 2;
	float z_near = 1.0;
	float z_far = 20.0;
	proj_matrix = mat4t_make_perspective(fov_y, aspect_y, z_near, z_far);

	init_frustum_planes(fov_x, fov_y, z_near, z_far);

	//Manually load the hardcodded texture data from static array
	load_png_texture_data("./assets/f22.png");

	//load_cube_mesh_data();
	load_obj_file_data("D:/VS/3DRenderer/assets/f22.obj");
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
		if (event.key.keysym.sym == SDLK_5)
			render_method = RENDER_TEXTURED;
		if (event.key.keysym.sym == SDLK_6)
			render_method = RENDERED_TEXTURED_WIRE;
		if (event.key.keysym.sym == SDLK_c)
			cull_method = CULL_BACKFACE;
		if (event.key.keysym.sym == SDLK_x)
			cull_method = CULL_NONE;
		if (event.key.keysym.sym == SDLK_UP)
			camera.position.y += 3.0 * delta_time;
		if (event.key.keysym.sym == SDLK_DOWN)
			camera.position.y -= 3.0 * delta_time;
		if (event.key.keysym.sym == SDLK_a)
			camera.yaw -= 1.0 * delta_time;
		if (event.key.keysym.sym == SDLK_d)
			camera.yaw += 1.0 * delta_time;
		if (event.key.keysym.sym == SDLK_w) {
			camera.forward_velocity = vec3_mul(camera.direction, 5.0 * delta_time);
			camera.position = vec3_add(camera.position, camera.forward_velocity);
		}
		if (event.key.keysym.sym == SDLK_s) {
			camera.forward_velocity = vec3_mul(camera.direction, 5.0 * delta_time);
			camera.position = vec3_sub(camera.position, camera.forward_velocity);
		}
	}
}



void update(void)
{
	int time_to_wait = FRAME_TARGET_TIME - (SDL_GetTicks() - previous_frame_time);

	if (time_to_wait > 0 && time_to_wait < FRAME_TARGET_TIME)
	{
		SDL_Delay(time_to_wait);
	}

	//Get a delta time factor converted to seconds to be used to update gameobjects
	delta_time = (SDL_GetTicks() - previous_frame_time) / 1000.0;

	//while (!SDL_TICKS_PASSED(SDL_GetTicks(), previous_frame_time + FRAME_TARGET_TIME));
	previous_frame_time = SDL_GetTicks();

	//Initialize array of triangles to render
	num_triangles_to_render = 0;

	// Change the mesh scale, rotation, and translation values per animation frame
	//mesh.rotation.x += 0.01 * delta_time;
	//mesh.rotation.y += 0.01;
	// mesh.rotation.z += 0.01;
	// mesh.scale.x += 0.002;
	// mesh.scale.y += 0.001;
	// mesh.translation.x += 0.01;
	mesh.translation.z = 5.0;

	//Create the view marix looking at a hardcoded target point
	vec3_t up_direction = { 0,1,0 };
	vec3_t target = { 0,0,1 };
	mat4_t camera_yaw_rotation = mat4_make_rotation_y(camera.yaw);
	camera.direction = vec3_from_vec4(mat4_mul_vec4(camera_yaw_rotation, vec4_from_vec3(target)));
	target = vec3_add(camera.position, camera.direction);
	view_matrix = mat4_look_at(camera.position, target, up_direction);

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

			world_matrix = mat4_identity();
			world_matrix = mat4_mul_mat4(scale_matrix, world_matrix);
			world_matrix = mat4_mul_mat4(rotation_matrix_z, world_matrix);
			world_matrix = mat4_mul_mat4(rotation_matrix_y, world_matrix);
			world_matrix = mat4_mul_mat4(rotation_matrix_x, world_matrix);
			world_matrix = mat4_mul_mat4(translation_matrix, world_matrix);

			transformed_vertex = mat4_mul_vec4(world_matrix, transformed_vertex);

			//Multiply the view matrix by the current vector to transform scene to camera space
			transformed_vertex = mat4_mul_vec4(view_matrix, transformed_vertex);

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


		vec3_t origin = { 0,0,0 };
		vec3_t camera_ray = vec3_sub(origin, vector_a); //from point to camera
		if (cull_method == CULL_BACKFACE)
		{

			//Bypass the triangles that are looking away from the camera
			if (vec3_dot(normal, camera_ray) < 0)
			{
				continue;
			}
		}


		// Create a polygon from the original transformed triangle to be clipped
		polygon_t polygon = polygon_from_triangle(
			vec3_from_vec4(transformed_vertices[0]),
			vec3_from_vec4(transformed_vertices[1]),
			vec3_from_vec4(transformed_vertices[2]),
			mesh_face.a_uv,
			mesh_face.b_uv,
			mesh_face.c_uv
		);

		// Clip the polygon and returns a new polygon with potential new vertices
		clip_polygon(&polygon);

		// Break the clipped polygon apart back into individual triangles
		triangle_t triangles_after_clipping[MAX_NUM_POLY_TRIANGLES];
		int num_triangles_after_clipping = 0;

		triangles_from_polygon(&polygon, triangles_after_clipping, &num_triangles_after_clipping);

		// Loops all the assembled triangles after clipping
		for (int t = 0; t < num_triangles_after_clipping; t++) {
			triangle_t triangle_after_clipping = triangles_after_clipping[t];

			vec4_t projected_points[3];

			// Loop all three vertices to perform projection and conversion to screen space
			for (int j = 0; j < 3; j++) {
				// Project the current vertex using a perspective projection matrix
				projected_points[j] = mat4_mul_vec4(proj_matrix, triangle_after_clipping.points[j]);

				// Perform perspective divide
				if (projected_points[j].w != 0) {
					projected_points[j].x /= projected_points[j].w;
					projected_points[j].y /= projected_points[j].w;
					projected_points[j].z /= projected_points[j].w;
				}

				// Flip vertically since the y values of the 3D mesh grow bottom->up and in screen space y values grow top->down
				projected_points[j].y *= -1;

				// Scale into the view
				projected_points[j].x *= (window_width / 2.0);
				projected_points[j].y *= (window_height / 2.0);

				// Translate the projected points to the middle of the screen
				projected_points[j].x += (window_width / 2.0);
				projected_points[j].y += (window_height / 2.0);
			}

			// Calculate the shade intensity based on how aliged is the normal with the flipped light direction ray
			float light_intensity_factor = -vec3_dot(normal, light.direction);

			// Calculate the triangle color based on the light angle
			uint32_t triangle_color = light_apply_intensity(mesh_face.color, light_intensity_factor);

			// Create the final projected triangle that will be rendered in screen space
			triangle_t triangle_to_render = {
				.points = {
					{ projected_points[0].x, projected_points[0].y, projected_points[0].z, projected_points[0].w },
					{ projected_points[1].x, projected_points[1].y, projected_points[1].z, projected_points[1].w },
					{ projected_points[2].x, projected_points[2].y, projected_points[2].z, projected_points[2].w }
				},
				.texcoords = {
					{ triangle_after_clipping.texcoords[0].u, triangle_after_clipping.texcoords[0].v },
					{ triangle_after_clipping.texcoords[1].u, triangle_after_clipping.texcoords[1].v },
					{ triangle_after_clipping.texcoords[2].u, triangle_after_clipping.texcoords[2].v }
				},
				.color = triangle_color
			};


			//Save the projected triangle in the array of triangles to render
			//triangles_to_render[i] = projected_triangle;
			if (num_triangles_to_render < MAX_TRIANGLES_PER_MESH)
			{
				triangles_to_render[num_triangles_to_render++] = triangle_to_render;

			}
		}


	}

	//int num_of_triangles = array_length(triangles_to_render);



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

	//int num_triangles = array_length(triangles_to_render);

	for (int i = 0; i < num_triangles_to_render; i++)
	{
		triangle_t triangle = triangles_to_render[i];
		/*	draw_rect(triangle.points[0].x, triangle.points[0].y, 3, 3, 0xFFFFFF00);
			draw_rect(triangle.points[1].x, triangle.points[1].y, 3, 3, 0xFFFFFF00);
			draw_rect(triangle.points[2].x, triangle.points[2].y, 3, 3, 0xFFFFFF00);*/
		if (render_method == RENDER_FILL_TRIANGLE || render_method == RENDER_FILL_TRIANGLE_WIRE)
			draw_filled_triangle(triangle.points[0].x, triangle.points[0].y, triangle.points[0].z, triangle.points[0].w, triangle.points[1].x, triangle.points[1].y, triangle.points[1].z, triangle.points[1].w, triangle.points[2].x, triangle.points[2].y, triangle.points[2].z, triangle.points[2].w, triangle.color);

		if (render_method == RENDERED_TEXTURED_WIRE || render_method == RENDER_TEXTURED)
			draw_textured_triangle(
				triangle.points[0].x, triangle.points[0].y, triangle.points[0].z, triangle.points[0].w, triangle.texcoords[0].u, triangle.texcoords[0].v, // vertex A
				triangle.points[1].x, triangle.points[1].y, triangle.points[1].z, triangle.points[1].w, triangle.texcoords[1].u, triangle.texcoords[1].v, // vertex B
				triangle.points[2].x, triangle.points[2].y, triangle.points[2].z, triangle.points[2].w, triangle.texcoords[2].u, triangle.texcoords[2].v, // vertex C
				mesh_texture
			);

		if (render_method == RENDER_WIRE || render_method == RENDER_WIRE_VERTEX || render_method == RENDER_FILL_TRIANGLE_WIRE || render_method == RENDERED_TEXTURED_WIRE)
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
	//array_free(triangles_to_render);


	render_color_buffer();
	clear_color_buffer(0xFF000000);
	clear_z_buffer();

	SDL_RenderPresent(renderer);
}

void free_resources(void)
{
	free(color_buffer);
	free(z_buffer);
	upng_free(png_texture);
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