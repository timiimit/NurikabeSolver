#include "NurikabeRegion.h"
#include "NurikabeBoard.h"

using namespace Nurikabe;

bool Region::IsSameState() const
{
	if (squares.size() == 0)
		return false;
	if (squares.size() == 1)
		return true;

	auto state = board->Get(squares[0]).GetState();
	for (int i = 0; i < squares.size(); i++)
	{
		if (board->Get(squares[i]).GetState() != state)
		{
			return false;
		}
	}

	return true;
}

bool Region::IsSameOrigin() const
{
	uint8_t invalidVal = ~0;

	if (squares.size() == 0)
		return invalidVal;

	auto val = invalidVal;
	for (int i = 0; i < squares.size(); i++)
	{
		auto sqVal = board->Get(squares[i]).GetOrigin();
		if (sqVal == invalidVal)
			continue;

		if (val == invalidVal)
			val = sqVal;
		
		if (sqVal != val)
			return false;
	}

	return true;
}

bool Region::IsContiguous() const
{
	if (squares.size() < 2)
		return true;

	const auto& squares = this->squares;

	auto contiguous = Region(board, squares[0]).ExpandAllInline([&squares](const Point& pt, const Square& sq) {
		return std::find(squares.begin(), squares.end(), pt) != squares.end();
	});

	return *this == contiguous;
}

SquareState Region::GetState() const
{
	if (squares.size() < 1)
		return SquareState::Unknown;

	return board->Get(squares[0]).GetState();
}

void Region::SetState(SquareState state)
{
	for (int i = 0; i < squares.size(); i++)
	{
		board->Get(squares[i]).SetState(state);
	}
}

uint8_t Region::GetSameSize() const
{
	uint8_t invalidVal = 0;

	if (squares.size() == 0)
		return invalidVal;

	auto val = invalidVal;
	for (int i = 0; i < squares.size(); i++)
	{
		auto sqVal = board->Get(squares[i]).GetSize();
		if (sqVal == invalidVal)
			continue;

		if (val == invalidVal)
			val = sqVal;
		
		if (sqVal != val)
			return invalidVal;
	}

	return val;
}

void Region::SetSize(uint8_t size)
{
	for (int i = 0; i < squares.size(); i++)
	{
		board->Get(squares[i]).SetSize(size);
	}
}

uint8_t Region::GetSameOrigin() const
{
	uint8_t invalidVal = ~0;

	if (squares.size() == 0)
		return invalidVal;

	auto val = invalidVal;
	for (int i = 0; i < squares.size(); i++)
	{
		auto sqVal = board->Get(squares[i]).GetOrigin();
		if (sqVal == invalidVal)
			continue;

		if (val == invalidVal)
			val = sqVal;
		
		if (sqVal != val)
			return invalidVal;
	}

	return val;
}

void Region::SetOrigin(uint8_t origin)
{
	for (int i = 0; i < squares.size(); i++)
	{
		board->Get(squares[i]).SetOrigin(origin);
	}
}


bool Nurikabe::operator==(const Region& a, const Region& b)
{
	if (a.GetSquareCount() != b.GetSquareCount())
		return false;

	for (int i = 0; i < a.GetSquareCount(); i++)
	{
		if (!a.Contains(b.GetSquares()[i]))
			return false;
	}

	return true;
}

bool Region::Contains(const Point& pt) const
{
	return std::find(squares.begin(), squares.end(), pt) != squares.end();
}

void Region::FixWhites()
{
	auto origin = GetSameOrigin();
	if (origin != (uint8_t)~0)
		SetOrigin(origin);
	
	auto size = GetSameSize();
	if (size != 0)
		SetSize(size);
}

Region::Region()
{
	this->board = nullptr;
}

Region::Region(Board* board)
{
	this->board = board;
}

Region::Region(Board* board, std::vector<Point> squares)
{
	this->board = board;
	this->squares = squares;
}

Region::Region(Board* board, const Point& square)
{
	this->board = board;
	this->squares.push_back(square);
}

void Region::ForEach(const PointSquareDelegate& callback)
{
	for (int i = 0; i < squares.size(); i++)
	{
		if (!callback(squares[i], board->Get(squares[i])))
			break;
	}
}

Nurikabe::Region Region::Union(const Region& a, const Region& b)
{
	if (a.GetBoard() != b.GetBoard())
		return Region(nullptr);

	Region ret(a);
	ret.squares.insert(ret.squares.end(), b.squares.begin(), b.squares.end());
	// NOTE: we assume regions dont overlap
	return ret;
}

Region Region::Intersection(const Region& a, const Region& b)
{
	if (a.GetBoard() != b.GetBoard())
		return Region(nullptr);

	Region ret(a.board);
	ret.squares.reserve((size_t)std::min(a.squares.size(), b.squares.size()));
	for (size_t i = 0; i < a.squares.size(); i++)
	{
		if (std::find(b.squares.begin(), b.squares.end(), a.squares[i]) != b.squares.end())
		{
			ret.squares.push_back(a.squares[i]);
		}
	}
	return ret;
}

