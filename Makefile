SRC = $(shell find src -name *.cpp)
OBJ = $(patsubst %.cpp,%.obj,$(SRC))
CC = g++
FLAGS = -g -O4 -std=c++17 -DUSE_PL=1 -DPL_IMPL_AUTO_INSTRUMENT=1	
LIBS = -lglfw -lGL
TARGET = bin/main.bin

all: clean copy_assets $(OBJ)
	$(CC) $(FLAGS) $(OBJ) -o $(TARGET) $(LIBS)

clean:
	rm -rf $(TARGET) $(OBJ)

%.obj: %.cpp
	$(CC) -c $(FLAGS) $^ -o $@

run_main: all
	$(TARGET)

copy_assets:
	cp -rf assets bin