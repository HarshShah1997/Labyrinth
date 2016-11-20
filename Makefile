UNAME := $(shell uname -s)
CC = g++
OBJ = miro.o 
LDFLAGS = -lGL -lGLU -lglut -lSOIL

miro : $(OBJ)
	$(CC) -o miro $(OBJ) $(LDFLAGS)

miro.o : miro.cpp miro.h 
	$(CC) -c -g -fpermissive miro.cpp

clean :
	rm $(OBJ)
