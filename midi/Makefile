CXX = clang++
CXXFLAGS = -g -std=c++11 -Wall -pedantic -I../include/
LIBS = -L../libs/ -lmidifile
OBJ = midi.o
OUT = miditest

all: $(OUT)

$(OUT): $(OBJ)
	$(CXX) $(CXXFLAGS) $(OBJ) -o $(OUT) $(LIBS)

%.o: %.cpp
	$(CXX) -c $(CXXFLAGS) -o $@ $<

%.o: %.c
	$(CC) -c $(CFLAGS) -o $@ $<

clean:
	rm -f $(OBJ)
