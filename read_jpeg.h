#ifndef READ_JPEG_H
#define READ_JPEG_H

#include <stdio.h>
#include <stdlib.h>

#include <jpeglib.h>

struct jpeg_img {
	int width;
	int height;
	int num_components;
	JSAMPLE *data;
};

/* reads a JPEG image from file system, then decode and return its data as a struct */
int read_jpeg(char* filename, struct jpeg_img *img);

#endif

