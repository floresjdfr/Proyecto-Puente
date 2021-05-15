main: main.o
	./main
main.o: main.c
	gcc main.c -o main -lpthread -lm