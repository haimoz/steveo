#include "npot_tex.h" // implements

#include <string.h>

/* utility function to calculate the closest power of two */
unsigned int pot(unsigned int x)
{
	unsigned int y;
	for (y=1; y<x; y*=2) {
	}
	return y;
}

void makePotTex(const struct jpeg_img *img, struct npot_tex *tex)
{
	/* tid is assigned by other routined */
	/* only care about calculating the Power-Of-Two size and clip size in s-t texture coordiantes */
	tex->real_width = img->width;
	tex->real_height = img->height;
	tex->pot_width = pot(tex->real_width);
	tex->pot_height = pot(tex->real_height);
	tex->clip_width = tex->real_width / (double)tex->pot_width;
	tex->clip_height = tex->real_height / (double)tex->pot_height;
	tex->real_aspect = tex->real_width / (double)tex->real_height;
	/* correct the real aspect ratio for stereo images */
	if (img->is_stereo) {
		tex->is_stereo = 1;
		tex->real_aspect /= 2;
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

