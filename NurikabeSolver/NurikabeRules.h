// NurikabeSolver.h : Include file for standard system include files,
// or project specific include files.

#pragma once
#include "NurikabeBoard.h"
#include "NurikabeRegion.h"
#include "Point.h"

namespace Nurikabe
{
	namespace Rules
	{
		bool ContainsBlack2x2(const Board& board);
		bool IsBlackContiguous(const Board& board);

		bool IsProperSize(const Board& board, const Point& pt);
		bool IsTouchingAnother(const Board& board, const Point& pt);

		bool IsSolved(const Board& board);

		std::vector<Point> FindConnectingSquares(const Board& board, const Point& pt);

		std::vector<Point> FindOverlappingConnectingSquares(const Board& board1, const Board& board2, const Point& pt);

		int FindDistance(const Point& start, const Point& end);
		bool CanReach(const Point& start, const Point& end, int maxDistance);
	};
}
