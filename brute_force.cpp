#include "brute_force.h"
#include <omp.h>

omp_lock_t stack_lock;
bool solution_found;

Board* create_copy_board(Board* b){
  Board* copy_board = new Board(b->dim);
	for(int row = 0; row < b->dim; row++) {
		for(int col = 0; col < b->dim; col++) {
			copy_board->solution[row][col] = b->solution[row][col];
			for(int index = 0; index < b->dim; index++) {
				copy_board->cells[row][col][index] = b->cells[row][col][index];
			}
		}
	}
  copy_board->cells_solved = b->cells_solved;
	return copy_board;
}

/* Returns false if it found a cell with no remaining values */
bool choose_cell_bf(Board* b, int &row, int &col) {
  int least = b->dim+1, counter;
  for(int r = 0; r < b->dim; r++) {
    for(int c = 0; c < b->dim; c++) {
      counter = 0;
      if(!b->solution[r][c]) {
        for(int num = 0; num < b->dim; num++) {
          if(b->cells[r][c][num]) {
            counter++;
          }
        }
        if(counter < least) {
          least = counter;
          row = r;
          col = c;
        }
      }
    }
  }
  return (least != 0);
}

void clear_number_bf(Board* board, int i, Align align, int value) {
  int index = value-1;
  if(align == ROW) {
    for(int c = 0; c < board->dim; c++)
    {
      #pragma omp atomic write
      board->cells[i][c][index] = 0;
    }

  } else if (align == COL) {
    for(int r = 0; r < board->dim; r++)
    {
      #pragma omp atomic write
      board->cells[r][i][index] = 0;
    }

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

void update_solution_bf(Board* board, int row, int col, int num) {
  int id = board->inner_dim;
  int success = __sync_bool_compare_and_swap(&board->solution[row][col], 0, num);
  if(success) {
    #pragma omp atomic update
    board->cells_solved++;
    clear_number_bf(board, row, ROW, num);
    clear_number_bf(board, col, COL, num);
    clear_number_bf(board, (row/id)*id + (col/id), BLOCK, num);
  }
}

/* Get next empty cell on board, create copy
 * of board for each valid value for that cell,
 * then push copy of board to stack
 */
void update_stack(Board* b){
	int row, col;
  Board* copy_board;

  if(choose_cell_bf(b, row, col)) {
    for (int num = 0; num < b->dim; num++){
      if (b->cells[row][col][num]){
        copy_board = create_copy_board(b);
			  update_solution_bf(copy_board, row, col, num+1);
        omp_set_lock(&stack_lock);
        boards.push(copy_board);
        omp_unset_lock(&stack_lock);
      }
	  }
  }
}

void parallel_brute_force() {
  Board* curr_board;
  int total = board->dim * board->dim;
  boards.push(board);
  solution_found = 0;

  #pragma omp parallel private(curr_board)
  {
    while(!solution_found) {
      curr_board = NULL;

      omp_set_lock(&stack_lock);
      if(!boards.empty()) {
        curr_board = boards.top();
        boards.pop();
      }
      omp_unset_lock(&stack_lock);

      if(curr_board) {
        if(curr_board->cells_solved < total) {
          update_stack(curr_board);
          delete curr_board;
        } else {
          std::cout << "BRUTE FORCE SOLVED\n";
          solution_found = 1;
          board = curr_board;
          break;
        }
      } else {
        //std::cerr << "ERROR: Brute force stack is empty\n";
        //break;
      }
    }
  }
}
