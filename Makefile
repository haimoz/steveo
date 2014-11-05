jpeg2csv: main.o read_jpeg.o
	gcc main.o read_jpeg.o -ljpeg -o jpeg2csv

main.o: main.c read_jpeg.h
	gcc -c main.c -o main.o

read_jpeg.o: read_jpeg.h read_jpeg.c
	gcc -c read_jpeg.c -o read_jpeg.o

clean:
	rm -f main.o read_jpeg.o jpeg2csv testimg.csv

test: jpeg2csv
	@./jpeg2csv testimg.jpg > testimg.csv.tmp
	@head testimg.csv.tmp -n1
	@sed <testimg.csv.tmp '1d' > testimg.csv
	@rm -f testimg.csv.tmp
	@octave test.m
