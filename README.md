SudokuMP - C++ and OpenMP
===

### 15-418: Parallel Computer Architecture and Programming

Parallelized Sudoku Solver project for final project in 15-418.

### Usage

To compile code:

    $ make
    $ ./sudoku /path/to/file.txt dimension

where /path/to/file.txt is the file path to the Sudoku board that you wish to be solved and dimension is the width of the Sudoku board (e.g. for a 9x9 board dimension would be 9).

### Tests

Tests are categorized as easy, medium, hard, or bruteforce

Easy - can be solved with elimination and lone ranger.
Medium - can be solved with elimination, lone ranger, and twins.
Hard - can be solved with elimination, lone ranger, twins, and triplets.
Bruteforce - need guesswork to solve.
