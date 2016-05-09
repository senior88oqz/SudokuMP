#include "make_guess.h"

/* Global stack storing boards for brute force */
extern std::stack<Board*> boards;


/* Solve the sudoku board using the brute force method */
void parallel_brute_force();
