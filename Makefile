SOURCE_FILES = main.cpp Bus.cpp CPULR35902.cpp PPU.cpp Screen.cpp
LIBS = -lsfml-graphics -lsfml-window -lsfml-system

all:
	g++ -g $(SOURCE_FILES) -o tameboy $(LIBS)

