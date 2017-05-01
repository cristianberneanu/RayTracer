#include "common.h"
#include "image.h"

#include <png.h>
#include <memory>

using namespace std;

static const uint32_t COLOR_DEPTH = 8;
static const uint32_t PIXEL_SIZE = 3;

bool save_png_to_file(const image_t& image, const char *path)
{
	png_structp png_ptr = nullptr;
	png_infop info_ptr = nullptr;	
	bool success = false;
	FILE* fp = nullptr;
	png_byte** row_pointers = nullptr;

	fp = fopen(path, "wb");
	if (!fp)
	{
		printf("Failed to open file for writing %s\n", path);
		goto cleanup;
	}

	png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if (!png_ptr)
	{
		printf("png_create_write_struct failed\n");
		goto cleanup;
	}

	info_ptr = png_create_info_struct(png_ptr);
	if (info_ptr == nullptr)
	{
		printf("png_create_info_struct failed\n");
		goto cleanup;
	}

	// set up error handling
	if (setjmp(png_jmpbuf(png_ptr)))
	{
		printf("Failed to write file %s\n", path);
		goto cleanup;
	}

	// set image attributes
	png_set_IHDR(png_ptr, info_ptr, image.width, image.height, COLOR_DEPTH,
		PNG_COLOR_TYPE_RGB, PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
	
	// allocate and fill temp image data
	row_pointers = new png_byte*[image.height];
	for (uint32_t y = 0; y < image.height; y++) 
	{

		png_byte* row = new png_byte[image.width * PIXEL_SIZE];
		row_pointers[y] = row;
		for (uint32_t x = 0; x < image.width; x++) 
		{
			pixel_t pixel = image.get(x, y);
			*row++ = pixel.r;
			*row++ = pixel.g;
			*row++ = pixel.b;
		}
	}

	// write image data to file
	png_init_io(png_ptr, fp);
	png_set_rows(png_ptr, info_ptr, row_pointers);
	png_write_png(png_ptr, info_ptr, PNG_TRANSFORM_IDENTITY, nullptr);

	success = true;

cleanup:
	for (uint32_t y = 0; y < image.height; y++)
		delete[] row_pointers[y];
	delete[] row_pointers;
	png_destroy_write_struct(&png_ptr, &info_ptr);
	if (fp) fclose(fp);
	return success;
}

image_t* load_png_from_file(const char* path)
{	
	FILE* fp = nullptr;
	image_t* image = nullptr;
	png_structp png_ptr = nullptr;
	png_infop info_ptr = nullptr;
	png_byte** row_pointers = nullptr;
	
	fp = fopen(path, "rb");
	if (fp == nullptr)
	{
		printf("Could not open file %s\n", path);
		goto cleanup;
	}

	png_byte header[8];
	fread(header, 1, sizeof(header), fp);
	if (png_sig_cmp(header, 0, sizeof(header)))
	{
		printf("Invalid PNG file %s\n", path);
		goto cleanup;
	}

	png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if (!png_ptr)
	{
		printf("png_create_read_struct failed\n");
		goto cleanup;
	}

	info_ptr = png_create_info_struct(png_ptr);
	if (!info_ptr)
	{
		printf("png_create_info_struct failed\n");
		goto cleanup;
	}

	// set up error handling
	if (setjmp(png_jmpbuf(png_ptr)))
	{
		printf("Failed to parse file %s\n", path);
		goto cleanup;
	}

	png_init_io(png_ptr, fp);
	png_set_sig_bytes(png_ptr, 8);

	png_read_info(png_ptr, info_ptr);

	uint32_t width = png_get_image_width(png_ptr, info_ptr);
	uint32_t height = png_get_image_height(png_ptr, info_ptr);
	png_byte color_type = png_get_color_type(png_ptr, info_ptr);
	png_byte bit_depth = png_get_bit_depth(png_ptr, info_ptr);

	if (bit_depth != COLOR_DEPTH || color_type != PNG_COLOR_TYPE_RGB)
	{
		printf("Image type not supported for %s (only 24-bit images supported)\n", path);
		goto cleanup;
	}

	png_read_update_info(png_ptr, info_ptr);

	// allocate temp image data
	row_pointers = new png_byte*[height];
	for (uint32_t i = 0; i < height; i++)
		row_pointers[i] = new png_byte[png_get_rowbytes(png_ptr, info_ptr)];

	png_read_image(png_ptr, row_pointers);

	image = new image_t{ width, height, make_unique<pixel_t[]>(width * height) };

	for (uint32_t i = 0; i < height; i++)
	{
		for (uint32_t j = 0; j < width; j++)
		{
			auto bytes = &row_pointers[i][j * PIXEL_SIZE];
			pixel_t pixel = { bytes[0], bytes[1], bytes[2] };
			image->put(j, i, pixel);
		}
	}

cleanup:
	for (uint32_t i = 0; i < height; i++)
		delete[] row_pointers[i];
	delete[] row_pointers;
	png_destroy_read_struct(&png_ptr, &info_ptr, nullptr);
	if (fp) fclose(fp);
	return image;
}