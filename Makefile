CXX = g++ -m64 -std=c++11
CXXFLAGS = -Wall -fopenmp

sudoku: board.cpp board.h make_guess.cpp make_guess.h
	$(CXX) $(CXXFLAGS) -o sudoku make_guess.cpp board.cpp









