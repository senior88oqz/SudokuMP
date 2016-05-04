CXX = g++ -m64 -std=c++11
CXXFLAGS = -Wall -fopenmp

sudoku: board.cpp board.h brute_force.cpp brute_force.h
	$(CXX) $(CXXFLAGS) -o sudoku brute_force.cpp board.cpp









