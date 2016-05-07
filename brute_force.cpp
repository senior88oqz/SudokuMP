#include "brute_force.h"
#include <omp.h>

Board* create_copy_board(Board* board){
	Board* copy_board = new Board(board->dim);
	for(int row = 0; row < board->dim; row++) {
		for(int col = 0; col < board->dim; col++) {
			copy_board->solution[row][col] = board->solution[row][col];
			for(int index = 0; index < board->dim; index++) {
				copy_board->cells[row][col][index] = board->cells[row][col][index];
			}
		}
	}
  copy_board->cells_solved = board->cells_solved;

	return copy_board;
}

void choose_cell_bf(Board* board, int &row, int &col) {
  int least = board->dim+1, counter;
  for(int r = 0; r < board->dim; r++) {
    for(int c = 0; c < board->dim; c++) {
      counter = 0;
      if(!board->solution[r][c]) {
        for(int num = 0; num < board->dim; num++) {
          if(board->cells[r][c][num]) {
            counter++;
          }
        }
        if(counter < least) {
          row = r;
          col = c;
          if(counter == 1) return;
        }
      }
    }
  }
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
  }

  clear_number_bf(board, row, ROW, num);
  clear_number_bf(board, col, COL, num);
  clear_number_bf(board, (row/id)*id + (col/id), BLOCK, num);
}

/* Get next empty cell on board, create copy
 * of board for each valid value for that cell,
 * then push copy of board to stack
 */
void update_stack(Board* board){
	int row, col;
	choose_cell_bf(board, row, col);

	for (int num = 0; num < board->dim; num++){
		Board* copy_board = create_copy_board(board);
		//std::cout << copy_board->cells_solved << "\n";
		if (copy_board->cells[row][col][num]){
			update_solution_bf(copy_board, row, col, num+1);
			boards.push(copy_board);
		}
	}
}


void parallel_brute_force(Board* bf_board, int total){
	// Create copy of current board
	// Find valid values for first empty cell
	// Create separate boards for each value in that cell
	// Push all of those copies onto boards stack
	// Threads will pop from stack in parallel
	// Repeat copy through pop

  Board *next_board = NULL;
	if (bf_board->cells_solved == total){

    board = bf_board;
    
    //print_board(board);
    std::cout << "SOLVED\n";
		return;
	}

  #pragma omp parallel firstprivate(bf_board, total)
  {
    
    update_stack(bf_board);
    if (boards.empty()){
    }
    else{
      #pragma omp critical
      next_board = boards.top();
    }

    if (next_board != NULL){
      #pragma omp critical
      boards.pop();
      parallel_brute_force(next_board, total);
    }
  }


}
