CC=LD_LIBRARY_PATH=/usr/local/lib64 g++ `pkg-config --cflags --libs glib-2.0` -I/usr/local/include/SDL2/ -I/usr/local/include/ -I/usr/include/ -I../../../bluez-5.41/lib/
CFLAGS=-c -Wall -std=c++0x
LDFLAGS=-L/usr/lib64 -lGL -lSDL2 -lGLEW -lpng -ljpeg -lbluetooth
SOURCES=../../source/app/appmain.cpp\
../../source/platform.cpp
OBJECTS=$(SOURCES:.cpp=.o)
EXECUTABLE=../../testfolder/pl

all: $(SOURCES) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS) 
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@

.cpp.o:
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -rf *o $(EXECUTABLE)
	rm -rf $(OBJECTS) 
