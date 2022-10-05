// NurikabeSolver.cpp : Defines the entry point for the application.
//

#include "NurikabeBoard.h"
#include <fstream>
#include <memory>

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
	memcpy(squares, other.squares, sizeof(Square) * width * height);
}

Board::~Board()
{
	if (squares)
	{
		delete[] squares;
		squares = nullptr;
	}
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

		if (val == ' ')
		{
			// square is white
			squares[y * width + x] = Square(false, 0);
		}
		else if (val >= '1' && val <= '9')
		{
			// parse digits
			squares[y * width + x] = Square(false, val - '0');
		}
		else if (val >= 'a' && val <= 'z')
		{
			// parse higher numbers, represented by letters
			squares[y * width + x] = Square(false, val - 'a' + 10);
		}
		else if (val >= 'A' && val <= 'Z')
		{
			// parse higher numbers, represented by letters
			squares[y * width + x] = Square(false, val - 'A' + 10);
		}
		else
		{
			// for any other symbol we assume the square is black
			squares[y * width + x] = Square(true, 0);
		}

		x++;
	}

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

Nurikabe::Square& Board::Get(int x, int y)
{
	return squares[y * width + x];
}

const Nurikabe::Square& Board::Get(int x, int y) const
{
	return squares[y * width + x];
}

bool Board::IsValidPosition(int x, int y) const
{
	return
		x >= 0 && x < GetWidth() &&
		y >= 0 && y < GetHeight();
}

bool Board::IsWhite(int x, int y) const
{
	if (!IsValidPosition(x, y))
		return false;
	return !Get(x, y).GetState();
}
bool Board::IsWhiteOrWall(int x, int y) const
{
	if (!IsValidPosition(x, y))
		return true;
	return !Get(x, y).GetState();
}
bool Board::IsBlack(int x, int y) const
{
	if (!IsValidPosition(x, y))
		return false;
	return Get(x, y).GetState();
}
bool Board::IsBlackOrWall(int x, int y) const
{
	if (!IsValidPosition(x, y))
		return true;
	return Get(x, y).GetState();
}

int Board::GetRequiredSize(int x, int y) const
{
	if (!IsValidPosition(x, y))
		return 0;
	return Get(x, y).GetSize();
}

void Board::SetWhite(int x, int y)
{
	if (!IsValidPosition(x, y))
		return;
	Get(x, y).SetState(false);
}
void Board::SetBlack(int x, int y)
{
	if (!IsValidPosition(x, y))
		return;
	Get(x, y).SetState(true);
}
void Board::SetSize(int x, int y, int size)
{
	if (!IsValidPosition(x, y))
		return;
	Get(x, y).SetSize(size);
}

void Board::Print(std::ostream& stream) const
{
	stream.put('+');
	for (int x = 0; x < width; x++)
	{
		stream.put('-');
	}
	stream.put('+');
	stream << std::endl;

	for (int y = 0; y < height; y++)
	{
		stream << '|';
		for (int x = 0; x < width; x++)
		{
			auto& val = Get(x, y);
			//if (val.isFinal)
			//{
			//	if (val.isBlack)
			//	{
			//		stream.put(' ');
			//	}
			//	else
			//	{
			//		if (val.size == 0)
			//		{
			//			stream.put('#');
			//		}
			//		else
			//		{
			//			if (val.size < 10)
			//			{
			//				stream.put(val.size + '0');
			//			}
			//			else
			//			{
			//				stream.put(val.size - 10 + 'a');
			//			}
			//		}
			//	}
			//}
			//else
			//{
			if (val.GetState())
			{
				stream.put(' ');
			}
			else
			{
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
			}
		}
		stream.put('|');
		stream << std::endl;
	}

	stream.put('+');
	for (int x = 0; x < width; x++)
	{
		stream.put('-');
	}
	stream.put('+');
}