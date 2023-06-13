#pragma once

struct Point
{
	int x = 0;
	int y = 0;

	Point Left() const;
	Point Right() const;
	Point Up() const;
	Point Down() const;
};

int Distance(int a, int b);
int Distance(const Point& a, const Point& b);

bool operator==(const Point& a, const Point& b);

Point operator-(const Point& a, const Point& b);
Point operator+(const Point& a, const Point& b);