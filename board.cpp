#include "brute_force.h"
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <string>
#include <iostream>
#include <sstream>

enum Align {ROW, COL, BLOCK};

Board* board;
std::stack<State*> states;

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

/* Remove all instances of value from row/col/block i */
void clear_number(int i, Align align, int value) {
  int index = value-1;
  if(align == ROW) {
    for(int c = 0; c < board->dim; c++)
      board->cells[i][c][index] = 0;
  } else if (align == COL) {
    for(int r = 0; r < board->dim; r++)
      board->cells[r][i][index] = 0;
  } else {
    int id = board->inner_dim;
    for(int r = (i/id) * id; r < (i/id)*id + id; r++) {
      for(int c = (i%id) * id; c < (i%id)*id + id; c++) {
        board->cells[r][c][index] = 0;
      }
    }
  }
}


/* Test is elimination is possible (i.e. 010000000 -> 8)
 * If possible, update solution and remove num from row, col, and block
 */
bool elimination(int i, int j, Align align) {
  int row, col, value_found;
  bool found = 0;
  index_to_row_col(i, j, align, row, col);

  if (!board->solution[row][col]) {
    for(int value = 0; value < board->dim; value++) {
      if(board->cells[row][col][value]) {
        if(found) {
          return 0;
        }
        found = 1;
        value_found = value;
      }
    }

    if(!found) {
      std::cout << "WRONG GUESS\n";
      backtrack();
      return 1;
    }
    update_solution(row, col, value_found+1);
    return 1;
  }
  return 0;
}

/* Solve row/col/block i using lone ranger
 * Return true if something changed
 */
bool loneranger(int i, Align align) {
  int row, col, ret = 0;

  // Data structure for lone rangers
  int num_found[board->dim], row_found[board->dim], col_found[board->dim];
  for(int num = 0; num < board->dim; num++) {
    num_found[num] = 0;
  }

  for(int j = 0; j < board->dim; j++) {
    index_to_row_col(i, j, align, row, col);
    if(!board->solution[row][col]) {
      for(int num = 0; num < board->dim; num++) {
        if(board->cells[row][col][num]) {
          num_found[num]++;
          row_found[num] = row;
          col_found[num] = col;
        }
      }
    }
  }
  for(int num = 0; num < board->dim; num++) {
    if(num_found[num] == 1) {
      ret = 1;
      update_solution(row_found[num], col_found[num], num+1);
    }
  }
  return ret;
}

bool twins(int i, Align align) {
  int ret = 0, row, col, row_found[2], col_found[2], num_found;

  for(int num1 = 0; num1 < board->dim; num1++) {
    for(int num2 = num1+1; num2 < board->dim; num2++) {
      num_found = 0;
      for(int j = 0; j < board->dim; j++) {
        index_to_row_col(i, j, align, row, col);
        if(!board->solution[row][col]) {
          if(board->cells[row][col][num1] && board->cells[row][col][num2]) {
            num_found++;
            if(num_found > 2) break;
            row_found[num_found-1] = row;
            col_found[num_found-1] = col;
          } else if(board->cells[row][col][num1] || board->cells[row][col][num2]) {
            num_found = 0;
            break;
          }
        }
      }
      if(num_found == 2) {
        for(int num = 0; num < board->dim; num++) {
          if(num != num1 && num != num2) {
            if(board->cells[row_found[0]][col_found[0]][num]) {
              board->cells[row_found[0]][col_found[0]][num] = 0;
              ret = 1;
            }
            if(board->cells[row_found[1]][col_found[1]][num]) {
              board->cells[row_found[1]][col_found[1]][num] = 0;
              ret = 1;
            }
          }
        }
      }
    }
  }
  if(ret)
    std::cout << "FOUND TWIN\n";
  return ret;
}

bool triplets(int i, Align align) {
  int ret = 0, row, col, row_found[3], col_found[3], num_found;

  // Only considered unsolved numbers
  bool unsolved[board->dim];
  for(int j = 0; j < board->dim; j++)
    unsolved[j] = 1;

  for(int j = 0; j < board->dim; j++) {
    index_to_row_col(i, j, align, row, col);
    if(board->solution[row][col]) {
      unsolved[board->solution[row][col]-1] = 0;
    }
  }

  for(int idx1 = 0; idx1 < board->dim; idx1++) {
    for(int idx2 = idx1+1; idx2 < board->dim; idx2++) {
      for(int idx3 = idx2+1; idx3 < board->dim; idx3++) {
        if(unsolved[idx1] && unsolved[idx2] && unsolved[idx3]) {
          num_found = 0;
          for(int j = 0; j < board->dim; j++) {
            index_to_row_col(i, j, align, row, col);
            if(!board->solution[row][col]) {
              if(board->cells[row][col][idx1] ||
                 board->cells[row][col][idx2] ||
                 board->cells[row][col][idx3]) {
                num_found++;
                if(num_found > 3) break;
                row_found[num_found-1] = row;
                col_found[num_found-1] = col;
              }
            }
          }
          if(num_found == 3) {
            for(int num = 0; num < board->dim; num++) {
              if(num != idx1 && num != idx2 && num != idx3) {
                if(board->cells[row_found[0]][col_found[0]][num]) {
                  board->cells[row_found[0]][col_found[0]][num] = 0;
                  ret = 1;
                }
                if(board->cells[row_found[1]][col_found[1]][num]) {
                  board->cells[row_found[1]][col_found[1]][num] = 0;
                  ret = 1;
                }
                if(board->cells[row_found[2]][col_found[2]][num]) {
                  board->cells[row_found[2]][col_found[2]][num] = 0;
                  ret = 1;
                }
              }
            }
          }
        }
      }
    }
  }
  if(ret) {
    std::cout << "FOUND TRIPLET\n";
  }
  return ret;
}

