UNAME := $(shell uname -s)
CC = g++
OBJ = miro.o pathfinder.o tga.o
LDFLAGS = -lGL -lGLU -lglut

# Mac OS
ifeq ($(UNAME), Darwin)
	LDFLAGS = -framework OpenGL -framework glut
endif

miro : $(OBJ)
	$(CC) -o miro $(OBJ) $(LDFLAGS)

miro.o : miro.cpp miro.h pathfinder.h
	$(CC) -c -g -fpermissive miro.cpp

pathfinder.o : pathfinder.cpp pathfinder.h miro.h
	$(CC) -c -g pathfinder.cpp

tga.o : tga.c tga.h miro.h pathfinder.h
	$(CC) -c -g tga.c -fpermissive

clean :
	rm $(OBJ)
