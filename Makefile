CC=g++

CFLAGS = -std=c++11 -Wall -O2

INSTALLDIR = /usr/lib/xscreensaver
CONFIGDIR = /usr/share/xscreensaver/config

ifneq ($(glut),1)
	MAIN_CPP = main.cpp
	OUT = m3d
	CLIBS = -lX11 -lm -lGL -lGLU
else 
	MAIN_CPP = main_glut.cpp
	OUT = m3d_glut
	CLIBS = -lX11 -lm -lGL -lGLU -lglut 
endif 	

all : $(MAIN_CPP) infra m3d_target AppParams UserDefaults 
		$(CC) $(CFLAGS) -o $(OUT) $(MAIN_CPP) infra.o m3d.o AppParams.o UserDefaults.o $(CLIBS)

m3d_target : m3d.cpp m3d.h
		$(CC) $(CFLAGS) -c m3d.cpp

infra : infra.cpp
		$(CC) $(CFLAGS) -c infra.cpp

AppParams : AppParams.cpp
		$(CC) $(CFLAGS) -c AppParams.cpp	

UserDefaults : UserDefaults.cpp
		$(CC) $(CFLAGS) -c UserDefaults.cpp

install: all copy_files

copy_files : 
	cp -f ./$(OUT) $(INSTALLDIR)
	cp -f ./m3d.xml $(CONFIGDIR)

.PHONY: clean

clean :
		-rm m3d infra.o AppParams.o UserDefaults.o m3d.o