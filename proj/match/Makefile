CC=LD_LIBRARY_PATH=/usr/local/lib g++
CFLAGS=-c -Wall -std=c++0x
LDFLAGS=-L/usr/lib64 -lodbc -lgcrypt -lSDL2 -lSDL2_net
SOURCES=../../source/match/svmain.cpp\
../../source/net/net.cpp\
../../source/net/netconn.cpp\
../../source/net/readpackets.cpp\
../../source/net/sendpackets.cpp\
../../source/sim/simflow.cpp\
../../source/utils.cpp\
../../source/net/parity.cpp\
../../source/math/fixmath.cpp\
../../source/version.cpp
OBJECTS=$(SOURCES:.cpp=.o)
EXECUTABLE=csmm

all: $(SOURCES) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS) 
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@

.cpp.o:
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -rf *o csmm
	rm -rf $(OBJECTS) 