#include "NurikabeRules.h"
#include "Point.h"
#include <vector>
#include <queue>

using namespace Nurikabe;

bool Rules::IsBlackContiguous(const Board& board)
{
	// find any black
	Point pt;
	for (pt.y = 0; pt.y < board.GetHeight(); pt.y++)
	{
		for (pt.x = 0; pt.x < board.GetWidth(); pt.x++)
		{
			if (board.IsBlack(pt))
				break;
		}

		if (board.IsBlack(pt))
			break;
	}

	if (pt.x == board.GetWidth() && pt.y == board.GetHeight())
	{
		// there is no black square which is technically
		// probably satisfies our condition.
		// if we have a 2x1 board and one square has 2
		// as white size, then the board is solved even
		// without any black squares
		return true;
	}

	// search for all connecting blacks
	std::vector<Point> all = FindConnectingSquares(board, pt);

	// look through remainder of squares and check
	// if there exists a black that is not in "all"
	for (; pt.y < board.GetHeight(); pt.y++)
	{
		for (; pt.x < board.GetWidth(); pt.x++)
		{
			if (board.IsBlack(pt))
			{
				if (std::find(all.begin(), all.end(), pt) == all.end())
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

	auto count = FindConnectingSquares(board, pt).size();
	return count == required;
}

bool Rules::IsTouchingAnother(const Board& board, const Point& pt)
{
	auto touching = FindConnectingSquares(board, pt);
	int count = 0;
	for (int i = 0; i < touching.size(); i++)
	{
		if (board.GetRequiredSize(touching[i]) > 0)
		{
			count++;
			if (count > 1)
				return true;
		}
	}
	return false;
}

bool Rules::IsSolved(const Board& board)
{
	if (!IsBlackContiguous(board))
		return false;

	if (ContainsBlack2x2(board))
		return false;

	for (int y = 0; y < board.GetHeight(); y++)
	{
		for (int x = 0; x < board.GetWidth(); x++)
		{
			Point pt = { x, y };
			if (board.IsWhite(pt) && board.GetRequiredSize(pt) > 0)
			{
				if (!IsProperSize(board, pt))
					return false;

				if (IsTouchingAnother(board, pt))
					return false;
			}
		}
	}

	return true;
}

std::vector<Point> Rules::FindConnectingSquares(const Board& board, const Point& pt)
{
	std::vector<Point> all;
	std::vector<Point> border;
	border.push_back(pt);

	bool isBlack = board.IsBlack(pt);
	auto AddIfNewAndValid = [&board, &all, &border, isBlack](const Point& pt)
	{
		if (board.IsValidPosition(pt) && board.IsBlack(pt) == isBlack)
		{
			auto itAll = std::find(all.begin(), all.end(), pt);
			auto itBorder = std::find(border.begin(), border.end(), pt);
			if (itAll == all.end() && itBorder == border.end())
			{
				border.push_back(pt);
			}
		}
	};

	while (border.size())
	{
		Point current = border.back();
		border.pop_back();

		all.push_back(current);

		for (int i = 0; i < all.size(); i++)
		{
			AddIfNewAndValid(current.Left());
			AddIfNewAndValid(current.Right());
			AddIfNewAndValid(current.Up());
			AddIfNewAndValid(current.Down());
		}
	}
	return all;
}

std::vector<Point> Nurikabe::Rules::FindOverlappingConnectingSquares(const Board& board1, const Board& board2, const Point& pt)
{
	std::vector<Point> all;
	std::vector<Point> border;
	border.push_back(pt);

	bool isBlack1 = board1.IsBlack(pt);
	bool isBlack2 = board2.IsBlack(pt);
	auto AddIfNewAndValid = [&board1, &board2, &all, &border, isBlack1, isBlack2](const Point& pt)
	{
		if (board1.IsValidPosition(pt) && board1.IsBlack(pt) == isBlack1 &&
			board2.IsValidPosition(pt) && board2.IsBlack(pt) == isBlack2)
		{
			auto itAll = std::find(all.begin(), all.end(), pt);
			auto itBorder = std::find(border.begin(), border.end(), pt);
			if (itAll == all.end() && itBorder == border.end())
			{
				border.push_back(pt);
			}
		}
	};

	while (border.size())
	{
		Point current = border.back();
		border.pop_back();

		all.push_back(current);

		for (int i = 0; i < all.size(); i++)
		{
			AddIfNewAndValid(current.Left());
			AddIfNewAndValid(current.Right());
			AddIfNewAndValid(current.Up());
			AddIfNewAndValid(current.Down());
		}
	}
	return all;
}

int Rules::FindDistance(const Board& board, const Point& start, const Point& end)
{
	int dist = (int)abs(end.x - start.x);
	dist += (int)abs(end.y - start.y);
	return dist + 1;
}

bool Rules::CanReach(const Board& board, const Point& start, const Point& end, int maxDistance)
{
	return FindDistance(board, start, end) <= maxDistance;
}