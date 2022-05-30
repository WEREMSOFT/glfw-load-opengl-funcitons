SRC = $(shell find src -name *.cpp)
OBJ = $(patsubst %.cpp,%.obj,$(SRC))
CC = g++
FLAGS = -g -O0
LIBS = -lglfw -lGL
TARGET = bin/main.bin

all: clean $(OBJ)
	$(CC) $(FLAGS) $(OBJ) -o $(TARGET) $(LIBS)

clean:
	rm -rf $(TARGET) $(OBJ)

%.obj: %.cpp
	$(CC) -c $(FLAGS) $^ -o $@

run_main: all
	$(TARGET)
