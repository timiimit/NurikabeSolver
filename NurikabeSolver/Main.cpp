#include "Nurikabe.h"
#include <iostream>
#include <chrono>

int main()
{
	Nurikabe::Board board;
	if (!board.Load("10x18-hard.txt"))
	{
		std::cout << "failed to read file" << std::endl;
		return 1;
	}
	board.Print(std::cout);

	Nurikabe::Solver solver(board);

	auto timeStart = std::chrono::system_clock::now();

	auto isSolved = solver.Solve();

	auto timeStop = std::chrono::system_clock::now();
	double timeElapsed = (timeStop - timeStart).count() / 1'000'000.0;

	if (!isSolved)
	{
		std::cout << "Failed to solve given puzzle" << std::endl << std::endl;
	}
	else
	{
		std::cout << std::endl << std::endl << std::endl;
		std::cout << "   --- Starting board ---   " << std::endl;
		board.Print(std::cout);
		std::cout << "   ---  Solved board  ---   " << std::endl;
		solver.GetBoard().Print(std::cout);
		std::cout << std::endl;
	}

	std::cout
		<< "Runtime: " << timeElapsed << "ms" << std::endl
		<< "Iterations: " << solver.GetIteration() << std::endl;

	return 0;
}