Region Region::Subtract(const Region& a, const Region& b)
{
	if (a.GetBoard() != b.GetBoard())
		return Region(nullptr);

	Region ret(a.board);
	ret.squares.reserve(a.squares.size());
	for (size_t i = 0; i < a.squares.size(); i++)
	{
		if (std::find(b.squares.begin(), b.squares.end(), a.squares[i]) == b.squares.end())
		{
			ret.squares.push_back(a.squares[i]);
		}
	}
	return ret;
}

Region Region::Neighbours(const PointSquareDelegate& predicate, bool includeWalls) const
{
	Region ret(board);
	auto AddIfNewAndValid = [this, &ret, &predicate, &includeWalls](const Point& pt)
	{
		// ignore squares not on the board
		if (!includeWalls && !board->IsValidPosition(pt))
			return;

		// ignore squares already in "ret"
		if (std::find(ret.squares.begin(), ret.squares.end(), pt) != ret.squares.end())
			return;

		// ignore squares in current region
		if (std::find(squares.begin(), squares.end(), pt) != squares.end())
			return;

		// ignore squares not required by predicate
		if (!predicate(pt, board->Get(pt)))
			return;

		ret.squares.push_back(pt);
	};
	for (int i = 0; i < squares.size(); i++)
	{
		const auto& pt = squares[i];
		AddIfNewAndValid(pt.Left());
		AddIfNewAndValid(pt.Right());
		AddIfNewAndValid(pt.Up());
		AddIfNewAndValid(pt.Down());
	}
	return ret;
}

Region& Region::ExpandSingleInline(const PointSquareDelegate& predicate, bool includeWalls)
{
	auto AddIfNewAndValid = [this, &predicate, &includeWalls](const Point& pt)
	{
		// ignore squares not on the board
		if (!includeWalls && !board->IsValidPosition(pt))
			return;

		// ignore squares already in current region
		if (std::find(squares.begin(), squares.end(), pt) != squares.end())
			return;

		// ignore squares not required by predicate
		if (!predicate(pt, board->Get(pt)))
			return;

		squares.push_back(pt);
	};
	int sqCount = GetSquareCount();
	for (int i = 0; i < sqCount; i++)
	{
		const auto& pt = squares[i];
		AddIfNewAndValid(pt.Left());
		AddIfNewAndValid(pt.Right());
		AddIfNewAndValid(pt.Up());
		AddIfNewAndValid(pt.Down());
	}
	return *this;
}

Region& Region::ExpandAllInline(const PointSquareDelegate& predicate)
{
	int sqCount = GetSquareCount();
	while (true)
	{
		ExpandSingleInline(predicate);

		int sqCountNew = GetSquareCount();
		if (sqCount == sqCountNew)
			break;
		sqCount = sqCountNew;
	}
	return *this;
}

//Region Region::GetDirectNeighbours(bool includeWalls) const
//{
//	std::vector<Point> ret;
//	auto AddIfNewAndValid = [this, &ret, &includeWalls](const Point& pt)
//	{
//		if (std::find(ret.begin(), ret.end(), pt) == ret.end())
//		{
//			// this square is not in "ret" yet.
//			if (board->IsValidPosition(pt))
//			{
//				// this square is a valid square on a board
//				if (std::find(squares.begin(), squares.end(), pt) == squares.end())
//				{
//					// this square is not in this region
//					ret.push_back(pt);
//				}
//			}
//			else
//			{
//				// this point is not a valid square on the board
//				if (includeWalls)
//				{
//					ret.push_back(pt);
//				}
//			}
//		}
//	};
//	for (int i = 0; i < squares.size(); i++)
//	{
//		const auto& pt = squares[i];
//		AddIfNewAndValid(pt.Left());
//		AddIfNewAndValid(pt.Right());
//		AddIfNewAndValid(pt.Up());
//		AddIfNewAndValid(pt.Down());
//	}
//	return Region(board, ret);
//}
//
//Region Region::GetDirectNeighboursWithState(const std::vector<SquareState>& _states) const
//{
//	// get rid of duplicates just in case
//	bool hasWall = false;
//	std::vector<SquareState> states;
//	for (int i = 0; i < _states.size(); i++)
//	{
//		if (_states[i] == SquareState::Wall)
//		{
//			hasWall = true;
//			continue;
//		}
//
//		if (std::find(states.begin(), states.end(), _states[i]) == states.end())
//			states.push_back(_states[i]);
//	}
//
//	std::vector<Point> ret;
//	auto AddIfNewAndValid = [this, &ret, states, hasWall](const Point& pt)
//	{
//		if (std::find(ret.begin(), ret.end(), pt) == ret.end())
//		{
//			// this square is not in "ret" yet.
//			if (board->IsValidPosition(pt))
//			{
//				// this square is a valid square on a board
//				if (std::find(squares.begin(), squares.end(), pt) == squares.end())
//				{
//					// this square is not in target region yet
//					ret.push_back(pt);
//				}
//			}
//			else
//			{
//				// this point is not a valid square on the board
//				if (hasWall)
//				{
//					ret.push_back(pt);
//				}
//			}
//		}
//	};
//	for (int i = 0; i < squares.size(); i++)
//	{
//		const auto& pt = squares[i];
//		AddIfNewAndValid(pt.Left());
//		AddIfNewAndValid(pt.Right());
//		AddIfNewAndValid(pt.Up());
//		AddIfNewAndValid(pt.Down());
//	}
//	return Region(board, ret);
//}

