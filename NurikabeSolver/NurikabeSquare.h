#pragma once
#include <cinttypes>

namespace Nurikabe
{
	class Square
	{
		bool isBlack : 1;
		uint8_t size : 7;

	public:
		void SetState(bool isBlack);
		bool GetState() const;

		uint8_t GetSize() const;
		void SetSize(uint8_t size);

	public:
		Square(bool isBlack, uint8_t size);
		Square();
	};
}