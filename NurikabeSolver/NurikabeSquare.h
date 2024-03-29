#pragma once
#include <cstdint>
#include "Point.h"
#include <functional>

namespace Nurikabe
{
	enum class SquareState : uint8_t
	{
		Unknown,
		White,
		Black,
		Wall
	};

	class Square
	{
		SquareState state;// : 2;
		uint8_t origin;// : 7;
		uint8_t size;// : 7;

	public:
		void SetState(SquareState state);
		SquareState GetState() const;

		uint8_t GetOrigin() const;
		void SetOrigin(uint8_t origin);

		uint8_t GetSize() const;
		void SetSize(uint8_t size);

	public:
		Square(SquareState state, uint8_t size);
		Square();

	public:
		bool operator==(const Square& other) const;
		bool Equals(const Square &other, bool compareOrigin) const;
	};

	typedef std::function<bool(const Point& point, const Square& square)> PointSquareDelegate;
}