#ifndef NPOT_TEX_H
#define NPOT_TEX_H

#include "read_jpeg.h"
#include <GL/gl.h>

/* Non-Power-Of-Two texture */
struct npot_tex {
	unsigned int real_width;
	unsigned int real_height;
	unsigned int pot_width;
	unsigned int pot_height;
	double clip_width;
	double clip_height;
	double per_eye_aspect;
	int is_stereo;
	GLuint tid;
	void *data;
};

/* make a Power-Of-Two texture from given data */
void makePotTex(const struct jpeg_img *img, struct npot_tex *tex);

void print_npot_tex(const struct npot_tex *t);

#endif

