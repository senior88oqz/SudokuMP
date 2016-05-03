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

    cells = new bool**[dim];
    solution = new int*[dim];
    for(int r = 0; r < dim; r++) {
      cells[r] = new bool*[dim];
      for(int c = 0; c < dim; c++) {
        cells[r][c] = new bool[dim];
      }
      solution[r] = new int[dim];
    }
  }

  int dim;

  int inner_dim;

  // cells = dim x dim x dim array of bools representing possible values
  // If values are all zero, then the cell must be solved
  bool*** cells;

  // solution = dim x dim board containing solutions
  int** solution;

  // Number of cells solved
  int cells_solved;
};

/* Global board struct */
extern Board* board;

/* Prints the cells of possible values */
void print_cells();

/* Prints the solution board */
void print_board();

/* Creates a board with initialized values, store in global variable */
bool create_board(char* filename, int dim);

/* Solve the sudoku board */
void solve();

