#include "Nurikabe.h"
#include <iostream>
#include <chrono>
#include <assert.h>
#include <cstring>

int main(int argc, const char** argv)
{
	Nurikabe::Solver::SolveSettings settings;
	settings.maxDepth = 2;

	std::vector<const char*> filenames;

	bool isFilename = false;
	for (int i = 1; i < argc; i++)
	{
		if (isFilename)
		{
			filenames.push_back(argv[i]);
			continue;
		}

		if (!std::strcmp(argv[i], "-i"))
		{
			i++;
			sscanf(argv[i], "%d", &settings.stopAtIteration);
		}

		if (!std::strcmp(argv[i], "-f"))
		{
			isFilename = true;
			continue;
		}
	}

	if (filenames.size() == 0)
	{
		std::cout << "Usage: NurikabeSolver [-i <iteration_to_stop_at>] -f <filename1> [filename2] [filename3] ..." << std::endl;
		return 0;
	}

	int failCount = 0;

	for (int i = 0; i < filenames.size(); i++)
	{
		Nurikabe::Board board;
		if (!board.Load(filenames[i]))
		{
			std::cout << "Failed to read '" << filenames[i] << "'" << std::endl;
			return 1;
		}

		std::cout << "Solving '" << filenames[i] << "' ..." << std::endl;

		int iteration = 0;
		Nurikabe::Solver solver(board, &iteration);

		auto timeStart = std::chrono::system_clock::now();

		auto isSolved = solver.Solve(settings);

		auto timeStop = std::chrono::system_clock::now();
		double timeElapsed = (double)std::chrono::duration_cast<std::chrono::nanoseconds>(timeStop - timeStart).count();
		timeElapsed /= 1'000'000.0;

		if (!isSolved)
		{
			std::cout << "Failed to solve:" << std::endl << std::endl;
			board.Print(std::cout);
			failCount++;
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

	return failCount;
}
