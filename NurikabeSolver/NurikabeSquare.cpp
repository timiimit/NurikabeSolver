#include "NurikabeSquare.h"

using namespace Nurikabe;

SquareState Square::GetState() const
{
	return state;
}

void Square::SetState(SquareState state)
{
	this->state = state;
}

uint8_t Square::GetOrigin() const
{
	return origin;
}

void Square::SetOrigin(uint8_t origin)
{
	this->origin = origin;
}

uint8_t Square::GetSize() const
{
	return size;
}

void Square::SetSize(uint8_t size)
{
	this->size = size;
}

Square::Square(SquareState state, uint8_t size)
	: state(state)
	, origin(~0)
	, size(size) // WARNING: unhandled overflow (this->size is not 8 bits)
{

}

Square::Square()
	: Square(SquareState::Unknown, 0)
{

}

bool Nurikabe::Square::operator==(const Square &other) const
{
    return
		state == other.state &&
		size == other.size &&
		origin == other.origin;
}