#pragma once
#include "NurikabeSquare.h"
#include "Point.h"
#include <ostream>
#include <functional>
//#include <vector>

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
		Board(Board&& other);
		~Board();
		
		Board& operator=(const Board& other);
		Board& operator=(Board&& other);

		bool operator==(const Board& other) const;
	public:
		bool Load(const char* filename);
		bool IsLoaded() const;

	public:
		Square& Get(const Point& pt);
		const Square& Get(const Point& pt) const;

	public:
		bool IsValidPosition(const Point& pt) const;

		bool IsWhite(const Point& pt) const;
		bool IsWhiteOrWall(const Point& pt) const;
		bool IsBlack(const Point& pt) const;
		bool IsBlackOrWall(const Point& pt) const;
		bool IsUnknown(const Point& pt) const;
		bool IsUnknownOrWall(const Point& pt) const;

		// returns the mandatory size of whatever is at this position, 0 means it can change color
		int GetRequiredSize(const Point& pt) const;

	public:
		void SetWhite(const Point& pt);
		void SetBlack(const Point& pt);
		void SetSize(const Point& pt, int size);


	public:

		bool IsFinal(const Point& pt) const;
		void SetFinal(const Point& pt);

	public:
		void ForEachSquare(const PointSquareDelegate& callback);
		void ForEachSquare(const PointSquareConstDelegate& callback) const;

		void Print(std::ostream& stream) const;
	};
}