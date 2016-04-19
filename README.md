SudokuMP - C++ and OpenMP
===

### 15-418: Parallel Computer Architecture and Programming

Parallelized Sudoku Solver project for final project in 15-418.

### USAGE

To compile code:

    $ g++ -o sudoku board.cpp 
    $ ./sudoku /path/to/file.txt dimension

where /path/to/file.txt is the file path to the Sudoku board that you wish to be solved and dimension is the width of the Sudoku board (e.g. for a 9x9 board dimension would be 9).