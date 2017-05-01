#pragma once

#include "color.h"

#include <memory>
#include <assert.h>

struct image_t
{
	uint32_t width, height;
	std::unique_ptr<pixel_t[]> data;

	uint32_t wrap(float value, uint32_t bound) const
	{
		int32_t coord = (int32_t)(value * bound) % bound;
		if (coord < 0) coord += (int32_t)bound;
		return (uint32_t)coord;
	}

	pixel_t get(float x, float y) const
	{
		uint32_t ux = wrap(x, width);
		uint32_t uy = wrap(y, height);
		return get(ux, uy);
	}

	pixel_t get(uint32_t x, uint32_t y) const
	{
		assert(x < width && y < height);
		return data[x + y * width];
	}

	void put(uint32_t x, uint32_t y, pixel_t pixel)
	{
		assert(x < width && y < height);
		data[x + y * width] = pixel;
	}
};

bool save_png_to_file(const image_t& output, const char *path);
image_t* load_png_from_file(const char* path);
