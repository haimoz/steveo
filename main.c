#include <stdio.h>
#include <stdlib.h>

#include "read_jpeg.h"

int main(int argc, char** argv)
{
	/* test function: 
	 * decode a JPEG file
	 * then output in csv format in the following layout:
	 * 
	 * R11, G11, B11, R12, G12, B12, ...
	 * R21, G21, B21, ...
	 * ...
	 */
	
	/* read data first */
	struct jpeg_img img;
	read_jpeg(argv[1], &img); /* pick up image file name from command line */
	/* output some info */
	printf("Dimension: %d x %d x %d\n", img.width, img.height, img.num_components);
	/* output data as text to command line, 
	 * use pipeline to redirect to file 
	 */
	int row, col;
	for (row = 0; row < img.height; ++row) {
		for (col = 0; col < img.width; ++col) {
			col ? printf(",") : 0 ; /* prefix a leading comma for any element that is not the first in the row */
			int offset = (row * img.width + col) * img.num_components;
			int comp = 0;
			for (comp = 0; comp < img.num_components; ++comp) {
				comp ? printf(",") : 0; /* prefix a leading comma to separate the components */
				printf("%d", img.data[offset+comp]);
			}
		}
		printf("\n");
	}
	return 0;
}

