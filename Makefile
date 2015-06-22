steveo: main.o read_jpeg.o image_manager.o npot_tex.o presentation_control_side_by_side.o
	gcc presentation_control_side_by_side.o npot_tex.o read_jpeg.o image_manager.o main.o -ljpeg -lGL -lGLU -lglut -o steveo

main.o: main.c read_jpeg.h npot_tex.h image_manager.h presentation_control.h presentation_control_side_by_side.h debug.h
	gcc -c main.c -o main.o -DDEBUG

read_jpeg.o: read_jpeg.h read_jpeg.c debug.h
	gcc -c read_jpeg.c -o read_jpeg.o -DDEBUG

npot_tex.o: npot_tex.h npot_tex.c debug.h
	gcc -c npot_tex.c -o npot_tex.o -DDEBUG

image_manager.o: npot_tex.h read_jpeg.h image_manager.h image_manager.c debug.h
	gcc -c image_manager.c -o image_manager.o -DDEBUG

presentation_control_side_by_side.o: presentation_control.h presentation_control_side_by_side.h presentation_control_side_by_side.c debug.h
	gcc -c presentation_control_side_by_side.c -o presentation_control_side_by_side.o -DDEBUG

clean:
	rm -f *.o steveo

