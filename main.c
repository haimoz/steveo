#include "read_jpeg.h"
#include "npot_tex.h"
#include "image_manager.h"
#include "presentation_control.h"
#include "presentation_control_side_by_side.h"
#include "debug.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>

#include <GL/glut.h>

#define LEFT  0
#define RIGHT 1

#define SCROLL_FACTOR 0.1
#define PARAM_CHANGE_RATE_PER_SECOND 1.2

const struct presentation *presenter;

struct timespec down_time[1 << (8 * sizeof(char))] = { 0 };
int is_kb_busy = 0; // whether the keyboard (non-special keys in GLUT context) is busy

struct {
	int x;
	int y;
} drag_start = { -1, -1 };
int is_dragging = 0;

double time_diff(const struct timespec *start, const struct timespec *end)
{
	struct timespec diff;
	if (start->tv_sec > end->tv_sec) {
		fprintf(stderr, "In %s: end time %ds:%ldns is earlier than start time %ds:%ldns\n", __func__, end->tv_sec, end->tv_nsec, start->tv_sec, start->tv_nsec);
		exit(EXIT_FAILURE);
	}
	diff.tv_sec = end->tv_sec - start->tv_sec;
	if (start->tv_nsec > end->tv_nsec) {
		if (--diff.tv_sec < 0) {
			fprintf(stderr, "In %s: end time %ds:%ldns is earlier than start time %ds:%ldns\n", __func__, end->tv_sec, end->tv_nsec, start->tv_sec, start->tv_nsec);
			exit(EXIT_FAILURE);
		} else {
			diff.tv_nsec = 1000000000 - start->tv_nsec + end->tv_nsec;
		}
	} else {
		diff.tv_nsec = end->tv_nsec - start->tv_nsec;
	}
	return (double)diff.tv_nsec * 1e-9 + diff.tv_sec;
}

int is_zero_timespec(const struct timespec *ts)
{
	return ts->tv_sec == 0 && ts->tv_nsec == 0;
}

/* draw the image in the Z = 0 plane, centered at the origin, with a width of 2, 
 * and height according to the aspect ratio of the designated viewport.  
 */
void drawImage(int side) {
	GLint data[4];
	glGetIntegerv(GL_VIEWPORT, data);
	CHECK_GL;
	double vp_asp = data[2] / (double)data[3];
	WHAT_IS(data[0], "%d");
	WHAT_IS(data[1], "%d");
	WHAT_IS(data[2], "%d");
	WHAT_IS(data[3], "%d");
	WHAT_IS(vp_asp, "%lf");
	const struct npot_tex *tex = get_curr_tex();
	double quad_left, quad_right, quad_top, quad_bottom;
	double tex_left, tex_right, tex_top, tex_bottom;
	if (tex->per_eye_aspect > vp_asp) {
		// fill width
		quad_left = -1;
		quad_right = 1;
		quad_top = 1 / tex->per_eye_aspect;
		quad_bottom = -1 * quad_top;
	} else {
		// fill height
		quad_top = 1 / vp_asp;
		quad_bottom = -1 * quad_top;
		quad_right = quad_top * tex->per_eye_aspect;
		quad_left = -1 * quad_right;
	}
	if (tex->is_stereo) {
		if (side == LEFT) {
			tex_left = 0;
			tex_right = tex->clip_width / 2;
		} else {
			tex_left = tex->clip_width / 2;
			tex_right = tex->clip_width;
		}
	} else {
		tex_left = 0;
		tex_right = tex->clip_width;
	}
	tex_top = 0;
	tex_bottom = tex->clip_height;
	glBindTexture(GL_TEXTURE_2D, tex->tid);
	WHAT_IS(tex_left, "%lf");
	WHAT_IS(tex_right, "%lf");
	WHAT_IS(tex_top, "%lf");
	WHAT_IS(tex_bottom, "%lf");
	WHAT_IS(quad_left, "%lf");
	WHAT_IS(quad_right, "%lf");
	WHAT_IS(quad_top, "%lf");
	WHAT_IS(quad_bottom, "%lf");
	CHECK_GL;
	glBegin(GL_QUADS);
	glTexCoord2d(tex_left, tex_top);
	glVertex2d(quad_left, quad_top);
	glTexCoord2d(tex_right, tex_top);
	glVertex2d(quad_right, quad_top);
	glTexCoord2d(tex_right, tex_bottom);
	glVertex2d(quad_right, quad_bottom);
	glTexCoord2d(tex_left, tex_bottom);
	glVertex2d(quad_left, quad_bottom);
	glEnd();
	CHECK_GL;
	return;
}

