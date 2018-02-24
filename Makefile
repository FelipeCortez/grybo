CC = clang
CFLAGS = -Wall -g -Iinclude/
CXX = clang++
CXXFLAGS = -g -std=c++11 -Wall -pedantic `sdl2-config --cflags` -Iinclude/
LIBS = `sdl2-config --libs`
OBJ = main.o gl3w.o
OUT = game

all: $(OUT)

$(OUT): $(OBJ)
	$(CXX) $(CXXFLAGS) $(OBJ) -o $(OUT) $(LIBS)

%.o: %.cpp
	$(CXX) -c $(CXXFLAGS) -o $@ $<

%.o: %.c
	$(CC) -c $(CFLAGS) -o $@ $<

clean:
	rm -f $(OBJ)
