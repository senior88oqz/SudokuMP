#include "make_guess.h"
#include "CycleTimer.h"
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <string>
#include <iostream>
#include <sstream>
#include <omp.h>


enum Align {ROW, COL, BLOCK};

Board* board;
std::stack<State*> states;

// Lock when changing solution
omp_lock_t read_lock;
omp_lock_t write_lock;
int readers = 0;


/************ Helper functions **************************************/

/* R/W lock functions */
void reader_lock() {
  omp_set_lock(&read_lock);
  readers++;
  if(readers == 1) omp_set_lock(&write_lock);
  omp_unset_lock(&read_lock);
}

void reader_unlock() {
  omp_set_lock(&read_lock);
  readers--;
  if(readers == 0) omp_unset_lock(&write_lock);
  omp_unset_lock(&read_lock);
}

void writer_lock() {
  omp_set_lock(&write_lock);
}

void writer_unlock() {
  omp_unset_lock(&write_lock);
}



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
      #pragma omp atomic write
      board->cells[i][c][index] = 0;
  } else if (align == COL) {
    for(int r = 0; r < board->dim; r++)
      #pragma omp atomic write
      board->cells[r][i][index] = 0;
  } else {
    int id = board->inner_dim;
    for(int r = (i/id) * id; r < (i/id)*id + id; r++) {
      for(int c = (i%id) * id; c < (i%id)*id + id; c++) {
        #pragma omp atomic write
        board->cells[r][c][index] = 0;
      }
    }
  }
}


/* Test is elimination is possible (i.e. 010000000 -> 8)
 * If possible, update solution and remove num from row, col, and block
 */
bool elimination(int i, int j, Align align, bool &need_backtrack) {
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

    if(!found && !board->solution[row][col]) {
      #pragma omp atomic write
      need_backtrack = 1;
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
  int row, col, ret = 0, row_found, col_found, num_found;

  for(int num = 0; num < board->dim; num++) {
    num_found = 0;
    reader_lock();
    for(int j = 0; j < board->dim; j++) {
      index_to_row_col(i, j, align, row, col);
      if(!board->solution[row][col] && board->cells[row][col][num]) {
        num_found++;
        if(num_found > 1) break;
        row_found = row;
        col_found = col;
      }
    }
    reader_unlock();
    if(num_found == 1) {
      ret = 1;
      update_solution(row_found, col_found, num+1);
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
  for(int row = 0; row < dim; row++) {
    for(int col = 0; col < dim; col++) {
      if(board->solution[row][col]) {
        update_solution(row, col, board->solution[row][col]);
        board->cells_solved++;
      }
    }
  }
  return 1;
}

void update_solution(int row, int col, int num) {

  int id = board->inner_dim, success;
  writer_lock();
  success = __sync_bool_compare_and_swap(&board->solution[row][col], 0, num);
  writer_unlock();

  if(success) {
    #pragma omp atomic update
    board->cells_solved++;
  }
  clear_number(row, ROW, num);
  clear_number(col, COL, num);
  clear_number((row/id)*id + (col/id), BLOCK, num);
}

void solve() {
  bool changed, need_backtrack;
  int total = board->dim * board->dim;
  omp_set_num_threads(board->dim);
  while(board->cells_solved < total) {
    changed = 0;
    need_backtrack = 0;
    #pragma omp parallel for
    for(int thread_id = 0; thread_id < board->dim; thread_id++) {
      for(int j = 0; j < board->dim; j++) {
        #pragma omp atomic update
        changed |= elimination(thread_id, j, BLOCK, need_backtrack);
      }
    }
    if(need_backtrack) {
      backtrack();
      changed = 1;
    }

    if(!changed) {
      #pragma omp parallel for
      for(int thread_id = 0; thread_id < board->dim; thread_id++) {
        #pragma omp atomic update
        changed |= loneranger(thread_id, ROW);
        #pragma omp atomic update
        changed |= loneranger(thread_id, COL);
        #pragma omp atomic update
        changed |= loneranger(thread_id, BLOCK);
      }
    }
    print_board();
    print_cells();

    if(!changed) {
      #pragma omp parallel for
      for(int thread_id = 0; thread_id < board->dim; thread_id++) {
        #pragma omp atomic update
        changed |= twins(thread_id, ROW);
        #pragma omp atomic update
        changed |= twins(thread_id, COL);
        #pragma omp atomic update
        changed |= twins(thread_id, BLOCK);
      }
    }

    print_board();
    print_cells();

    if(!changed) {
      #pragma omp parallel for
      for(int thread_id = 0; thread_id < board->dim; thread_id++) {
        #pragma omp atomic update
        changed |= triplets(thread_id, ROW);
        #pragma omp atomic update
        changed |= triplets(thread_id, COL);
        #pragma omp atomic update
        changed |= triplets(thread_id, BLOCK);
      }
    }

    if(!changed) {
      make_guess();
    }
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

    print_board();
    //print_cells();

    double start = CycleTimer::currentSeconds();
    solve();
    double time = CycleTimer::currentSeconds() - start;

    std::cout<< "\nSOLVED BOARD: \n";
		print_board();
    std::cout << "Time elapsed: " << time << " secs\n";
	}
	else {
		std::cerr << "Error: No text file path for puzzle included.";
		return 0;
	}
}
