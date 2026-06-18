# ================================================================
#  Cartoon Dunk  -  Makefile
#  Compilar: make
#  Ejecutar: make run
# ================================================================
CXX      = g++
CXXFLAGS = -std=c++17 -O2 -Wall -Wextra -Wno-unused-parameter -Iinclude
LIBS     = -lsfml-graphics -lsfml-window -lsfml-system -lsfml-audio
TARGET   = CartoonDunk

all: $(TARGET)

$(TARGET): src/main.cpp include/*.hpp
	$(CXX) $(CXXFLAGS) src/main.cpp -o $(TARGET) $(LIBS)

run: all
	./$(TARGET)

clean:
	rm -f $(TARGET)

.PHONY: all run clean
