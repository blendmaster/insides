
CC = g++
OPT = -I.

LIBOPT = -L/usr/X11R6/lib -L/usr/local/lib -lglut  -lGL -lm

all : insides

insides : insides.o trackball.o
	$(CC) -o insides insides.o trackball.o $(LIBOPT) -lGLEW

%.o: %.c *.h Makefile
	$(CC) $(OPT) -c -o $@ $<

clean :
	rm *.o insides

