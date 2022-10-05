#pragma once
#include "NurikabeSquare.h"
#include <ostream>

namespace Nurikabe
{
	class Board
	{
		Square* squares;

		int width;
		int height;

	public:
		int GetWidth() const { return width; }
		int GetHeight() const { return height; }

	public:
		Board();
		Board(const Board& other);
		~Board();
	public:
		bool Load(const char* filename);
		bool IsLoaded() const;

	protected:
		Square& Get(int x, int y);
		const Square& Get(int x, int y) const;

	public:
		bool IsValidPosition(int x, int y) const;

		bool IsWhite(int x, int y) const;
		bool IsWhiteOrWall(int x, int y) const;
		bool IsBlack(int x, int y) const;
		bool IsBlackOrWall(int x, int y) const;

		// returns the mandatory size of whatever is at this position, 0 means it can change color
		int GetRequiredSize(int x, int y) const;

	public:
		void SetWhite(int x, int y);
		void SetBlack(int x, int y);
		void SetSize(int x, int y, int size);

	public:
		void Print(std::ostream& stream) const;
	};
}