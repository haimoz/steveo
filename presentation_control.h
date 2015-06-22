#ifndef PRESENTATION_CONTROL_H
#define PRESENTATION_CONTROL_H

#define LEFT  0
#define RIGHT 1

/* convergence value:
 * The convergence value can be any double value.  The unit of the value is such 
 * that a convergence value of 0 corresponds to strictly parallel viewing, i.e., the 
 * two eyes' images are centered at the axes of the two eyeballs.  A convergence 
 * value of 1 corresponds to completely merged viewing, i.e., the orthographic 
 * projection of the two eyes' images complete overlap each other.  
 */

/* size:
 * Size is any double value.  The unit of size is such that a size of 1 corresponds 
 * to the image being fully fitted into the viewport.  A negative size would rotate 
 * the image by 180 degrees.  
 */

struct presentation {
	/* initialization */
	void (*init)(void);
	/* set up for each eye's view */
	void (*set_view)(int side);
	/* change convergence value by a certain amount */
	void (*change_convergence)(double amount);
	/* set convergence value */
	void (*set_convergence)(double value);
	/* change size of the drawing by a certain amount */
	void (*change_size)(double amount);
	/* set size of the drawing */
	void (*set_size)(double value);
	/* reset */
	void (*reset_convergence)(void);
	void (*reset_size)(void);
	void (*toggle_fullscreen)(void);
	void (*set_fullscreen)(int val);
	/* getters */
	double (*get_convergence)(void);
	double (*get_size)(void);
	/* panning the image */
	void (*pan)(int dx, int dy);
	void (*reset_pan)(void);
};

#endif

