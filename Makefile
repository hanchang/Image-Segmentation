all:
	g++ -o hw3-Chang hw3-Chang.c vision.c -lm
debug:
	g++ -g -O0 -o hw3-Chang hw3-Chang.c vision.c -lm
clean:
	rm hw3-Chang binary.pgm connected.ppm
