#include <stdlib.h>

struct board
{
  int dim;

  // cells = dim x dim array of ints, each: is a bit vector of possible values
  // i.e. 010000000 = 8 is the value of the cell
  // i.e. 010011010 = 2, 4, 5, 8 are possible values of cell
  int** cells;




};
