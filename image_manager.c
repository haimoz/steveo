#include "image_manager.h"

#include "debug.h"
#include "read_jpeg.h"
#include "npot_tex.h"
#include <stdlib.h>
#include <GL/gl.h>
#include <string.h>

static struct jpeg_img *imgs = NULL; // image data in CPU RAM
static int num_imgs = 0; // number of images
static GLuint *tids = NULL; // texture IDs
static struct npot_tex *textures = NULL; // texutres
static int curr_img_index = 0; // index of image being shown
static char **img_names = NULL;

static void clear_images(void)
{
	int i;
	if (imgs) {
		for (i = 0; i < num_imgs; ++i) {
			if (imgs[i].data) {
				free(imgs[i].data);
			}
		}
		free(imgs);
		imgs = NULL;
	}
	if (tids) {
		glDeleteTextures(num_imgs, tids);
		CHECK_GL;
		free(tids);
	}
	if (textures) {
		for (i = 0; i < num_imgs; ++i) {
			if (textures[i].data) {
				free(textures[i].data);
			}
		}
		free(textures);
		textures = NULL;
	}
	if (img_names) {
		for (i = 0; i < num_imgs; ++i) {
			if (img_names[i]) {
				free(img_names+i);
			}
		}
		free(img_names);
		img_names = NULL;
	}
	num_imgs = 0;
	curr_img_index = 0;
	return;
}

void load_images(int n, const char * const *filenames)
{
	clear_images();
	
	/* decode images */
	num_imgs = n;
	imgs = (struct jpeg_img*)calloc(num_imgs, sizeof(struct jpeg_img));
	img_names = (char **)calloc(num_imgs, sizeof(char *));
	int i;
	for (i = 0; i < n; ++i) {
		img_names[i] = strdup(filenames[i]);
		read_jpeg(filenames[i], imgs + i);
	}
	glEnable(GL_TEXTURE_2D);
	CHECK_GL;
	
	/* generate texture IDs */
	tids = (GLuint *)calloc(num_imgs, sizeof(GLuint));
	glGenTextures(num_imgs, tids);
	CHECK_GL;
	
	/* make NPOT texutres */
	textures = (struct npot_tex *)calloc(num_imgs, sizeof(struct npot_tex));
	for (i = 0; i < n; ++i) {
		makePotTex(imgs + i, textures + i);
		textures[i].tid = tids[i];
	}
	
	/* bind texture objects and transfer texture data */
	for (i = 0; i < n; ++i) {
		glBindTexture( GL_TEXTURE_2D , tids[i] );
		CHECK_GL;
		//glTexParameteri( GL_TEXTURE_2D , GL_TEXTURE_MAG_FILTER , GL_LINEAR );
		//glTexParameteri( GL_TEXTURE_2D , GL_TEXTURE_MIN_FILTER , GL_LINEAR );
		glPixelStorei(GL_UNPACK_ALIGNMENT,1);
		CHECK_GL;
		//glPixelStorei(GL_PACK_ALIGNMENT,1);
		glTexImage2D( 
				GL_TEXTURE_2D , 0 , GL_RGB , 
				textures[i].pot_width , textures[i].pot_height , 
				0 , GL_RGB , GL_UNSIGNED_BYTE , textures[i].data 
				);
		CHECK_GL;
		glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);
		CHECK_GL;
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		CHECK_GL;
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		CHECK_GL;
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		CHECK_GL;
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		CHECK_GL;
		print_npot_tex(textures + i);
	}
	
	curr_img_index = 0;
}

/* get the current texture that should be used for rendering */
const struct npot_tex *get_curr_tex(void)
{
	return &textures[curr_img_index];
}

/* images browsing */
void to_next_image(void)
{
	if (++curr_img_index >= num_imgs) {
		curr_img_index -= num_imgs;
	}
	return;
}

void to_prev_image(void) {
	if (--curr_img_index < 0) {
		curr_img_index += num_imgs;
	}
	return;
}

void to_first_image(void)
{
	curr_img_index = 0;
	return;
}

void to_last_image(void)
{
	curr_img_index = num_imgs - 1;
	return;
}

