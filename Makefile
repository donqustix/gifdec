all: example.c gifdec.c
	gcc -DNDEBUG -Os -march=native -mtune=native $^ -o gifplay -lSDL2
