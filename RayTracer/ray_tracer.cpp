#include "ray_tracer.h"

#include <chrono>
#include <vector>

using namespace std;

static const vec3_t x_axis = { 1.0f, 0.0f, 0.0f }, y_axis = { 0.0f, 1.0f, 0.0f }, z_axis = { 0.0f, 0.0f, 1.0f };

struct
{
	light_t light;
	camera_t camera;
	vector<unique_ptr<object_t>> objects;
} g_scene;

void scene_set_light(const light_t& light) { g_scene.light = light; }
void scene_set_camera(const camera_t& camera) { g_scene.camera = camera; }

void scene_add_object(unique_ptr<object_t> object)
{
	object->init();
	g_scene.objects.push_back(move(object));
}

struct ray_hit_t
{
	const object_t* object;
	vec3_t point, normal;
	color_t color;
};

// ray.dir must be normalized
float sphere_t::intersect(const ray_t& ray) const
{
	// https://www.siggraph.org/education/materials/HyperGraph/raytrace/rtinter1.htm
	float a = (ray.direction * ray.direction).sum();
	vec3_t os = ray.origin - position;
	float b = 2.0f * (ray.direction * os).sum();
	float c = (os * os).sum() - radius * radius;
	float d = b * b - 4.0f * a * c;

	// if ray can not intersect then stop
	if (d < 0.0f) return INFINITY;

	// ray can intersect the sphere, solve the closer hitpoint (smaller root)
	d = sqrtf(d);
	float t = -0.5f * (b + d) / a;
	if (t <= 0.0f) return INFINITY; // intersection point behind origin
	return sqrtf(a) * t;
}

vec3_t sphere_t::get_normal(const vec3_t& point) const
{
	return (point - position).normalize();
}

vec2_t sphere_t::get_tex_coords(const vec3_t& point) const
{
	vec3_t rel_point = point - position;
	// convert relative point into spherical coordinates (-pi..pi and 0..pi) and then into texture coordinates
	float x = 0.5f * (1.0f + atan2f(rel_point.z, rel_point.x) / PI);
	float y = acosf(rel_point.y / radius) / PI;
	return { x, y };
}

void plane_t::init()
{
	// compute and cache plane tangent and cotangent
	quat_t rot(normal, { 0.0f, 0.0f, -1.0f });
	tg = x_axis * rot;
	ctg = y_axis * rot;
}

// ray.dir must be normalized
float plane_t::intersect(const ray_t& ray) const
{
	// first do standard ray - plane intersection
	float denom = (ray.direction * normal).sum();
	if (fabs(denom) < 0.000001f) return INFINITY;

	float distance = ((position - ray.origin) * normal).sum() / denom;
	if (distance < 0) return INFINITY;

	vec3_t point = ray.origin + ray.direction * distance; // world coordinates
	// convert point into plane coordinates
	point -= position;
	point = rotate(point, normal, angle);
		
	// check if point inside bounds by projecting onto tangent and cotangent
	if (fabs(dot(tg, point)) > bounds.x || fabs(dot(ctg, point)) > bounds.y)
		return INFINITY;

	return distance;
}

vec3_t plane_t::get_normal(const vec3_t&) const { return normal; }
	
vec2_t plane_t::get_tex_coords(const vec3_t& point) const
{
	// project hit point into plane coordinates
	vec3_t rel_point = rotate(point - position, normal, angle);
	// compute texture coordinates by projecting relative hit point on plane tangent and cotangent
	float x = 0.5f * dot(rel_point, tg) / bounds.x + 0.5f;
	float y = 0.5f * dot(rel_point, ctg) / bounds.y + 0.5f;
	return { x, y };
}

bool trace_ray(const ray_t& ray, ray_hit_t* hit)
{
	float distance = INFINITY;

	for (const auto& object : g_scene.objects)
	{
		float object_distance = object->intersect(ray);
		if (object_distance < distance)
		{
			distance = object_distance;
			hit->object = object.get();
		}
	}

	if (distance == INFINITY) return false; // no hits

	hit->point = ray.origin + ray.direction * distance;
	hit->normal = hit->object->get_normal(hit->point);
	
	bool in_shadow = false;
	vec3_t light_dir = (g_scene.light.pos - hit->point).normalize();
	ray_t light_ray = { hit->point + hit->normal * 0.001f, light_dir };
	for (const auto& object : g_scene.objects)
	{
		if (object.get() != hit->object)
		{
			if (object->intersect(light_ray) < INFINITY)
			{
				in_shadow = true;
				break;
			}
		}
	}

	color_t objectColor = hit->object->color;
	image_t* texture = hit->object->material.texture.get();
	if (texture != nullptr)
	{
		vec2_t tex_coords = hit->object->get_tex_coords(hit->point);
		float scale = hit->object->texture_scale;
		pixel_t pixel = texture->get(tex_coords.x * scale, tex_coords.y * scale);
		objectColor *= color_t::from_pixel(pixel);
	}

	if (in_shadow)
	{
		hit->color = { 0.0f, 0.0f, 0.0f };
	}
	else
	{
		// diffuse shading
		hit->color = hit->object->material.diffuse_c * fmax((hit->normal * light_dir).sum(), 0.0f) * objectColor;
		// specular shading
		vec3_t camera_dir = (g_scene.camera.pos - hit->point).normalize();
		hit->color += hit->object->material.specular_c *
			pow(fmax((hit->normal * (light_dir + camera_dir).normalize()).sum(), 0.0f), hit->object->material.specular_k);
	}

	// ambient shading
	hit->color += objectColor * hit->object->material.ambient;
	hit->color *= g_scene.light.color; // modulate final color by light color

	return true;
}

void scene_render(image_t* output)
{
	const float ASPECT_RATIO = float(SCREEN_WIDTH) / SCREEN_HEIGHT;
	struct { float x0, y0, x1, y1; } screen_coords =
	{ -1.0f, -1.0f / ASPECT_RATIO + 0.25f, 1.0f, 1.0f / ASPECT_RATIO + 0.25f };

	float x_step = (screen_coords.x1 - screen_coords.x0) / SCREEN_WIDTH;
	float y_step = (screen_coords.y1 - screen_coords.y0) / SCREEN_HEIGHT;

	#pragma omp parallel for
	for (int j = 0; j < SCREEN_HEIGHT; j++)
	{
		for (uint32_t i = 0; i < SCREEN_WIDTH; i++) 
		{
			// compute the world ray for the current pixel
			float x = screen_coords.x0 + i * x_step;
			float y = screen_coords.y0 + (SCREEN_HEIGHT - j - 1) * y_step;
			vec3_t pixel_dir = { x, y - 0.5f, 1.0f };
			pixel_dir.normalize();
			ray_t ray = { g_scene.camera.pos, pixel_dir };

			color_t color = { 0.0f, 0.0f, 0.0f }; // color accumulator for current pixel
			float reflection = 1.0f; // reflection scale for current pixel
			for (uint32_t depth = 0; depth < REFLECTIONS; depth++)
			{
				ray_hit_t hit;
				if (!trace_ray(ray, &hit)) break; // exit if no hit

				ray.origin = hit.point + hit.normal * 0.001f;
				ray.direction = (ray.direction - 2.0f * (ray.direction * hit.normal).sum() * hit.normal).normalize();
				color += hit.color * reflection;

				reflection *= hit.object->material.reflection;
				if (reflection < 0.05f) break; // exit if reflection is too faded
			}

			output->put(i, j, color.normalize().to_pixel());
		}
	}
}


