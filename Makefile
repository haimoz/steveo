jpeg2csv: main.o read_jpeg.o npot_tex.o
	gcc read_jpeg.o npot_tex.o main.o -ljpeg -lGL -lGLU -lglut -o view

main.o: main.c read_jpeg.h
	gcc -c main.c -o main.o

read_jpeg.o: read_jpeg.h read_jpeg.c
	gcc -c read_jpeg.c -o read_jpeg.o

npot_tex.o: npot_tex.h npot_tex.c
	gcc -c npot_tex.c -o npot_tex.o

clean:
	rm -f main.o read_jpeg.o view

