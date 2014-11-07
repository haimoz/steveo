#include "read_jpeg.h"
#include "npot_tex.h"

#include <stdio.h>
#include <stdlib.h>

#include <GL/glut.h>

#define LEFT  0
#define RIGHT 1

int stereo_mode;

void checkGLError(int n)
{
	GLuint err;
	if ((err = glGetError())) {
		printf("GL Error %d: %s\n",n,gluErrorString(err));
	}
	
	return;
}

struct jpeg_img *imgs; // image data in CPU RAM
int num_imgs; // number of images
GLuint *tids; // texture IDs
struct npot_tex *textures; // texutres

int curr_img_index; // index of image being shown
double scr_aspect = 16/9.0;

void drawImage(int side) {
	/* clear screen */
	glClearColor(0,0,0,0);
	glClear( GL_COLOR_BUFFER_BIT );
	/* setup view */
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(-scr_aspect,scr_aspect,-1,1,0,10);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(0,0,1, 0,0,0, 0,1,0);
	/* bind texture */
	glBindTexture( GL_TEXTURE_2D , tids[curr_img_index] );
	/* push vertices */
	double w, h; // size drawn on the view frustum
	double asp = textures[curr_img_index].real_aspect;
	if (asp > scr_aspect) {
		// max width
		w = scr_aspect;
		h = scr_aspect / asp;
	} else {
		// max height
		h = 1;
		w = asp;
	}
	double tex_left, tex_right;
	if (textures[curr_img_index].is_stereo) {
		if (side == LEFT) {
			tex_left = 0;
			tex_right = textures[curr_img_index].clip_width / 2;
		} else {
			tex_left = textures[curr_img_index].clip_width / 2;
			tex_right = textures[curr_img_index].clip_width;
		}
	} else {
		tex_left = 0;
		tex_right = textures[curr_img_index].clip_width;
	}
	glBegin(GL_QUADS);
		glTexCoord2d(tex_left,0);
		glVertex3d(-w,h,0);
		
		glTexCoord2d(tex_left,textures[curr_img_index].clip_height);
		glVertex3d(-w,-h,0);
		
		glTexCoord2d(tex_right,textures[curr_img_index].clip_height);
		glVertex3d(w,-h,0);
		
		glTexCoord2d(tex_right,0);
		glVertex3d(w,h,0);
	glEnd();
	return;
}

void display()
{
	if (stereo_mode) {
		glDrawBuffer(GL_BACK_LEFT);
		drawImage(LEFT);
		glDrawBuffer(GL_BACK_RIGHT);
		drawImage(RIGHT);
	} else {
		drawImage(LEFT);
	}
	/* swap buffers */
	glutSwapBuffers();
	glutPostRedisplay();
	return;
}

/* handles left/right/home/end */
void specialKeyUp(int k, GLint x, GLint y)
{
	switch (k) {
		case GLUT_KEY_LEFT:
			curr_img_index--;
			break;
		case GLUT_KEY_RIGHT:
			curr_img_index++;
			break;
		case GLUT_KEY_HOME:
			curr_img_index = 0;
			break;
		case GLUT_KEY_END:
			curr_img_index = num_imgs - 1;
			break;
		default:
			return;
	}
	curr_img_index += num_imgs;
	curr_img_index %= num_imgs;
	glutPostRedisplay();
	return;
}

int main(int argc, char** argv)
{
	/* Set the stereo mode.  
	 * stereo enabled if the first argument is `s'
	 */
	stereo_mode = (argv[1][0] == 's') ? 1 : 0;
	/* initialize glut */
	glutInit(&argc, argv);
	stereo_mode ? 
		glutInitDisplayMode( GLUT_STEREO | GLUT_DOUBLE | GLUT_RGB ) : 
		glutInitDisplayMode( GLUT_DOUBLE | GLUT_RGB );
	glutInitWindowSize(1920,1080); // full HD display
	int wnd = glutCreateWindow("BeyondStereo & ColorBless Demo");
	glutFullScreen();
	printf("OpenGL version: %s\n",glGetString(GL_VERSION));
    glutDisplayFunc(display);
    glutSpecialUpFunc(specialKeyUp);
	
	/* prepare texture images */
	num_imgs = argc - 2;
	imgs = (struct jpeg_img*)malloc(sizeof(struct jpeg_img) * num_imgs);
	int img_index;
	for (img_index=0; img_index<num_imgs; ++img_index) {
		read_jpeg(argv[img_index+2], imgs+img_index);
	}
	glActiveTexture(GL_TEXTURE0);
	checkGLError(18);
	glEnable(GL_TEXTURE_2D);
	checkGLError(15);
	/* generate texture IDs */
	tids = (GLuint *)malloc(sizeof(GLuint) * num_imgs);
	glGenTextures(num_imgs,tids);
	/* make NPOT texutres */
	textures = (struct npot_tex *)malloc(sizeof(struct npot_tex) * num_imgs);
	for (img_index=0; img_index<num_imgs; ++img_index) {
		makePotTex(imgs+img_index,textures+img_index);
		textures[img_index].tid = tids[img_index];
	}
	printf("tid: %u\nreal: %d x %d\nclip: %lf x %lf\n", 
			textures[0].tid, 
			textures[0].real_width, 
			textures[0].real_height, 
			textures[0].clip_width, 
			textures[0].clip_height
		  );
	/* bind texture objects and transfer texture data */
	for (img_index=0; img_index<num_imgs; ++img_index) {
		glBindTexture( GL_TEXTURE_2D , tids[img_index] );
		checkGLError(16);
		//glTexParameteri( GL_TEXTURE_2D , GL_TEXTURE_MAG_FILTER , GL_LINEAR );
		//glTexParameteri( GL_TEXTURE_2D , GL_TEXTURE_MIN_FILTER , GL_LINEAR );
		glPixelStorei(GL_UNPACK_ALIGNMENT,1);
		//glPixelStorei(GL_PACK_ALIGNMENT,1);
		glTexImage2D( 
				GL_TEXTURE_2D , 0 , GL_RGB , 
				textures[img_index].pot_width , textures[img_index].pot_height , 
				0 , GL_RGB , GL_UNSIGNED_BYTE , textures[img_index].data 
				);
		glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);
		checkGLError(17);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	}
	printf("created %d textures with id %ud\n",num_imgs,tids[0]);
	checkGLError(14);
	
	curr_img_index = 0;
    glutMainLoop();
	return 0;
}

