main: main.o
	./main.o
main.o: semaforo.h main.c
	gcc main.c -o main.o -lpthread -lm