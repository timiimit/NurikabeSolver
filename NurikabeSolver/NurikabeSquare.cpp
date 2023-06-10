#include "NurikabeSquare.h"

using namespace Nurikabe;

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

SquareState Square::GetState() const
{
	return state;
}

void Square::SetState(SquareState state)
{
	this->state = state;
}

uint8_t Square::GetSize() const
{
	return size;
}
void Square::SetSize(uint8_t size)
{
	this->size = size;
}