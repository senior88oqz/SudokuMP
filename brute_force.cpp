#include "brute_force.h"

Board* create_copy_board(Board* board){
	Board* copy_board = new Board(board->dim);
	for(int row = 0; row < board->dim; row++) {
		for(int col = 0; col < board->dim; col++) {
			copy_board->solution[row][col] = board->solution[row][col];
			for(int index = 0; index < board->dim; index++) {
				copy_board->cells[row][col][index] = 1;
			}
		}
	}

	for(int row = 0; row < copy_board->dim; row++) {
		for(int col = 0; col < copy_board->dim; col++) {
			if(copy_board->solution[row][col]) {
				update_solution(row, col, copy_board->solution[row][col]);
				copy_board->cells_solved++;
			}
		}
	}

	return copy_board;
}

void choose_cell_bf(Board* board, int &row, int &col, int &value) {
  int least = board->dim+1, counter, guess_value;
  for(int r = 0; r < board->dim; r++) {
    for(int c = 0; c < board->dim; c++) {
      counter = 0;
      if(!board->solution[r][c]) {
        for(int num = 0; num < board->dim; num++) {
          if(board->cells[r][c][num]) {
            counter++;
            guess_value = num+1;
          }
        }
        if(counter < least) {
          row = r;
          col = c;
          value = guess_value;
          if(counter == 2) return;
        }
      }
    }
  }
}

void update_solution_bf(Board* board, int row, int col, int num) {

  int id = board->inner_dim, success;
  success = __sync_bool_compare_and_swap(&board->solution[row][col], 0, num);

  if(success) {
    #pragma omp atomic update
    board->cells_solved++;
  }
  clear_number(row, ROW, num);
  clear_number(col, COL, num);
  clear_number((row/id)*id + (col/id), BLOCK, num);
}

/* Get next empty cell on board, create copy 
 * of board for each valid value for that cell,
 * then push copy of board to stack
 */
void update_stack(Board* board){
	int row, col, value, counter = 0;
	choose_cell_bf(board, row, col, value);

	for (int num = 0; num < board->dim; num++){
		Board* copy_board = create_copy_board(board);
		if (copy_board->cells[row][col][num]){
			counter++;
			update_solution_bf(copy_board, row, col, counter);
			boards.push(copy_board);
		}
	}
}


void parallel_brute_force(Board* board, int total){
	// Create copy of current board
	// Find valid values for first empty cell
	// Create separate boards for each value in that cell
	// Push all of those copies onto boards stack
	// Threads will pop from stack in parallel
	// Repeat copy through pop
	Board *next_board = NULL;
	if (board->cells_solved == total){
		return;
	}
	//#pragma omp task firstprivate(board, total)
	{
		update_stack(board);
		if (boards.empty()){
			std::cerr << "ERROR: Stack is empty\n";
		}
		else{
			next_board = boards.top();
		}	
		
		if (next_board != NULL){
			boards.pop();
			parallel_brute_force(next_board, total);
		}
	}
	
}