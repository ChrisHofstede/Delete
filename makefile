CC = gcc
CPPFLAGS = -Wall -O3 -D STRICT -I ../include
LDFLAGS = -lsupc++
VPATH = src

OBJECTS = main.o delete.o
EXE = delete.exe

$(EXE) : $(OBJECTS)
	$(CC) -o $(EXE) $(OBJECTS) $(LDFLAGS)
	
main.o : main.cpp

delete.o : delete.rc delete.ico
	windres -i delete.rc -o delete.o
	
clean :
	rm -f $(EXE) $(OBJECTS)