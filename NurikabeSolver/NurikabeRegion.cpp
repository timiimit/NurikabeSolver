#include "NurikabeRegion.h"
#include "NurikabeBoard.h"
#include <assert.h>

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

void Region::SetState(SquareState state) const
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

void Region::SetSize(uint8_t size) const
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

void Region::SetOrigin(uint8_t origin) const
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

void Region::ForEachContiguousRegion(const RegionDelegate& callback)
{
	auto& board = this->board;

	Region handled(board);

	ForEach([this, &handled, &callback](const Point& pt, const Square&)
	{
		if (handled.Contains(pt))
			return true;

		Region contiguous = Region(GetBoard(), pt).ExpandAllInline(
			[this](const Point& ptInner, const Square&) { return Contains(ptInner); }
		);

		handled = Region::Union(handled, contiguous);

		return callback(contiguous);
	});
}

Nurikabe::Region Region::Union(const Region& a, const Region& b)
{
	if (a.GetBoard() != b.GetBoard())
		return Region(nullptr);

	Region ret(a);
	for (int i = 0; i < b.GetSquareCount(); i++)
	{
		if (!ret.Contains(b.squares[i]))
			ret.squares.push_back(b.squares[i]);
	}
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
		const auto pt = squares[i];
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

bool Region::StartNeighbourSpill(Square& out) const
{
	if (!IsSameState())
		return false;

	auto origin = GetSameOrigin();
	auto size = GetSameSize();
	if (origin == (uint8_t)~0)
		size = GetSquareCount();

	auto sq = Square(GetState(), size);
	sq.SetOrigin(origin);

	out = sq;
	return true;
}

Region Region::NeighbourSpill(const Square& sq) const
{
	auto direct = Neighbours([this, &sq](const Point& pt, const Square& sqInner)
	{
		if (sq.GetState() == SquareState::Black)
		{
			return sqInner.GetState() == SquareState::Black || sqInner.GetState() == SquareState::Unknown;
		}
		else if (sq.GetState() == SquareState::White)
		{
			if (sq.GetOrigin() == (uint8_t)~0)
			{
				if (sqInner.GetState() == SquareState::White)
				{
					if (sqInner.GetOrigin() == (uint8_t)~0)
						return true;
						
					auto whiteActualSize =
						Region((Board*)GetBoard(), pt)
						.ExpandAllInline([](const Point&, const Square& sq) { return sq.GetState() == SquareState::White; })
						.GetSquareCount();

					if (sq.GetSize() + 1 + whiteActualSize <= sqInner.GetSize())
						return true;
				}
				else if (sqInner.GetState() == SquareState::Unknown)
				{
					auto count = Region((Board*)GetBoard(), pt).Neighbours([this, &sq](const Point& ptInner, const Square& sqInner)
					{
						if (sqInner.GetState() != SquareState::White)
							return false;

						if (sqInner.GetOrigin() == (uint8_t)~0)
							return false;

						auto whiteActualSize =
							Region((Board*)GetBoard(), ptInner)
							.ExpandAllInline([](const Point&, const Square& sq) { return sq.GetState() == SquareState::White; })
							.GetSquareCount();

						if (sq.GetSize() + 1 + whiteActualSize <= sqInner.GetSize())
							return false;

						return true;
					}).GetSquareCount();

					return count == 0;
				}
			}
			else
			{
				if (sqInner.GetState() == SquareState::White)
				{
					if (sqInner.GetOrigin() == sq.GetOrigin() || sqInner.GetOrigin() == (uint8_t)~0)
					{
						return true;
					}
				}
				else if (sqInner.GetState() == SquareState::Unknown)
				{
					if (pt == Point{2,7} || pt == Point{3,8})
					{
						int a = 0;
					}
					auto count = Region((Board*)GetBoard(), pt).Neighbours([this, &sq](const Point& ptInner, const Square& sqInner)
					{
						if (sqInner.GetState() != SquareState::White)
							return false;
						
						if (sqInner.GetOrigin() != (uint8_t)~0)
						{
							if (sqInner.GetOrigin() == sq.GetOrigin())
								return false;
						}
						else
						{
							auto whiteActualSize =
								Region((Board*)GetBoard(), ptInner)
								.ExpandAllInline([](const Point&, const Square& sq) { return sq.GetState() == SquareState::White; })
								.GetSquareCount();

							if (GetSquareCount() + 1 + whiteActualSize <= sq.GetSize())
								return false;
						}
						
						return true;
					}).GetSquareCount();
					
					return count == 0;
				}
			}

			return false;
		}
		else
		{
			return sqInner.GetState() == sq.GetState();
		}

		return true;
	});

	if (sq.GetOrigin() == (uint8_t)~0 && sq.GetState() == SquareState::White && direct.GetSquareCount() > 1 && !direct.IsContiguous())
	{
		Region removeFromDirect = Region((Board*)GetBoard());

		direct.ForEachContiguousRegion([this, &sq, &direct, &removeFromDirect](const Region& r)
		{
			auto notR = Region::Subtract(direct, r);
			auto ignored = Region::Intersection(*this, direct.Neighbours());

			assert(ignored.GetSquareCount() > 0);

			bool isClosed = true;
			auto state = r.GetState();
			Region(r).ExpandAllInline([&notR, &ignored, state, &isClosed](const Point& pt, const Square& sqInner)
			{
				if (!isClosed)
					return false;

				if (sqInner.GetState() == SquareState::Black)
					return false;

				if (sqInner.GetState() == SquareState::Unknown)
					return true;

				if (ignored.Contains(pt))
				{
					return false;
				}

				if (sqInner.GetState() == SquareState::White)
				{
					isClosed = false;
					return false;
				}

				if (notR.Contains(pt))
				{
					isClosed = false;
					return false;
				}

				return true;
			});

			if (isClosed)
				removeFromDirect = Region::Union(removeFromDirect, r);

			return true;
		});

		if (removeFromDirect.GetSquareCount() > 0)
		{
			direct = Region::Subtract(direct, removeFromDirect);
		}
	}

	return direct;
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

