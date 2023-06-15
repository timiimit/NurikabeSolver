#include "Nurikabe.h"
#include <iostream>
#include <chrono>
#include <assert.h>

int main(int argc, const char** argv)
{
    const char** files = argv + 1;
	int fileCount = argc - 1;

    for (int i = 0; i < fileCount; i++)
    {
		Nurikabe::Board board;
		if (!board.Load(files[i]))
		{
            std::cout << "Failed to read '" << files[i] << "'" << std::endl;
            return 1;
		}

		std::cout << "Solving '" << files[i] << "' ..." << std::endl;

		int iteration = 0;
		Nurikabe::Solver solver(board, &iteration);

		auto timeStart = std::chrono::system_clock::now();

		Nurikabe::Solver::SolveSettings settings;
		settings.maxDepth = 2;
		auto isSolved = solver.Solve(settings);

		auto timeStop = std::chrono::system_clock::now();
        double timeElapsed = (double)std::chrono::duration_cast<std::chrono::nanoseconds>(timeStop - timeStart).count();
		timeElapsed /= 1'000'000.0;

		if (!isSolved)
        {
			assert(0);
			std::cout << "Failed to solve:" << std::endl << std::endl;
			board.Print(std::cout);
		}
		else
		{
			std::cout << "Before and After:" << std::endl;

			const Nurikabe::Board* boards[] = { &board, &solver.GetBoard() };
			Nurikabe::Board::Print(boards, 2, std::cout);
			std::cout << std::endl;
		}

		std::cout
			<< "Runtime: " << timeElapsed << "ms" << std::endl
			<< "Iterations: " << iteration << std::endl;
    }

    std::cout << std::endl << "Finished solving." << std::endl;

    return 0;
}
