# ============================================================
#  Cartoon Dunk  –  Makefile
#  Requiere: SFML 2.x (graphics, window, system, audio)
# ============================================================

CXX      = g++
CXXFLAGS = -std=c++17 -Wall -O2 -Iinclude
LIBS     = -lsfml-graphics -lsfml-window -lsfml-system -lsfml-audio

SRC      = src/main.cpp
TARGET   = CartoonDunk

all: $(TARGET)

$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) $< -o $@ $(LIBS)

run: all
	./$(TARGET)

clean:
	rm -f $(TARGET)

.PHONY: all run clean
