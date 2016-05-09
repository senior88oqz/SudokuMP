#include "board.h"
#include <stack>

struct State
{
  State() {
    copy_board = new Board(board->dim);
    for(int r = 0; r < board->dim; r++) {
      for(int c = 0; c < board->dim; c++) {
        copy_board->solution[r][c] = board->solution[r][c];
        for(int num = 0; num < board->dim; num++) {
          copy_board->cells[r][c][num] = board->cells[r][c][num];
        }
      }
    }
    copy_board->cells_solved = board->cells_solved;
  }

  ~State() {
    delete copy_board;
  }

  // copy_board = a copy of the state of the board before guessing
  Board* copy_board;

  // row = the row of the guess
  int row;

  // col = the column of the guess
  int col;

  // guess = value of the guess, NOT THE INDEX
  int guess;

};

/* Global stack for saving states before guessing */
extern std::stack<State*> states;


/* Choose the cell with the least possible values left to guess
 * Save the current board state in the stack
 * Make the guess
 */
void make_guess();

/* Pop a state from the stack
 * Copy contents from the saved board into the current board
 * Choose another guess, update the state, and push onto the stack
 * Make the guess
 * If no guesses are left, backtrack again
 */
void backtrack();
