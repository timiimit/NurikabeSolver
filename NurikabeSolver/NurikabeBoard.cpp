// NurikabeSolver.cpp : Defines the entry point for the application.
//

#include "NurikabeBoard.h"
#include <fstream>
#include <cstring>

using namespace Nurikabe;

Board::Board()
	: squares(nullptr)
	, width(0)
	, height(0)
{
}

Board::Board(const Board& other)
	: squares(new Square[other.GetWidth() * other.GetHeight()])
	, width(other.width)
	, height(other.height)
{
	std::memcpy(squares, other.squares, sizeof(Square) * width * height);
}

Board::~Board()
{
	if (squares)
	{
		delete[] squares;
		squares = nullptr;
	}
}

bool Board::operator==(const Board& other) const
{
	for (int y = 0; y < other.GetHeight(); y++)
	{
		for (int x = 0; x < other.GetWidth(); x++)
		{
			Point pt = { x, y };
			if (Get(pt) != other.Get(pt))
				return false;
		}
	}
	return true;
}

bool Board::Load(const char* filename)
{
	std::ifstream stream(filename, std::ios::binary);
	if (!stream.is_open())
		return false;

	int boardSize = 10000;
	Square* squares = new Square[boardSize];
	memset(squares, 0, boardSize * sizeof(*squares));
	int x = 0;
	int y = 0;
	char val;

	// read byte by byte
	// boards arent that big, so this shouldn't be a problem
	while (stream.read(&val, sizeof(val)))
	{
		if (val == '|' || val == '-' || val == '+')
		{
			// we let the file contain these symbols to mark
			// edges of the board, ignore character
			continue;
		}

		if (val == '\r' || val == '\n')
		{
			// detected end of line

			// if x = 0 then the line is empty, we can ignore those
			if (x == 0)
				continue;

			if (width != 0)
			{
				if (width != x)
				{
					// width is not the same on every line in the file
					delete[] squares;
					return false;
				}
			}

			// here we are either at the first end of line
			// or the line matched the length of previous lines
			width = x;
			x = 0;

			// move to next line
			y++;

			// we don't want to increment x
			continue;
		}

		if (val == ' ' || val == '_')
		{
			// square is white
			squares[y * width + x] = Square(SquareState::Unknown, 0);
		}
		else if (val >= '1' && val <= '9')
		{
			// parse digits
			squares[y * width + x] = Square(SquareState::White, val - '0');
		}
		else if (val >= 'a' && val <= 'z')
		{
			// parse higher numbers, represented by letters
			squares[y * width + x] = Square(SquareState::White, val - 'a' + 10);
		}
		else if (val >= 'A' && val <= 'Z')
		{
			// parse higher numbers, represented by letters
			squares[y * width + x] = Square(SquareState::White, val - 'A' + 10);
		}
		else
		{
			// for any other square we have no clue what it is
			squares[y * width + x] = Square(SquareState::Unknown, 0);
		}

		x++;
	}

	// if last character was newline, make sure to increment y
	if (!(val == '\r' || val == '\n') && x == width)
		y++;

	// remember height
	height = y;

	// give ownership of "board" to class
	this->squares = squares;

	return true;
}

bool Board::IsLoaded() const
{
	return squares != nullptr;
}

Nurikabe::Square& Board::Get(const Point& pt)
{
	return squares[pt.y * width + pt.x];
}

const Nurikabe::Square& Board::Get(const Point& pt) const
{
	return squares[pt.y * width + pt.x];
}

bool Board::IsValidPosition(const Point& pt) const
{
	return
		pt.x >= 0 && pt.x < GetWidth() &&
		pt.y >= 0 && pt.y < GetHeight();
}

bool Board::IsWhite(const Point& pt) const
{
	if (!IsValidPosition(pt))
		return false;
	return Get(pt).GetState() == SquareState::White;
}
bool Board::IsWhiteOrWall(const Point& pt) const
{
	if (!IsValidPosition(pt))
		return true;
	return Get(pt).GetState() == SquareState::White;
}
bool Board::IsBlack(const Point& pt) const
{
	if (!IsValidPosition(pt))
		return false;
	return Get(pt).GetState() == SquareState::Black;
}
bool Board::IsBlackOrWall(const Point& pt) const
{
	if (!IsValidPosition(pt))
		return true;
	return Get(pt).GetState() == SquareState::Black;
}

int Board::GetRequiredSize(const Point& pt) const
{
	if (!IsValidPosition(pt))
		return 0;
	return Get(pt).GetSize();
}

void Board::SetWhite(const Point& pt)
{
	if (!IsValidPosition(pt))
		return;
	Get(pt).SetState(SquareState::White);
}
void Board::SetBlack(const Point& pt)
{
	if (!IsValidPosition(pt))
		return;
	Get(pt).SetState(SquareState::Black);
}
void Board::SetSize(const Point& pt, int size)
{
	if (!IsValidPosition(pt))
		return;
	Get(pt).SetSize(size);
}


void Board::ForEachSquare(const std::function<bool(const Point&, Square&)>& callback)
{
	for (int y = 0; y < GetHeight(); y++)
	{
		for (int x = 0; x < GetWidth(); x++)
		{
			Point pt = { x, y };
			if (!callback(pt, Get(pt)))
				return;
		}
	}
}

void Board::ForEachSquare(const std::function<bool(const Point&, const Square&)>& callback) const
{
	for (int y = 0; y < GetHeight(); y++)
	{
		for (int x = 0; x < GetWidth(); x++)
		{
			Point pt = { x, y };
			if (!callback(pt, Get(pt)))
				return;
		}
	}
}

void Board::Print(std::ostream& stream) const
{
	stream.put('+');
	for (int x = 0; x < width; x++)
	{
		stream.put('-');
		if (x < width - 1)
			stream.put('-');
	}
	stream.put('+');
	stream << std::endl;

	for (int y = 0; y < height; y++)
	{
		stream << '|';
		for (int x = 0; x < width; x++)
		{
			auto& val = Get({ x, y });
			switch (val.GetState())
			{
			case SquareState::Unknown:
				stream.put('?');
				break;

			case SquareState::White:
				if (val.GetSize() == 0)
				{
					stream.put('#');
				}
				else if (val.GetSize() < 10)
				{
					stream.put(val.GetSize() + '0');
				}
				else
				{
					stream.put(val.GetSize() - 10 + 'a');
				}
				break;
			case SquareState::Black:
				stream.put(' ');
				break;
			}
			if (x < width - 1)
				stream.put(' ');
		}
		stream.put('|');
		stream << std::endl;
	}

	stream.put('+');
	for (int x = 0; x < width; x++)
	{
		stream.put('-');
		if (x < width - 1)
			stream.put('-');
	}
	stream.put('+');
	stream << std::endl;
}