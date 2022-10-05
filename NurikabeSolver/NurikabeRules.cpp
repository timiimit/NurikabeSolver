#include "NurikabeRules.h"
#include "Point.h"
#include <vector>
#include <queue>

using namespace Nurikabe;

bool Rules::IsBlackContiguous(const Board& board)
{
	// find any black
	int x;
	int y;
	for (y = 0; y < board.GetHeight(); y++)
	{
		for (x = 0; x < board.GetWidth(); x++)
		{
			if (board.IsBlack(x, y))
				break;
		}

		if (board.IsBlack(x, y))
			break;
	}

	if (x == board.GetWidth() && y == board.GetHeight())
	{
		// there is no black square which is technically
		// probably satisfies our condition.
		// if we have a 2x1 board and one square has 2
		// as white size, then the board is solved even
		// without any black squares
		return true;
	}

	// search for all connecting blacks
	std::vector<Point> all = FindConnectingSquares(board, x, y);

	// look through remainder of squares and check
	// if there exists a black that is not in "all"
	for (; y < board.GetHeight(); y++)
	{
		for (; x < board.GetWidth(); x++)
		{
			if (board.IsBlack(x, y))
			{
				if (std::find(all.begin(), all.end(), Point{ x, y }) == all.end())
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
			if (board.IsBlack(x, y) &&
				board.IsBlack(x - 1, y) &&
				board.IsBlack(x, y - 1) &&
				board.IsBlack(x - 1, y - 1))
			{
				return true;
			}
		}
	}
	return false;
}

bool Rules::IsProperSize(const Board& board, int x, int y)
{
	auto required = board.GetRequiredSize(x, y);
	if (required == 0)
		return true;

	auto count = FindConnectingSquares(board, x, y).size();
	return count == required;
}

bool Rules::IsTouchingAnother(const Board& board, int x, int y)
{
	auto touching = FindConnectingSquares(board, x, y);
	int count = 0;
	for (int i = 0; i < touching.size(); i++)
	{
		if (board.GetRequiredSize(touching[i].x, touching[i].y) > 0)
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
			if (board.IsWhite(x, y) && board.GetRequiredSize(x, y) > 0)
			{
				if (!IsProperSize(board, x, y))
					return false;

				if (IsTouchingAnother(board, x, y))
					return false;
			}
		}
	}

	return true;
}


std::vector<Point> Rules::FindConnectingSquares(const Board& board, int x, int y)
{
	std::vector<Point> all;
	std::queue<Point> border;
	border.push({ x, y });

	bool isBlack = board.IsBlack(x, y);
	auto AddIfNewAndValid = [&board, &all, &border, isBlack](const Point& pt)
	{
		if (board.IsValidPosition(pt.x, pt.y) && board.IsBlack(pt.x, pt.y) == isBlack &&
			std::find(all.begin(), all.end(), pt) == all.end())
		{
			border.push(pt);
		}
	};

	while (border.size())
	{
		Point current = border.front();
		border.pop();

		all.push_back(current);

		for (int i = 0; i < border.size(); i++)
		{
			AddIfNewAndValid({ current.x - 1, current.y });
			AddIfNewAndValid({ current.x + 1, current.y });
			AddIfNewAndValid({ current.x, current.y - 1 });
			AddIfNewAndValid({ current.x, current.y + 1 });
		}
	}
	return all;
}