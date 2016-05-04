#include "make_guess.h"

/* Choose cell with least possible values */
void choose_cell(int &row, int &col, int &value) {
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



void make_guess() {
  int row, col, guess;

  choose_cell(row, col, guess);
  State* new_state = new State();
  new_state->row = row;
  new_state->col = col;
  new_state->guess = guess;
  states.push(new_state);

  update_solution(row, col, guess);
}

void backtrack() {
  if(states.empty()) {
    std::cerr << "ERROR: Stack is empty\n";
    //return;
  }
  State* old_state = states.top();
  int new_guess = 0;
  int row = old_state->row, col = old_state->col, guess = old_state->guess;

  old_state->copy_board->cells[row][col][guess-1] = 0;

  // Find next value to guess
  for(int num = 0; num < board->dim; num++) {
    if(old_state->copy_board->cells[row][col][num])
      new_guess = num+1;
  }

  // No values left to guess, so backtrack once more
  if(!new_guess) {
    delete old_state;
    states.pop();
    backtrack();
    return;
  }
  // Copy contents of old_state back to board
  for(int r = 0; r < board->dim; r++) {
    for(int c = 0; c < board->dim; c++) {
      for(int num = 0; num < board->dim; num++) {
        board->cells[r][c][num] = old_state->copy_board->cells[r][c][num];
      }
      board->solution[r][c] = old_state->copy_board->solution[r][c];
    }
  }
  board->cells_solved = old_state->copy_board->cells_solved;

  old_state->guess = new_guess;

  update_solution(row, col, new_guess);

}
