#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>

static inline uint8_t to_byte(float v) { return (uint8_t)(v * 255.0f); }
static inline float from_byte(uint8_t v) { return v * (1.0f / 255.0f); }

static inline float clamp(float v, float low, float up) { return (float)fmax(fmin(v, up), low); }

static const float DEG_TO_RAD = 0.017453292519943295769236907684886f;
static const float RAD_TO_DEG = 57.295779513082320876798154814105f;
static const float PI = 3.14159265358979323846f;