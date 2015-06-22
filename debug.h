#ifndef DEBUG_H
#define DEBUG_H

#include <stdio.h>
#include <stdlib.h>
#include <GL/gl.h>

#ifdef DEBUG
	#define DEB(x) (x)
	#define WHERE fprintf(stderr, "FILE: %s\nLINE: %d\n\n", __FILE__, __LINE__)
	#define STACK fprintf(stderr, "FUNCTION: %s\n\n", __func__)
	#define WHAT_IS(x, f) fprintf(stderr, "%s = "f"\n\n", #x, x)
	#define CHECK_GL                                                                \
	{                                                                               \
		GLuint err;                                                                 \
		if (err = glGetError()) {                                                   \
			fprintf(stderr, "FILE: %s, LINE: %d\n", __FILE__, __LINE__);            \
			fprintf(stderr, "GL Error: %s\n\n", gluErrorString(err));               \
			exit(EXIT_FAILURE);                                                     \
		}                                                                           \
	}
#else
	#define DEB(x)
	#define WHERE
	#define STACK
	#define WHAT_IS(x, f)
	#define CHECK_GL
#endif

#endif

