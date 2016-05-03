sudoku: board.o brute_force.o
	g++ -o sudoku brute_force.o board.o

board.o: board.cpp board.h
	g++ -c board.cpp

brute_force.o: brute_force.cpp brute_force.h
	g++ -c brute_force.cpp

clean:
	rm -f board.o brute_force.o
