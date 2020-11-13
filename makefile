CXX = clang++

INC = -I./include
LIB = -L./lib

CXXFLAGS = --target=x86_64-pc-window-mingw -Wall -W -Wfatal-errors $(INC) $(LIB)

all:
	$(CXX) $(CXXFLAGS) main.cpp -o ./bin/game.exe -lncursesw
