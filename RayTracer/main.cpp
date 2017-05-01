#include "ray_tracer.h"

#include <chrono>
#include <iostream>

using namespace std;

int main()
{	
	camera_t camera;
	camera.pos = { -0.5f, 2.5f, -4.0f };
	scene_set_camera(camera);

	light_t light;
	light.pos = { 5.0f, 5.0f, -10.0f };
	light.color = { 1.0f, 1.0f, 1.0f };
	scene_set_light(light);

	shared_ptr<image_t> marble_texture(load_png_from_file("marble.png"));
	shared_ptr<image_t> metal_texture(load_png_from_file("metal.png"));

	material_t glass_simple = { 0.1f, 1.0f, 1.0f, 50.0f, 0.4f, nullptr };
	material_t glass_metal = { 0.1f, 1.0f, 0.8f, 40.0f, 0.3f, metal_texture };
	material_t marble = { 0.1f, 0.75f, 0.5f, 50.0f, 0.45f, marble_texture };

	{
		auto sphere = make_unique<sphere_t>();
		sphere->material = glass_metal;
		sphere->position = { 0.75f, 0.1f, 1.0f };
		sphere->radius = 0.6f;
		sphere->color = { 0.0f, 0.5f, 1.0f };
		scene_add_object(move(sphere));
	}
	{
		auto sphere = make_unique<sphere_t>();
		sphere->material = glass_simple;
		sphere->position = { -0.75f, 0.1f, 2.25f };
		sphere->radius = 0.6f;
		sphere->color = { 0.5f, 0.223f, 0.5f };
		scene_add_object(move(sphere));
	}
	{
		auto sphere = make_unique<sphere_t>();
		sphere->material = glass_metal;
		sphere->position = { -2.15f, 0.1f, 1.5f };
		sphere->radius = 0.6f;
		sphere->color = { 1.0f, 0.572f, 0.184f };		
		scene_add_object(move(sphere));
	}
	{
		auto sphere = make_unique<sphere_t>();
		sphere->material = glass_metal;
		sphere->position = { -2.75f, -0.2f, 0.5f };
		sphere->radius = 0.4f;
		sphere->color = { 0.6f, 0.772f, 0.284f };
		scene_add_object(move(sphere));
	}
	{
		auto sphere = make_unique<sphere_t>();
		sphere->material = glass_simple;
		sphere->position = { 2.15f, -0.1f, 0.5f };
		sphere->radius = 0.4f;
		sphere->color = { 0.9f, 0.272f, 0.184f };
		scene_add_object(move(sphere));
	}
	{
		auto plane = make_unique<plane_t>();
		plane->material = marble;
		plane->position = { 0.0f, -0.5f, 2.0f };
		plane->bounds = { 4.0f, 2.0f };
		plane->normal = { 0.0f, 1.0f, 0.0f };
		plane->angle = 0.0f * DEG_TO_RAD;
		plane->color = { 0.8f, 0.8f, 0.8f };
		scene_add_object(move(plane));
	}
	{
		auto plane = make_unique<plane_t>();
		plane->material = marble;
		plane->position = { 0.0f, 1.99f, 3.99f };
		plane->bounds = { 4.0f, 2.5f };
		plane->normal = { 0.0f, 0.0f, -1.0f };
		plane->angle = 0.0f * DEG_TO_RAD;
		plane->color = { 0.8f, 0.8f, 0.8f };
		scene_add_object(move(plane));
	}
	{
		auto plane = make_unique<plane_t>();
		plane->material = marble;
		plane->position = { -3.99f, 1.99f, 2.0f };
		plane->bounds = { 2.0f, 2.5f };
		plane->normal = { 1.0f, 0.0f, 0.0f };
		plane->angle = 0.0f * DEG_TO_RAD;
		plane->color = { 0.8f, 0.8f, 0.8f };
		scene_add_object(move(plane));
	}

	image_t output = { SCREEN_WIDTH, SCREEN_HEIGHT, make_unique<pixel_t[]>(SCREEN_WIDTH * SCREEN_HEIGHT) };
	auto begin = chrono::high_resolution_clock::now();
	scene_render(&output);
	auto end = chrono::high_resolution_clock::now();
	cout << chrono::duration_cast<chrono::milliseconds>(end - begin).count() << " ms\n";

	save_png_to_file(output, "scene.png");

	return 0;
}