/********************************************************************/


void print_cells() {
  std::ostringstream stm;
  for(int r = 0; r < board->dim; r++) {
    for(int c = 0; c < board->dim; c++) {
      for(int num = 0; num < board->dim; num++) {
        stm << board->cells[r][c][num];
      }
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

void print_board(){
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
    stm << "CELLS SOLVED: " << board->cells_solved << "\n";
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

  int num;
  for(int row = 0; row < dim; row++) {
    for(int col = 0; col < dim; col++) {
      fscanf(file, "%d", &num);
      board->solution[row][col] = num;
      for(int index = 0; index < board->dim; index++) {
        board->cells[row][col][index] = 1;
      }
    }
  }

  // Reset cells based on numbers given
  int id = board->inner_dim;
  for(int row = 0; row < dim; row++) {
    for(int col = 0; col < dim; col++) {
      if(board->solution[row][col]) {
        update_solution(row, col, board->solution[row][col]);
      }
    }
  }
  return 1;
}

void update_solution(int row, int col, int num) {
  int id = board->inner_dim;
  board->solution[row][col] = num;
  board->cells_solved++;
  clear_number(row, ROW, num);
  clear_number(col, COL, num);
  clear_number((row/id)*id + (col/id), BLOCK, num);
}

void solve() {
  bool changed = 0;
  int total = board->dim * board->dim;
  while(board->cells_solved < total) {
    changed = 0;
    for(int thread_id = 0; thread_id < board->dim; thread_id++) {
      for(int j = 0; j < board->dim; j++) {
       changed |= elimination(thread_id, j, BLOCK);
      }
    }

    if(!changed) {
      for(int thread_id = 0; thread_id < board->dim; thread_id++) {
        changed |= loneranger(thread_id, ROW);
        changed |= loneranger(thread_id, COL);
        changed |= loneranger(thread_id, BLOCK);
      }
    }

    if(!changed) {
      for(int thread_id = 0; thread_id < board->dim; thread_id++) {
        changed |= twins(thread_id, ROW);
        changed |= twins(thread_id, COL);
        changed |= twins(thread_id, BLOCK);
      }
    }

    if(!changed) {
      for(int thread_id = 0; thread_id < board->dim; thread_id++) {
        changed |= triplets(thread_id, ROW);
        changed |= triplets(thread_id, COL);
        changed |= triplets(thread_id, BLOCK);
      }
    }

    if(!changed) {
      std::cout << "GUESSED\n";
      make_guess();
    }
  }
}

bool brute_force(int** input, int dim, int inner_dim, int row, int col){
	//std::cout<<col;
	if (col >= dim){
		row++;
		col = 0;
		if (row >= dim){
			return true;
		}
	}
	if (input[row][col] != 0){
		return brute_force(input, dim, inner_dim, row, col+1);
	}
  for (int num = 1; num <= dim; num++){
		if (check_move(input, dim, inner_dim, row, col, num)){
		  input[row][col] = num;
		  if (brute_force(input, dim, inner_dim, row, col+1)){
				return true;
		  }
		}
  }
	input[row][col] = 0;
	return false;
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

    print_board();
    print_cells();
    solve();

    std::cout<< "\nSOLVED BOARD: \n";
		print_board();

		//clock_t t = clock();
		//if (brute_force(input, Board.dim, Board.inner_dim, 0, 0)){
			//print_board(input, Board.dim, Board.inner_dim);
		//}
		//else{
			//std::cout << "Error: Sudoku board is unsolvable";
		//}
		//t = clock()-t;
		//std::cout << "\n\n";
		//std::cout << "Runtime: ";
		//std::cout << ((float)t)/CLOCKS_PER_SEC;
		//std::cout << " seconds\n";
	}
	else {
		std::cerr << "Error: No text file path for puzzle included.";
		return 0;
	}
}
