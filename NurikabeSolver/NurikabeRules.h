// NurikabeSolver.h : Include file for standard system include files,
// or project specific include files.

#pragma once
#include "NurikabeBoard.h"
#include <vector>
#include "Point.h"

namespace Nurikabe
{
	namespace Rules
	{
		bool ContainsBlack2x2(const Board& board);
		bool IsBlackContiguous(const Board& board);

		bool IsProperSize(const Board& board, int x, int y);
		bool IsTouchingAnother(const Board& board, int x, int y);

		bool IsSolved(const Board& board);

		std::vector<Point> FindConnectingSquares(const Board& board, int x, int y);

		//bool CanChangeColor(const Board& board, int x, int y);
	};
}
