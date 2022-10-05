#include "NurikabeSquare.h"

using namespace Nurikabe;

Square::Square(bool isBlack, uint8_t size)
	: isBlack(isBlack)
	, size(size) // WARNING: unhandled overflow (this->size is not 8 bits)
{

}

Square::Square()
	: Square(false, 0)
{

}

bool Square::GetState() const
{
	return isBlack;
}

void Square::SetState(bool isBlack)
{
	this->isBlack = isBlack;
}

uint8_t Square::GetSize() const
{
	return size;
}
void Square::SetSize(uint8_t size)
{
	this->size = size;
}