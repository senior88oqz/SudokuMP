#include "make_guess.h"

Board* create_copy_board(Board* board);

void choose_cell_bf(Board* board, int &row, int &col, int &value);

void update_solution_bf(Board* board, int row, int col, int num);

void update_stack(Board* board);

/* Solve the sudoku board using the brute force method */
void parallel_brute_force(Board* board, int total);