void display()
{
	WHERE;
	STACK;
	
	double amount;
	struct timespec now;
	struct timespec *pt_less_size, *pt_more_size, *pt_less_conv, *pt_more_conv;
	int clock_err;
	/* handle parameter change */
	if (is_kb_busy > 0) {
		if (clock_err = clock_gettime(CLOCK_MONOTONIC, &now)) {
			fprintf(stderr, "Failed to get time, returned %d\n", clock_err);
			exit(EXIT_FAILURE);
		}
		pt_less_size = down_time + '-';
		pt_more_size = down_time + '=';
		pt_less_conv = down_time + ',';
		pt_more_conv = down_time + '.';
		if (!is_zero_timespec(pt_less_size) && is_zero_timespec(pt_more_size)) {
			amount = PARAM_CHANGE_RATE_PER_SECOND * time_diff(pt_less_size, &now) * presenter->get_size();
			presenter->change_size(-amount);
		} else if (is_zero_timespec(pt_less_size) && !is_zero_timespec(pt_more_size)) {
			amount = PARAM_CHANGE_RATE_PER_SECOND * time_diff(pt_more_size, &now) * presenter->get_size();
			presenter->change_size(amount);
		}
		if (!is_zero_timespec(pt_less_conv) && is_zero_timespec(pt_more_conv)) {
			amount = PARAM_CHANGE_RATE_PER_SECOND * time_diff(pt_less_conv, &now);
			presenter->change_convergence(-amount);
		} else if (is_zero_timespec(pt_less_conv) && !is_zero_timespec(pt_more_conv)) {
			amount = PARAM_CHANGE_RATE_PER_SECOND * time_diff(pt_more_conv, &now);
			presenter->change_convergence(amount);
		}
		if (!is_zero_timespec(pt_less_size)) {
			*pt_less_size = now;
		}
		if (!is_zero_timespec(pt_more_size)) {
			*pt_more_size = now;
		}
		if (!is_zero_timespec(pt_less_conv)) {
			*pt_less_conv = now;
		}
		if (!is_zero_timespec(pt_more_conv)) {
			*pt_more_conv = now;
		}
		glutPostRedisplay(); // we wanna update the size and convergence in real-time
	}
	
	int side;
	for (side = LEFT; side <= RIGHT; ++side) {
		presenter->set_view(side);
		drawImage(side);
	}
	
	glutSwapBuffers();
	return;
}

/* handles left/right/home/end */
void specialKeyUp(int k, GLint x, GLint y)
{
	WHERE;
	STACK;
	switch (k) {
		case GLUT_KEY_LEFT:
			to_prev_image();
			//presenter->reset_pan();
			break;
		case GLUT_KEY_RIGHT:
			to_next_image();
			//presenter->reset_pan();
			break;
		case GLUT_KEY_HOME:
			to_first_image();
			//presenter->reset_pan();
			break;
		case GLUT_KEY_END:
			to_last_image();
			//presenter->reset_pan();
			break;
		case GLUT_KEY_F11:
			presenter->toggle_fullscreen();
			break;
		default:
			return;
	}
	glutPostRedisplay();
	return;
}

void keyDown(unsigned char k, int x, int y)
{
	WHERE;
	STACK;
	++is_kb_busy;
	int clock_err;
	if (clock_err = clock_gettime(CLOCK_MONOTONIC, down_time + k)) {
		fprintf(stderr, "Failed to get time for key \'%c\'\n", k);
		fprintf(stderr, "error code of `clock_gettime\': %d\n", clock_err);
		exit(EXIT_FAILURE);
	}
	if (k == 'r') {
		presenter->reset_convergence();
		presenter->reset_size();
		presenter->reset_pan();
	}
	glutPostRedisplay(); // handle the parameter change in the display loop
	return;
}

