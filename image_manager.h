#ifndef IMAGA_MANAGER_H
#define IMAGA_MANAGER_H

/* The image manager handles reading and decoding of an image from the file system, 
 * and creation of OpenGL textures.  It also handles browsing of the images.  
 */

#include "npot_tex.h"

void load_images(int n, const char * const *filenames);
/* get the current texture that should be used for rendering */
const struct npot_tex *get_curr_tex(void);
/* images browsing */
void to_next_image(void);
void to_prev_image(void);
void to_first_image(void);
void to_last_image(void);

#endif

