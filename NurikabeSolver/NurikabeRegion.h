#include "Point.h"
#include "NurikabeSquare.h"
#include <functional>
#include <vector>

namespace Nurikabe
{
	class Board;

	class Region
	{
		Board* board;
		std::vector<Point> squares;
	public:
		Board* GetBoard() { return board; }
		const Board* GetBoard() const { return board; }

		const std::vector<Point>& GetSquares() const { return squares; }
		int GetSquareCount() const { return (int)squares.size(); }

	public:
		// true if all squares in this region are of the same state
		bool IsSameState() const;
		bool IsSameOrigin() const;

		// get state of all squares. undefined behavior if IsSameState returns false
		SquareState GetState() const;

		// set state of all squares in this region
		void SetState(SquareState state);
		
		void SetSize(uint8_t size);
		void SetOrigin(uint8_t size);


	public:
		Region();
		Region(Board* board);
		Region(Board* board, std::vector<Point> squares);
		Region(Board* board, const Point& square);

		void ForEach(const PointSquareDelegate& callback);

		static Region Union(const Region& a, const Region& b);
		static Region Intersection(const Region& a, const Region& b);
		static Region Subtract(const Region& a, const Region& b);

		//Region GetDirectNeighbours(const PointSquareDelegate& predicate) const;
		Region Neighbours(const PointSquareDelegate& predicate, bool includeWalls = false) const;
		Region& ExpandSingleInline(const PointSquareDelegate& predicate, bool includeWalls = false);
		Region& ExpandAllInline(const PointSquareDelegate& predicate);
		//Region FindPathTo(Point& pt, const PointSquareDelegate& predicate) const;

		Region Neighbours() const
		{
			return Neighbours([](const Point&, Square&) { return true; });
		}
		Region& ExpandSingleInline()
		{
			return ExpandSingleInline([](const Point&, Square&) { return true; });
		}


		//Region GetDirectNeighbours(bool includeWalls) const;
		//Region GetDirectNeighbours() const { return GetDirectNeighbours(false); }
		//Region GetDirectNeighboursWithState(const std::vector<SquareState>& _states) const;
		//Region GetDirectNeighboursWithState(SquareState state) const
		//{	
		//	std::vector<SquareState> states({ state });
		//	return GetDirectNeighboursWithState(states);
		//}
	};
}