void keyUp(unsigned char k, int x, int y)
{
	WHERE;
	STACK;
	if (--is_kb_busy < 0) {
		/* This is to prevent key up event when starting the application.  
		 * Sometimes, there will be a key up event for '\n' when the program is 
		 * started with a press of the enter key.  This key up event will be sent 
		 * to GLUT but the corresponding key down event is processed by the OS, 
		 * resulting in the `is_kb_busy' variable reduced to -1 when this happens.  
		 */
		is_kb_busy = 0;
	}
	down_time[k] = (struct timespec) { 0 , 0 };
	glutPostRedisplay();
	return;
}

void mouseMotion(int x, int y)
{
	is_dragging = 1;
	presenter->pan(x - drag_start.x, y - drag_start.y);
	drag_start.x = x;
	drag_start.y = y;
	glutPostRedisplay();
	return;
}

void mouse(int button, int state, int x, int y)
{
	WHAT_IS(button, "%d");
	WHAT_IS(state, "%d");
	if (button == GLUT_RIGHT_BUTTON && state == GLUT_UP) {
		to_prev_image();
		//presenter->reset_pan();
		drag_start.x = drag_start.y = -1;
		glutPostRedisplay();
	} else if (button == GLUT_LEFT_BUTTON) {
		if (state == GLUT_DOWN) {
			drag_start.x = x;
			drag_start.y = y;
			glutMotionFunc(mouseMotion);
		} else {
			if (!is_dragging) {
				to_next_image();
				//presenter->reset_pan();
				drag_start.x = drag_start.y = -1;
				glutPostRedisplay();
			}
			glutMotionFunc(NULL);
			is_dragging = 0;
		}
	} else if (button == 3 || button == 4) {
		/* scroll wheels as mouse buttons are reported as these buttons down 
		 * immediately followed by an up
		 */
		if (state == GLUT_DOWN) {
			double delta_size = SCROLL_FACTOR * presenter->get_size() * PARAM_CHANGE_RATE_PER_SECOND;
			presenter->change_size(button == 3 ? delta_size : -delta_size);
			glutPostRedisplay();
		}
	}
	return;
}

int main(int argc, char** argv)
{
	/* parse arguments */
	int option;
	switch (option = getopt(argc, argv, "m:")) {
	case 'm':
		/* set stereo mode */
		if (strcmp(optarg, "nv3d") == 0) {
			fprintf(stderr, "NVidia 3D Vision support is not yet implemented\n");
			exit(EXIT_FAILURE);
		} else if (strcmp(optarg, "ovr1") == 0) {
			config_side_by_side(optarg);
			presenter = &presentation_side_by_side;
		} else if (strcmp(optarg, "ovr2") == 0) {
			config_side_by_side(optarg);
			presenter = &presentation_side_by_side;
		} else if (strcmp(optarg, "cross") == 0) {
			config_side_by_side(optarg);
			presenter = &presentation_side_by_side;
		} else if (strcmp(optarg, "parallel") == 0) {
			config_side_by_side(optarg);
			presenter = &presentation_side_by_side;
		} else {
			fprintf(stderr, "Unrecognized stereo mode: \'%s\'\n", optarg);
			exit(EXIT_FAILURE);
		}
		break;
	default:
		fprintf(stderr, "Unrecognized option character: \'%c\'\n", option);
		exit(EXIT_FAILURE);
		break;
	}
	
	/* initialize glut */
	glutInit(&argc, argv);
	presenter->init();
	printf("OpenGL version: %s\n",glGetString(GL_VERSION));
	glutSetWindowTitle("steveo");
	
	/* initialize image manager */
	load_images(argc - 3, (const char * const *)argv + 3);
	
	/* register program components */
	glutDisplayFunc(display);
	glutSpecialUpFunc(specialKeyUp);
	glutKeyboardFunc(keyDown);
	glutKeyboardUpFunc(keyUp);
	glutMouseFunc(mouse);
	
	glutMainLoop();
	return 0;
}

