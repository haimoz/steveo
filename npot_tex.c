#include "npot_tex.h" // implements
#include "debug.h"

#include <string.h>
#include <stdio.h>

#define PRT(x, f) printf(#x" = "f"\n", x)

/* utility function to calculate the closest power of two */
unsigned int pot(unsigned int x)
{
	unsigned int y;
	for (y=1; y<x; y*=2) {
	}
	return y;
}

void print_npot_tex(const struct npot_tex *t)
{
	PRT(t->real_width, "%d");
	PRT(t->real_height, "%d");
	PRT(t->pot_width, "%d");
	PRT(t->pot_height, "%d");
	PRT(t->clip_width, "%lf");
	PRT(t->clip_height, "%lf");
	PRT(t->per_eye_aspect, "%lf");
	PRT(t->is_stereo, "%d");
	PRT(t->tid, "%d");
	PRT(t->data, "%p");
	putchar('\n');
	return;
}

void makePotTex(const struct jpeg_img *img, struct npot_tex *tex)
{
	/* tid is assigned by other routines */
	/* only care about calculating the Power-Of-Two size and clip size in s-t texture coordiantes */
	tex->real_width = img->width;
	tex->real_height = img->height;
	tex->pot_width = pot(tex->real_width);
	tex->pot_height = pot(tex->real_height);
	tex->clip_width = tex->real_width / (double)tex->pot_width;
	tex->clip_height = tex->real_height / (double)tex->pot_height;
	tex->per_eye_aspect = tex->real_width / (double)tex->real_height;
	/* correct the real aspect ratio for stereo images */
	if (img->is_stereo) {
		tex->is_stereo = 1;
		tex->per_eye_aspect /= 2;
	} else {
		tex->is_stereo = 0;
	}
	/* copy data and pad */
	tex->data = malloc(tex->pot_width * tex->pot_height * 3);
	int rowStride = tex->pot_width * 3;
	int rowLength = tex->real_width * 3;
	unsigned char *imgPtr;
	unsigned char *dataPtr;
	int row;
	for (imgPtr=img->data, dataPtr=tex->data, row=0; row < tex->real_height; imgPtr+=rowLength, row++, dataPtr+=rowStride) {
		memcpy(dataPtr, imgPtr, rowLength);
	}
	return;
}

