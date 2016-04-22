#include "board.h"


void print_board(int **input, int dim, int inner_dim){
	for (int row = 0; row < dim; row++){
		for (int col = 0; col < dim; col++){
			if (col % inner_dim == inner_dim-1 && col != dim-1){
				std::cout << input[row][col] << "  ";
			}
			else{
				std::cout << input[row][col] << " ";
			}
		}
		if (row % inner_dim == inner_dim-1 && row != dim-1){
			std::cout << std::endl;
			std::cout << std::endl;
		}
		else{
			std::cout << std::endl;
		}
	}
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

bool brute_force(int **input, int dim, int inner_dim, int row, int col){
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
	else{
		for (int num = 1; num <= dim; num++){
			if (check_move(input, dim, inner_dim, row, col, num)){
				input[row][col] = num;
				if (brute_force(input, dim, inner_dim, row, col+1)){
					return true;
				}
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
			std::cerr << "Error: Path to text file for puzzle is not valid.";
			return 0;
		}

		Board.dim = atoi(argv[2]);
		if ((int)pow((float)sqrt(Board.dim), 2.0) != Board.dim) {
			std::cerr << "Error: Dimension is not a perfect square.";
			return 0;
		}
		else{
			Board.inner_dim = (int)sqrt(Board.dim);
		}
		int **input;

		input = (int **)calloc(Board.dim, sizeof(int*));
		for (int i = 0; i < Board.dim; i++){
			input[i] = (int *)calloc(Board.dim, sizeof(int));
		}

		FILE *file = fopen(rp, "r");
		if (!file) return 0;

		for (int row = 0; row < Board.dim; row++){
			for (int col = 0; col < Board.dim; col++){
				fscanf(file, "%d", &input[row][col]);
			}
		}

		print_board(input, Board.dim, Board.inner_dim);
		std::cout << "\n-----------------------------------\n";
		clock_t t = clock();
		if (brute_force(input, Board.dim, Board.inner_dim, 0, 0)){
			print_board(input, Board.dim, Board.inner_dim);
		}
		else{
			std::cout << "Error: Sudoku board is unsolvable";
		}
		t = clock()-t;
		std::cout << "\n\n";
		std::cout << "Runtime: ";
		std::cout << ((float)t)/CLOCKS_PER_SEC;
		std::cout << " seconds\n";
	}
	else {
		std::cerr << "Error: No text file path for puzzle included.";
		return 0;
	}
}