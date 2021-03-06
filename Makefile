CC = clang
CFLAGS = -Wall -g -Iinclude/
CXX = clang++
CXXFLAGS = -g -std=c++11 -Wall -pedantic `sdl2-config --cflags` -Iinclude/
LIBS = `sdl2-config --libs` -Llibs/ -lassimp -lmidifile -lsoundio
HEADERONLY = shapes.h midi.h model.h mesh.h shader.h
BINFOLDER = bin/
OBJ = main.o audio.o gl3w.o imgui/imgui.o imgui/imgui_demo.o imgui/imgui_draw.o imgui/imgui_impl_sdl_gl3.o stb_vorbis.o
OUT = $(BINFOLDER)game

all: $(OUT)
$(OUT): $(OBJ)
	ls bin||mkdir bin
	$(CXX) $(CXXFLAGS) $(OBJ) -o $(OUT) $(LIBS)
	mv *.o $(BINFOLDER)
%.o: %.cpp $(HEADERONLY)
	$(CXX) -c $(CXXFLAGS) -o $@ $<

%.o: %.c
	$(CC) -c $(CFLAGS) -o $@ $<

clean:
	rm -rf $(BINFOLDER)
