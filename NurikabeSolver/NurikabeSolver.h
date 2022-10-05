#pragma once
#include "NurikabeRules.h"
#include "NurikabeBoard.h"
#include <vector>

namespace Nurikabe
{
	class Solver
	{
		// current state of the board we are trying to solve
		Board board;

		// flags of current state of the board where isBlack means isFinal
		// and size means which startingWhite current square is a part of
		Board flags;

		std::vector<Point> initialWhites;


	public:
		Solver(const Board& initialBoard);

	private:
		void SolveForcing();

	public:
		bool Solve();
	};
}