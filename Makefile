regions.o: regions.c regions.h
	clang -Wall -c regions.c -o regions.o
main: regions.o main.c regions.h
	clang -Wall main.c regions.o -o main
maindndebug: regions.o main.c regions.h
	clang -DNDEBUG main.c regions.o -o maindnd