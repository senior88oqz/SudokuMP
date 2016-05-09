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

/* Choose cell with the least number of possible values
 * Return 0 if that cell has 0 values
 */
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
			  update_solution(copy_board, row, col, num+1);
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
      } else if(NUM_THREADS == 1) {
          std::cerr << "ERROR: Brute force stack is empty\n";
          break;
      }
    }
  }
}
