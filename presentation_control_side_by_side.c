#include "presentation_control_side_by_side.h"
#include "debug.h"

#include <GL/glut.h>
#include <string.h>
#include <stdio.h>

#define INIT_WIN_W 800
#define INIT_WIN_H 600

#define MIN_SIZE 0.1
#define MAX_SIZE 10.0
#define MIN_CONV 0
#define MAX_CONV 1

static int is_parallel = 1;
static double default_size = 1;
static double default_convergence = 0;
static double curr_size = 1;
static double curr_convergence = 0;
static int is_fullscreen = 0;
static struct {
	double x;
	double y;
} offset = { 0.0 , 0.0 };

void config_side_by_side(const char * const mode)
{
	if (strcmp(mode, "ovr1") == 0) {
		is_parallel = 1;
		is_fullscreen = 1;
		default_size = 0.25;
		default_convergence = 1 - 0.84802359342575073; // from OVR SDK for DK1: LensSeparationDistance / HScreenSize * 2.0;
	} else if (strcmp(mode, "ovr2") == 0) {
		is_parallel = 1;
		is_fullscreen = 1;
		default_size = 0.25;
		default_convergence = 1 - 0.84802359342575073; // TODO: check DK2's OVR::HMDInfo in the OVR SDK
	} else if (strcmp(mode, "cross") == 0) {
		is_parallel = 0;
		is_fullscreen = 0;
		default_size = 1;
		default_convergence = 0;
	} else if (strcmp(mode, "parallel") == 0) {
		is_parallel = 1;
		is_fullscreen = 0;
		default_size = 1;
		default_convergence = 0;
	} else {
		fprintf(stderr, "Unrecognized side-by-side mode: \'%s\'\n", mode);
		exit(EXIT_FAILURE);
	}
	curr_size = default_size;
	curr_convergence = default_convergence;
}

void init_side_by_side(void)
{
	glutInitDisplayMode( GLUT_DOUBLE | GLUT_RGB );
	glutInitWindowSize(INIT_WIN_W, INIT_WIN_H);
	glutCreateWindow("");
	if (is_fullscreen) {
		glutFullScreen();
	}
	glEnable(GL_SCISSOR_TEST);
}

void set_view_side_by_side(int side)
{
	int w, h, vp_w, vp_h, vp_x, vp_y;
	w = glutGet(GLUT_WINDOW_WIDTH);
	h = glutGet(GLUT_WINDOW_HEIGHT);
	vp_w = w / 2;
	vp_h = h;
	double vp_asp = vp_w / (double)vp_h;
	if (!is_parallel != (side != RIGHT)) { // is_parallel XOR side == RIGHT
		vp_x = 0;
	} else {
		vp_x = w - vp_w;
	}
	vp_y = 0;
	// setup orthographic projection
	glMatrixMode(GL_PROJECTION);
	CHECK_GL;
	glLoadIdentity();
	CHECK_GL;
	double proj_t; // top value of frustum
	proj_t = 1 / vp_asp;
	glOrtho(-1, 1, -1 * proj_t, proj_t, 0, 10);
	CHECK_GL;
	// setup scissor
	glScissor(vp_x, vp_y, vp_w, vp_h);
	CHECK_GL;
	// setup viewport
	glViewport(vp_x, vp_y, vp_w, vp_h);
	CHECK_GL;
	// clear color
	glClear(GL_COLOR_BUFFER_BIT);
	CHECK_GL;
	// setup model view transformation
	glMatrixMode(GL_MODELVIEW);
	CHECK_GL;
	glLoadIdentity();
	CHECK_GL;
	glTranslated((side == LEFT ? curr_convergence : -curr_convergence), 0.0, 0.0);
	CHECK_GL;
	glScaled(curr_size, curr_size, 1);
	CHECK_GL;
	glTranslated(offset.x, offset.y, 0.0);
}

void change_convergence_side_by_side(double amount)
{
	curr_convergence += amount;
	if (curr_convergence < MIN_CONV) {
		curr_convergence = MIN_CONV;
	} else if (curr_convergence > MAX_CONV) {
		curr_convergence = MAX_CONV;
	}
	WHAT_IS(curr_convergence, "%lf");
	return;
}

void set_convergence_side_by_side(double value)
{
	if (value < MIN_CONV) {
		curr_convergence = MIN_CONV;
	} else if (value > MAX_CONV) {
		curr_convergence = MAX_CONV;
	} else {
		curr_convergence = value;
	}
	WHAT_IS(curr_convergence, "%lf");
	return;
}

void change_size_side_by_side(double amount)
{
	curr_size += amount;
	if (curr_size < MIN_SIZE) {
		curr_size = MIN_SIZE;
	} else if (curr_size > MAX_SIZE) {
		curr_size = MAX_SIZE;
	}
	WHAT_IS(curr_size, "%lf");
	return;
}

void set_size_side_by_side(double value)
{
	if (value < MIN_SIZE) {
		curr_size = MIN_SIZE;
	} else if (value > MAX_SIZE) {
		curr_size = MAX_SIZE;
	} else {
		curr_size = value;
	}
	WHAT_IS(curr_size, "%lf");
	return;
}

void reset_convergence_side_by_side(void)
{
	curr_convergence = default_convergence;
	WHAT_IS(curr_convergence, "%lf");
	return;
}

void reset_size_side_by_side(void)
{
	curr_size = default_size;
	WHAT_IS(curr_size, "%lf");
	return;
}

void toggle_fullscreen_side_by_side(void)
{
	if (is_fullscreen) {
		glutReshapeWindow(INIT_WIN_W, INIT_WIN_H);
		glutPositionWindow(100, 100);
		is_fullscreen = 0;
	} else {
		glutFullScreen();
		is_fullscreen = 1;
	}
	return;
}

void set_fullscreen_side_by_side(int val)
{
	if (!val != !is_fullscreen) {
		if (val) {
			glutFullScreen();
			is_fullscreen = 1;
		} else {
			glutReshapeWindow(INIT_WIN_W, INIT_WIN_H);
			glutPositionWindow(100, 100);
			is_fullscreen = 0;
		}
	}
	return;
}

double get_convergence_side_by_side(void)
{
	return curr_convergence;
}

double get_size_side_by_side(void)
{
	return curr_size;
}

void pan_side_by_side(int dx, int dy)
{
	int w = glutGet(GLUT_WINDOW_WIDTH);
	double unit_per_pix = 4.0 / (double)w / curr_size;
	offset.x += dx * unit_per_pix;
	offset.y -= dy * unit_per_pix;
}

void reset_pan_side_by_side(void)
{
	offset.x = offset.y = 0.0;
	return;
}

const struct presentation presentation_side_by_side = {
	&init_side_by_side,
	&set_view_side_by_side,
	&change_convergence_side_by_side,
	&set_convergence_side_by_side,
	&change_size_side_by_side,
	&set_size_side_by_side,
	&reset_convergence_side_by_side,
	&reset_size_side_by_side,
	&toggle_fullscreen_side_by_side,
	&set_fullscreen_side_by_side,
	&get_convergence_side_by_side,
	&get_size_side_by_side,
	&pan_side_by_side,
	&reset_pan_side_by_side
};

