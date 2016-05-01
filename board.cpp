#include "board.h"
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <string>
#include <iostream>
#include <sstream>

enum Align {ROW, COL, BLOCK};

/************ Helper functions **************************************/

/* Turn i, j, align into row, col */
void index_to_row_col(int i, int j, Align align, int &row, int &col) {
  if (align == ROW) {
    row = i;
    col = j;
  } else if (align == COL) {
    row = j;
    col = i;
  } else {
    row = (i/board->inner_dim)*board->inner_dim + (j/board->inner_dim);
    col = (i%board->inner_dim)*board->inner_dim + (j%board->inner_dim);
  }
}

/* Return true if old_changed is all zeroes */
bool nothing_has_changed() {
  for(int r = 0; r < board->dim; r++) {
    for(int c = 0; c < board->dim; c++) {
      if (board->old_changed[r][c])
        return 0;
    }
  }
  return 1;
}

/* Swap old_changed and new_changed, clear new_changed */
void clear_changed() {
  bool** temp = board->new_changed;
  board->new_changed = board->old_changed;
  board->old_changed = temp;

  for(int r = 0; r < board->dim; r++) {
    for(int c = 0; c < board->dim; c++) {
      board->new_changed = 0;
    }
  }
}

/* Remove all instances of value from row/col/block i */
void clear_number(int i, Align align, int value) {
  int mask = ~ (1 << (value-1));
  if(align == ROW) {
    for(int c = 0; c < board->dim; c++)
      board->cells[i][c] = mask & board->cells[i][c];
  } else if (align == COL) {
    for(int r = 0; r < board->dim; r++)
      board->cells[r][i] = mask & board->cells[r][i];
  } else {
    int id = board->inner_dim;
    for(int r = (i/id) * id; r < (i/id)*id + id; r++) {
      for(int c = (i%id) * id; c < (i%id)*id + id; c++) {
        board->cells[r][c] = mask & board->cells[r][c];
      }
    }
  }
}


/* Test is elimination is possible (i.e. 010000000 -> 8)
 * If possible, update solution and remove num from row, col, and block
 */
bool elimination(int i, int j, Align align) {
  int row, col, value, num = 1;
  index_to_row_col(i, j, align, row, col);

  if (!board->solution[row][col] && board->old_changed[row][col]) {
    value = board->cells[row][col];
    while(value % 2 == 0) {
      value = value/2;
      num++;
    }
    if(value == 1) {
      int id = board->inner_dim;
      board->solution[row][col] = num;
      clear_number(row, ROW, num);
      clear_number(col, COL, num);
      clear_number((row/id)*id + (col/id), BLOCK, num);
      return 1;
    }
  }
  return 0;

}

void loneranger_twins_triplets(int i, Align align) {

}

/********************************************************************/


void print_board(){
    std::string line = "";
    std::ostringstream stm;
    for(int r = 0; r < board->dim; r++) {
      for(int c = 0; c < board->dim; c++) {
        stm << board->solution[r][c];
        stm << " ";
        if((c+1) % board->inner_dim == 0)
          stm << " ";
      }
      stm << "\n";
      if((r+1) % board->inner_dim == 0)
        stm << "\n";
    }
    std::cout << stm.str();
}

bool check_move(int **input, int dim, int inner_dim, int row, int col, int num){
	for (int i = 0; i < dim; i++){
		if (input[i][col] == num){
			return 0;
		}
		else if (input[row][i] == num){
			return 0;
		}
	}

	int box_row = (row - (row % inner_dim));
	int box_col = (col - (col % inner_dim));

	for (int new_row = box_row; new_row < (box_row + inner_dim); new_row++){
		for (int new_col = box_col; new_col < (box_col + inner_dim); new_col++){
			if (input[new_row][new_col] == num){
				return false;
			}
		}
	}
	return true;
}

bool create_board(const char* filename, int dim) {
  FILE *file = fopen(filename, "r");
  if(!file) return 0;

  board = new Board(dim);

  int num, bits;
  for(int row = 0; row < dim; row++) {
    for(int col = 0; col < dim; col++) {
      fscanf(file, "%d", &num);
      if(num == 0)
        bits = (1 << dim) - 1;
      else
        bits = 1 << (num-1);
      board->solution[row][col] = num;
      board->cells[row][col] = bits;
      board->old_changed[row][col] = 1;
      board->new_changed[row][col] = 0;
    }
  }

  // Reset cells based on numbers given
  int id = board->inner_dim;
  for(int row = 0; row < dim; row++) {
    for(int col = 0; col < dim; col++) {
      if(board->solution[row][col]) {
        int num = board->solution[row][col];
        clear_number(row, ROW, num);
        clear_number(col, COL, num);
        clear_number((row/id)*id + (col/id), BLOCK, num);
      }
    }
  }
  return 1;
}

/* Solve using block using elimination, lone ranger, twins, and triplets
 * If nothing has changed, use brute force
 */
void solve_block(int i) {
  for(int j = 0; j < board->dim; j++) {
    elimination(i, j, BLOCK);
  }

  loneranger_twins_triplets(i, ROW);
  loneranger_twins_triplets(i, COL);
  loneranger_twins_triplets(i, BLOCK);
}

void solve() {
  if (nothing_has_changed()) {
    // Brute force solution
    return;
  }

  // Repeat this loop while not solved
  for(int thread_id = 0; thread_id < board->dim; thread_id++) {
    solve_block(thread_id);
  }


}

int main(int argc, const char* argv[]){
	if (argc == 3){
		int path_max;

		#ifdef PATH_MAX
			path_max = PATH_MAX;
		#else
			path_max = pathconf(path, _PC_PATH_MAX);
			if (path_max <= 0){
				path_max = 4096;
			}
		#endif

		char resolved_path[path_max];
		char *rp = realpath(argv[1], resolved_path);

		if(!rp){
			std::cerr << "Error: Path to text file for puzzle is not valid.\n";
			return 0;
		}

		int dim = atoi(argv[2]);
		if ((int)pow((float)sqrt(dim), 2.0) != dim) {
			std::cerr << "Error: Dimension is not a perfect square.\n";
			return 0;
		}

    if (!create_board((const char*) rp, dim)) {
      std::cerr << "Error: Problem in create_board.\n";
      return 0;
    }

    solve();

		print_board();
	}
	else {
		std::cerr << "Error: No text file path for puzzle included.";
		return 0;
	}
}
