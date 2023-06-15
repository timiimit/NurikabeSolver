// NurikabeSolver.cpp : Defines the entry point for the application.
//

#include "NurikabeBoard.h"
#include <fstream>
#include <cstring>
#include <utility>

using namespace Nurikabe;

Board::Board()
	: squares(nullptr)
	, width(0)
	, height(0)
	, iteration(0)
{
}

Board::Board(const Board& other)
	: squares(new Square[other.GetWidth() * other.GetHeight()])
	, width(other.width)
	, height(other.height)
	, iteration(other.iteration)
{
	std::memcpy(squares, other.squares, sizeof(Square) * width * height);
}

Board::Board(Board&& other)
	: squares(std::exchange(other.squares, nullptr))
	, width(other.width)
	, height(other.height)
	, iteration(other.iteration)
{
	
}

Board::~Board()
{
	if (squares)
	{
		delete[] squares;
		squares = nullptr;
	}
}

Board& Board::operator=(const Board& other)
{
	if (squares)
		delete[] squares;

	squares = new Square[other.GetWidth() * other.GetHeight()];
	std::memcpy(squares, other.squares, sizeof(Square) * width * height);

	width = other.width;
	height = other.height;
	iteration = other.iteration;

	return *this;
}

Board& Board::operator=(Board&& other)
{
	squares = std::exchange(other.squares, squares);
	width = std::exchange(other.width, width);
	height = std::exchange(other.height, height);
	iteration = std::exchange(other.iteration, iteration);

	return *this;
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

Square& Board::GetInternal(const Point& pt)
{
	return squares[pt.y * width + pt.x];
}

const Square& Board::Get(const Point& pt) const
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
	GetInternal(pt).SetState(SquareState::White);
	iteration++;
}
void Board::SetBlack(const Point& pt)
{
	if (!IsValidPosition(pt))
		return;
	GetInternal(pt).SetState(SquareState::Black);
	iteration++;
}
void Board::SetSize(const Point& pt, int size)
{
	if (!IsValidPosition(pt))
		return;
	GetInternal(pt).SetSize(size);
	iteration++;
}
void Board::SetOrigin(const Point& pt, int origin)
{
	if (!IsValidPosition(pt))
		return;
	GetInternal(pt).SetOrigin(origin);
	iteration++;
}


void Board::ForEachSquare(const PointSquareDelegate& callback) const
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
	const Board* pThis[] = { this };
	Board::Print(pThis, 1, stream);
}

void Board::Print(const Board** boards, int boardCount, std::ostream& stream)
{
	// We start with drawing 2 lines of stuff above board content
	int row = -2;
	int maxHeight = 0;
	bool isLastRow = false;
	while (true)
	{
		for (int bi = 0; bi < boardCount; bi++)
		{
			// Draw some space between boards
			if (bi > 0)
				stream << ' ';

			const auto& board = *(boards[bi]);
			if (row == -2)
			{
				// Draw X-Axis numbers [0-9]
				stream.put(' ');
				stream.put(' ');
				stream.put(' ');
				for (int x = 0; x < board.width; x++)
				{
					stream << x % 10;
					if (x < board.width - 1)
						stream.put(' ');

					// find max board height
					if (board.height > maxHeight)
						maxHeight = board.height;
				}
				stream.put(' ');
			}
			else if (row == -1 || row == board.height)
			{
				// Draw top or bottom border
				stream.put(' ');
				stream.put(' ');
				stream.put('+');
				for (int x = 0; x < board.width; x++)
				{
					stream.put('-');
					if (x < board.width - 1)
						stream.put('-');
				}
				stream.put('+');
			}
			else
			{
				int y = row;

				// Draw Y-Axis numbers [0-99]
				int yMod = y % 100;
				if (yMod < 10)
					stream << ' ';
				stream << yMod;

				// Draw left border
				stream << '|';

				// Draw board row
				for (int x = 0; x < board.width; x++)
				{
					const auto& val = board.Get({ x, y });
					switch (val.GetState())
					{
					case SquareState::Unknown:
						stream.put('*');
						break;

					case SquareState::Wall:
						stream.put('+');
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
					if (x < board.width - 1)
						stream.put(' ');
				}

				// Draw right border
				stream.put('|');
			}
		}

		// Go to new line
		stream << std::endl;

		if (isLastRow)
			break;

		row++;

		// We want to draw one more row after the end of boards
		if (row >= maxHeight)
			isLastRow = true;
	}
}