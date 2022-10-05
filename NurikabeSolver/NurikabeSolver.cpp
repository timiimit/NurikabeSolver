#include "NurikabeSolver.h"
#include <iostream>

using namespace Nurikabe;

Solver::Solver(const Board& initialBoard)
	: board(initialBoard)
	, flags(initialBoard)
{

}

void Solver::SolveForcing()
{
	// if white square has a 1, mark neighbours black and final
	for (int y = 0; y < board.GetHeight(); y++)
	{
		for (int x = 0; x < board.GetWidth(); x++)
		{
			if (board.IsWhite(x, y) && board.GetRequiredSize(x, y) == 1)
			{
				board.SetBlack(x - 1, y);
				board.SetBlack(x + 1, y);
				board.SetBlack(x, y - 1);
				board.SetBlack(x, y + 1);

				flags.SetBlack(x - 1, y);
				flags.SetBlack(x + 1, y);
				flags.SetBlack(x, y - 1);
				flags.SetBlack(x, y + 1);
			}
		}
	}
}

bool Solver::Solve()
{
	for (int y = 0; y < board.GetHeight(); y++)
	{
		for (int x = 0; x < board.GetWidth(); x++)
		{
			if (board.GetRequiredSize(x, y) == 0)
			{
				// set all unnumbered squares to black
				board.SetBlack(x, y);

				// if this square neighbours more than one sized white blocks
				// it is 100% black
				int sizedNeighbours = 0;
				if (board.GetRequiredSize(x - 1, y) != 0) sizedNeighbours++;
				if (board.GetRequiredSize(x + 1, y) != 0) sizedNeighbours++;
				if (board.GetRequiredSize(x, y - 1) != 0) sizedNeighbours++;
				if (board.GetRequiredSize(x, y + 1) != 0) sizedNeighbours++;
				if (sizedNeighbours > 1)
				{
					flags.SetBlack(x, y);
				}
			}

			if (board.GetRequiredSize(x, y) == 1)
			{
				// mark neighbours of size=1 as final
				flags.SetBlack(x - 1, y);
				flags.SetBlack(x + 1, y);
				flags.SetBlack(x, y - 1);
				flags.SetBlack(x, y + 1);
			}

			if (board.GetRequiredSize(x, y) != 0)
			{
				// make sure we point to the originating white
				initialWhites.push_back({ x, y });
				flags.SetSize(x, y, initialWhites.size() - 1);
			}
		}
	}

	SolveForcing();
	board.Print(std::cout);
	return false;
}