#include "Point.h"
#include "NurikabeSquare.h"
#include <functional>
#include <vector>

namespace Nurikabe
{
	class Board;
	class Region;

	typedef std::function<bool(const Region&)> RegionDelegate;

	class Region
	{
		mutable Board* board;
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
		bool IsContiguous() const;
		bool HasSquareWithState(SquareState state) const;

		// get state of all squares. undefined behavior if IsSameState returns false
		SquareState GetState() const;

		// set state of all squares in this region
		void SetState(SquareState state) const;
		
		uint8_t GetSameSize() const;
		void SetSize(uint8_t size) const;

		uint8_t GetSameOrigin() const;
		void SetOrigin(uint8_t size) const;

		friend bool operator==(const Region& a, const Region& b);

		bool Contains(const Point& pt) const;

		void FixWhites();


	public:
		Region();
		Region(Board* board);
		Region(Board* board, std::vector<Point> squares);
		Region(Board* board, const Point& square);

		void ForEach(const PointSquareDelegate& callback) const;
		void ForEachContiguousRegion(const RegionDelegate& callback) const;

		static Region Union(const Region& a, const Region& b);
		static Region Intersection(const Region& a, const Region& b);
		static Region Subtract(const Region& a, const Region& b);

		Region& Append(const Region& other);

		//Region GetDirectNeighbours(const PointSquareDelegate& predicate) const;
		Region Neighbours(const PointSquareDelegate& predicate, bool includeWalls = false) const;
		Region& ExpandSingleInline(const PointSquareDelegate& predicate, bool includeWalls = false);
		Region& ExpandAllInline(const PointSquareDelegate& predicate);

		bool StartNeighbourSpill(Square& out) const;
		Region NeighbourSpill(const Square& sq) const;
		//Region FindPathTo(Point& pt, const PointSquareDelegate& predicate) const;

		Region Neighbours() const
		{
			return Neighbours([](const Point&, const Square&) { return true; });
		}
		Region Neighbours(SquareState state) const
		{
			return Neighbours([state](const Point&, const Square& sq) { return sq.GetState() == state; });
		}
		Region& ExpandSingleInline()
		{
			return ExpandSingleInline([](const Point&, const Square&) { return true; });
		}
		Region& ExpandSingleInline(SquareState state)
		{
			return ExpandSingleInline([state](const Point&, const Square& sq) { return sq.GetState() == state; });
		}
		Region& ExpandAllInline(SquareState state)
		{
			return ExpandAllInline([state](const Point&, const Square& sq) { return sq.GetState() == state; });
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