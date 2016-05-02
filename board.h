#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <math.h>
#include <string>
#include <iostream>
#include <time.h>

struct Board
{
  Board(int d = 9) {
    dim = d;
    inner_dim = (int)sqrt(d);
    cells_solved = 0;

    cells = new int*[dim];
    solution = new int*[dim];
    for(int r = 0; r < dim; r++) {
      cells[r] = new int[dim];
      solution[r] = new int[dim];
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

  // Number of cells solved
  int cells_solved;
};

/* Global board struct */
Board* board;

/* Prints the cells of possible values */
void print_cells();

/* Prints the solution board */
void print_board();

/* Creates a board with initialized values, store in global variable */
bool create_board(char* filename, int dim);

/* Solve the sudoku board */
void solve();

bool check_move(int **input, int dim, int inner_dim, int row, int col, int num);

bool brute_force(int** input, int dim, int inner_dim, int row, int col);

