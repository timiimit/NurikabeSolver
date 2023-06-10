#include "Nurikabe.h"
#include <iostream>

int main()
{
	Nurikabe::Board board;
	if (!board.Load("7x7-hard.txt"))
	{
		printf("failed to read file.\n");
		return 1;
	}
	board.Print(std::cout);

	Nurikabe::Solver solver(board);
	if (!Nurikabe::Solver::Solve(solver))
	{
		printf("failed to solve given puzzle.\n\n");
	}

	//board.Print(std::cout);

	return 0;
}
