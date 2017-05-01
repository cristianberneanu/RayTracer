#pragma once

#include <stdint.h>

#include "common.h"

struct pixel_t
{
	uint8_t r, g, b;
};

struct color_t
{
	float r, g, b;

	pixel_t to_pixel() const
		{ return { to_byte(r), to_byte(g), to_byte(b) }; }
	static color_t from_pixel(pixel_t pixel) 
		{ return { from_byte(pixel.r), from_byte(pixel.g), from_byte(pixel.b) }; }

	color_t& normalize()
	{
		r = clamp(r, 0.0f, 1.0f);
		g = clamp(g, 0.0f, 1.0f);
		b = clamp(b, 0.0f, 1.0f);
		return *this;
	}

	color_t& operator+=(const color_t& rhs) { r += rhs.r; g += rhs.g; b += rhs.b; return *this; }
	color_t& operator-=(const color_t& rhs) { r -= rhs.r; g -= rhs.g; b -= rhs.b; return *this; }
	color_t& operator*=(const color_t& rhs) { r *= rhs.r; g *= rhs.g; b *= rhs.b; return *this; }
	color_t& operator+=(float rhs) { r += rhs; g += rhs; b += rhs; return *this; }
	color_t& operator-=(float rhs) { r -= rhs; g -= rhs; b -= rhs; return *this; }
	color_t& operator*=(float rhs) { r *= rhs; g *= rhs; b *= rhs; return *this; }
};

inline color_t operator+(color_t lhs, const color_t& rhs) { lhs += rhs; return lhs; }
inline color_t operator-(color_t lhs, const color_t& rhs) { lhs -= rhs; return lhs; }
inline color_t operator*(color_t lhs, const color_t& rhs) { lhs *= rhs; return lhs; }
inline color_t operator+(color_t lhs, float rhs) { lhs += rhs; return lhs; }
inline color_t operator-(color_t lhs, float rhs) { lhs -= rhs; return lhs; }
inline color_t operator*(color_t lhs, float rhs) { lhs *= rhs; return lhs; }
inline color_t operator+(float lhs, color_t rhs) { rhs += lhs; return rhs; }
inline color_t operator-(float lhs, color_t rhs) { rhs -= lhs; return rhs; }
inline color_t operator*(float lhs, color_t rhs) { rhs *= lhs; return rhs; }
