#include "Nurikabe.h"
#include <iostream>
#include <chrono>
#include <assert.h>

int main()
{
	const char* files[] =
	{
		"5x5-easy.txt",
		"7x7-hard.txt",
		"10x10-1.txt",
		"10x10-2.txt",
		"10x10-2-2.txt",
		"10x10-3.txt",
		"10x18-3.txt",
		"10x18-4.txt",
		"10x18-hard.txt",
		"10x18-medium.txt",
		"16x30-1.txt",
	};

	for (int i = 0; i < sizeof(files) / sizeof(*files); i++)
	{
		Nurikabe::Board board;
		if (!board.Load(files[i]))
		{
			std::cout << "Failed to read file" << std::endl;
			return 1;
		}

		std::cout << "   --- Starting board ---   " << std::endl;
		board.Print(std::cout);

		int iteration = 0;
		Nurikabe::Solver solver(board, &iteration);

		auto timeStart = std::chrono::system_clock::now();

		Nurikabe::Solver::SolveSettings settings;
		auto isSolved = solver.Solve(settings);

		auto timeStop = std::chrono::system_clock::now();
		double timeElapsed = (timeStop - timeStart).count() / 1'000'000.0;

		if (!isSolved)
		{
			assert(0);
			std::cout << "Failed to solve given puzzle" << std::endl << std::endl;
		}
		else
		{
			//std::cout << std::endl << std::endl << std::endl;
			// std::cout << "   --- Starting board ---   " << std::endl;
			// board.Print(std::cout);
			std::cout << "   ---  Solved board  ---   " << std::endl;
			solver.GetBoard().Print(std::cout);
			std::cout << std::endl;
		}

		std::cout
			<< "Runtime: " << timeElapsed << "ms" << std::endl
			<< "Iterations: " << iteration << std::endl;
	}

	return 0;
}
