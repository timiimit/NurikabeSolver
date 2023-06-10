#include "Point.h"

Point Point::Left() const
{
	return Point{ x - 1, y };
}

Point Point::Right() const
{
	return Point{ x + 1, y };
}

Point Point::Up() const
{
	return Point{ x, y - 1 };
}

Point Point::Down() const
{
	return Point{ x, y + 1 };
}

int Distance(int a, int b)
{
	int d = a - b;
	if (d < 0)
		return -d;
	return d;
}

int Distance(const Point& a, const Point& b)
{
	return Distance(a.x, b.x) + Distance(a.y, b.y);
}

bool operator==(const Point& a, const Point& b)
{
	return
		a.x == b.x &&
		a.y == b.y;
}

Point operator-(const Point& a, const Point& b)
{
	return { a.x - b.x, a.y - b.y };
}

Point operator+(const Point& a, const Point& b)
{
	return { a.x + b.x, a.y + b.y };
}