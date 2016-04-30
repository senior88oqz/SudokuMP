#include <stdlib.h>
#include <math.h>

struct Board
{
  Board(int d = 9) {
    dim = d;
    inner_dim = (int)sqrt(d);

    cells = new int*[dim];
    solution = new int*[dim];
    old_changed = new bool*[dim];
    new_changed = new bool*[dim];
    for(int r = 0; r < dim; r++) {
      cells[r] = new int[dim];
      solution[r] = new int[dim];
      old_changed[r] = new bool[dim];
      new_changed[r] = new bool[dim];
    }
  }

  int dim;

  int inner_dim;

  // cells = dim x dim array of ints, each is a bit vector of possible values
  // i.e. 010000000 = 8 is the value of the cell
  // i.e. 010011010 = 2, 4, 5, 8 are possible values of cell
  int** cells;

  // solution = dim x dim board containing solutions
  int** solution;

  // old_changed = whether or not a cell changed on the last iteration
  bool** old_changed;

  //new_changed = whether or not a cell changed on this iteration
  bool** new_changed;
};

/* Global board struct */
Board* board;

/* Prints the board */
void print_board();

/* Creates a board with initialized values, store in global variable */
bool create_board(char* filename, int dim);

/* Solve the sudoku board */
void solve();

bool check_move(int **input, int dim, int inner_dim, int row, int col, int num);
