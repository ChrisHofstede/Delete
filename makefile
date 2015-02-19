CC = gcc
CPPFLAGS = -Wall -O3 -D STRICT -I ../include
LDFLAGS = -lsupc++
VPATH = src

OBJECTS = main.o delete.o
EXE = delete.exe

delete.exe : $(OBJECTS)
	$(CC) -o $(EXE) $(OBJECTS) $(LDFLAGS)
	
main.o :

delete.o : delete.rc
	windres -i delete.rc -o delete.o
	
clean :
	rm -f $(EXE) $(OBJECTS)