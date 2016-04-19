#include "board.h"
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <math.h>
#include <string>
#include <iostream>



void print_board(int **input, int dim, int inner_dim){
    std::string line = "";
    for (int i = 0; i < inner_dim; i++){
        for (int j = 0; j < dim; j++){
            for (int k = 0; k < inner_dim; k++){
                line += (" " + std::to_string(input[k*i][j]) + " ");
            }
            line += " ";
            if ((j+1) % inner_dim == 0){
                line += "\n";
            }
        }
        line += "\n";
    }
    std::cout << line;
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
		FILE *file = fopen(rp, "r");
		if (!file) return 0;

		for (int row = 0; row < Board.dim; row++){
			for (int col = 0; col < Board.dim; col++){
				fscanf(file, "%d", &input[row][col]);
			}
		}
		print_board(input, Board.dim, Board.inner_dim);
	}
	else {
		std::cerr << "Error: No text file path for puzzle included.";
		return 0;
	}
}