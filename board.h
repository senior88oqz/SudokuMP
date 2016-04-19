#include <stdlib.h>

static struct board
{
  int dim;
  int inner_dim;
  // cells = dim x dim array of ints, each is a bit vector of possible values
  // i.e. 010000000 = 8 is the value of the cell
  // i.e. 010011010 = 2, 4, 5, 8 are possible values of cell
  int** cells;
} Board;

/* Board functions here */
void print_board(int **input, int dim, int inner_dim);
