#include "NurikabeRules.h"
#include "Point.h"
#include <vector>
#include <queue>
#include <assert.h>

using namespace Nurikabe;

bool Rules::FindAnySquareOfState(const Board& board, SquareState state, Point& out)
{
	for (out.y = 0; out.y < board.GetHeight(); out.y++)
	{
		for (out.x = 0; out.x < board.GetWidth(); out.x++)
		{
			if (board.Get(out).GetState() == state)
				return true;
		}

		if (board.Get(out).GetState() == state)
			return true;
	}
	
	return false;
}

bool Rules::IsBlackContiguous(const Board& board)
{
	Point pt;

	if (!FindAnySquareOfState(board, SquareState::Black, pt))
	{
		// there is no black square which technically
		// probably satisfies our condition.
		//
		// if we have a 2x1 board and one square has 2
		// as white size, then the board is solved even
		// without any black squares
		return true;
	}

	// search for all connecting blacks
	Region all = Region((Board*)&board, pt);
	SquareState state = all.GetState();
	all.ExpandAllInline([state](const Point&, const Square& sq) {
		return sq.GetState() == state;
	});

	// look through remainder of squares and check
	// if there exists a black that is not in "all"
	for (; pt.y < board.GetHeight(); pt.y++)
	{
		for (; pt.x < board.GetWidth(); pt.x++)
		{
			if (board.IsBlack(pt))
			{
				if (std::find(all.GetSquares().begin(), all.GetSquares().end(), pt) == all.GetSquares().end())
					return false;
			}
		}
	}
	return true;
}

bool Rules::ContainsBlack2x2(const Board& board)
{
	for (int y = 1; y < board.GetHeight(); y++)
	{
		for (int x = 1; x < board.GetWidth(); x++)
		{
			Point pt = { x, y };
			if (board.IsBlack(pt) &&
				board.IsBlack(pt.Left()) &&
				board.IsBlack(pt.Up()) &&
				board.IsBlack(pt.Up().Left()))
			{
				return true;
			}
		}
	}
	return false;
}

bool Rules::IsProperSize(const Board& board, const Point& pt)
{
	auto required = board.GetRequiredSize(pt);
	if (required == 0)
		return true;

	Region r = Region((Board*)&board, pt);
	SquareState state = r.GetState();
	r.ExpandAllInline([state](const Point&, const Square& sq) {
		return sq.GetState() == state;
	});

	return r.GetSquareCount() == required;
}

bool Rules::IsTouchingAnother(const Board& board, const Point& pt)
{
	Region r = Region((Board*)&board, pt);

	if (r.GetState() != SquareState::White)
		return false;
	
	r.ExpandAllInline([](const Point&, const Square& sq) {
		return sq.GetState() == SquareState::White;
	});

	return !r.IsSameOrigin();
}

bool Rules::IsSolved(const Board& board)
{
	if (!IsBlackContiguous(board))
		return false;

	if (ContainsBlack2x2(board))
		return false;

	bool ret = true;

	board.ForEachSquare([&board, &ret](const Point& pt, const Square& sq)
	{
		if (board.IsWhite(pt) && board.GetRequiredSize(pt) > 0)
		{
			if (!IsProperSize(board, pt))
			{
				ret = false;
				return false;
			}

			if (IsTouchingAnother(board, pt))
			{
				ret = false;
				return false;
			}
		}
		return true;
	});

	return ret;
}

bool Rules::IsSolvable(const Board& board)
{
	if (ContainsBlack2x2(board))
		return false;

	Point pt;
	if (!FindAnySquareOfState(board, SquareState::Black, pt))
	{
		if (!FindAnySquareOfState(board, SquareState::Unknown, pt))
		{
			// TODO: handle edge case where there are only white squares
			assert(0);
		}
	}
	auto squares = Region((Board*)&board, pt)
		.ExpandAllInline([](const Point&, const Square& sq) {
			return sq.GetState() == SquareState::Black || sq.GetState() == SquareState::Unknown;
		})
		.GetSquares();
	
	bool ret = true;
	board.ForEachSquare([&squares, &ret, &board](const Point& pt, const Square& sq)
	{
		if (sq.GetState() == SquareState::Black && std::find(squares.begin(), squares.end(), pt) == squares.end())
		{
			ret = false;
			return false;
		}

		if (sq.GetState() == SquareState::White && sq.GetOrigin() == (uint8_t)~0)
		{
			Region unconnectedWhite = Region((Board*)&board, pt)
				.ExpandAllInline([](const Point&, const Square& sqInner) {
					return sqInner.GetState() == SquareState::White && sqInner.GetOrigin() == (uint8_t)~0;
				});

			Region pathToConnectWhite = Region(unconnectedWhite)
				.ExpandAllInline([](const Point&, const Square& sqInner) {
					return sqInner.GetState() == SquareState::Unknown;
				});

			Region reachableOriginTouchingWhites = pathToConnectWhite
				.Neighbours([](const Point&, const Square& sqInner) {
					return sqInner.GetState() == SquareState::White && sqInner.GetOrigin() != (uint8_t)~0;
				});

			if (reachableOriginTouchingWhites.GetSquareCount() == 0)
			{
				ret = false;
				return false;
			}

			reachableOriginTouchingWhites.ForEach([&unconnectedWhite, &pathToConnectWhite](const Point& pt, const Square& sq) {
				if (sq.GetSize() <= unconnectedWhite.GetSquareCount())
				{
					pathToConnectWhite = Region::Subtract(pathToConnectWhite, Region(pathToConnectWhite.GetBoard(), pt).Neighbours());
				}
				return true;
			});

			reachableOriginTouchingWhites = pathToConnectWhite
				.Neighbours([](const Point&, const Square& sqInner) {
					return sqInner.GetState() == SquareState::White && sqInner.GetOrigin() != (uint8_t)~0;
				});

			if (reachableOriginTouchingWhites.GetSquareCount() == 0)
			{
				ret = false;
				return false;
			}

			int maxSize = 0;
			reachableOriginTouchingWhites.ForEach([&maxSize](const Point& pt, const Square& sq)
			{
				int size = sq.GetSize();
				if (size > maxSize)
					maxSize = size;
				
				return true;
			});

			if (unconnectedWhite.GetSquareCount() > maxSize)
			{
				ret = false;
				return false;
			}
		}

		return true;
	});

	return ret;
}

int Rules::FindDistance(const Point& start, const Point& end)
{
	int dist = (int)abs(end.x - start.x);
	dist += (int)abs(end.y - start.y);
	return dist;
}

bool Rules::CanReach(const Point& start, const Point& end, int maxDistance)
{
	return FindDistance(start, end) <= maxDistance;
}