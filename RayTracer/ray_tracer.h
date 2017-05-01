#pragma once

#include "common.h"
#include "vec.h"
#include "color.h"
#include "quat.h"
#include "image.h"

#include <memory>
#include <limits>

struct material_t
{	
	float ambient, diffuse_c, specular_c, specular_k, reflection;
	std::shared_ptr<image_t> texture;
};

struct ray_t
{
	vec3_t origin, direction;
};

struct object_t
{
	vec3_t position;
	material_t material;
	color_t color;
	float texture_scale;

	object_t() : color{1.0f, 1.0f, 1.0f}, texture_scale(1.0f) {}

	virtual void init() {} // called once when object is added to scene
	virtual float intersect(const ray_t& ray) const = 0;
	virtual vec3_t get_normal(const vec3_t& point) const = 0;
	virtual vec2_t get_tex_coords(const vec3_t& point) const = 0;
	virtual ~object_t() {}
};

struct light_t
{
	vec3_t pos;
	color_t color;
};

struct camera_t
{
	vec3_t pos;
};

const uint32_t SCREEN_WIDTH = 1920, SCREEN_HEIGHT = 1080;
const uint32_t REFLECTIONS = 3;

struct sphere_t : public object_t
{
	float radius;

	float intersect(const ray_t& ray) const;
	vec3_t get_normal(const vec3_t& point) const;
	vec2_t get_tex_coords(const vec3_t& point) const;
};

struct plane_t : public object_t
{
	vec2_t bounds;
	float angle; // rotation angle around normal
	vec3_t normal;	
	vec3_t tg, ctg; // tangent and cotangent, automatically computed during init

	void init();
	float intersect(const ray_t& ray) const;
	vec3_t get_normal(const vec3_t& point) const;
	vec2_t get_tex_coords(const vec3_t& point) const;
};

void scene_set_light(const light_t& light);
void scene_set_camera(const camera_t& camera);

void scene_add_object(std::unique_ptr<object_t> object);

void scene_render(image_t* output);
