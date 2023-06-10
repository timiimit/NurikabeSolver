#include "NurikabeSolver.h"
#include <iostream>

using namespace Nurikabe;


Solver::Solver(const Board& initialBoard)
	: board(initialBoard)
//	, flags(initialBoard)
{

}

void Solver::SolveInitial()
{
	board.ForEachSquare([this](const Point& pt, Square& square)
		{
			if (square.GetSize() == 1)
			{
				Region(&board, pt)
					.Neighbours()
					.SetState(SquareState::Black);
			}

			if (square.GetSize() != 0)
			{
				// add this white to our lists
				if (square.GetSize() != 1)
					unsolvedWhites.push_back((int)initialWhites.size());
				initialWhites.push_back(pt);
			}

			return true;
		});

	SolveUnreachable();
}

void Solver::SolvePerSquare()
{
	board.ForEachSquare([this](const Point& pt, Square& square)
		{
			if (square.GetState() == SquareState::Unknown)
			{
				int whiteNeighbourCount = Region(&board, pt)
					.Neighbours([](const Point& pt, Square& square) {
						return square.GetState() == SquareState::White;
					})
					.GetSquareCount();

				if (whiteNeighbourCount > 1)
				{
					board.SetBlack(pt);
					return true;
				}


				Region diagonals = Region(&board, pt)
					.Neighbours([](const Point& pt, Square& square) {
						return square.GetState() == SquareState::Black;
					})
					.Neighbours([&pt](const Point& ptDiag, Square& square) {
						if (Distance(ptDiag.x, pt.x) > 1)
							return false;

						return square.GetState() == SquareState::Black;
					});

				diagonals.ForEach([this, &pt](const Point& ptDiag, Square& square) {
					Point delta = pt - ptDiag;
					if (board.IsBlack(Point{ ptDiag.x + delta.x, ptDiag.y }) &&
						board.IsBlack(Point{ ptDiag.x, ptDiag.y + delta.y }))
					{
						board.SetWhite(pt);
						return false;
					}
					return true;
				});
				
				if (square.GetState() != SquareState::Unknown)
					return true;
			}
			else
			{
				// If there's only one way to go, then go there

				Region region = Region(&board, pt)
					.ExpandAllInline([square](const Point&, Square& squareInner) {
						return squareInner.GetState() == square.GetState();
					})
					.Neighbours([](const Point&, Square& squareInner) {
						return squareInner.GetState() == SquareState::Unknown;
					});

				if (region.GetSquareCount() == 1)
				{
					region.SetState(square.GetState());
					if (square.GetState() == SquareState::White)
					{
						auto iter = std::find(initialWhites.begin(), initialWhites.end(), pt);
						if (iter != initialWhites.end())
						{
							int originIndex = iter - initialWhites.begin();
							region.ForEach([originIndex](const Point&, Square& sq) { sq.SetOrigin((uint8_t)originIndex); return true; });
						}
					}
				}
			}

			return true;
		});
}

void Solver::SolvePerUnsolvedWhite()
{
	for (int i = 0; i < unsolvedWhites.size(); i++)
	{
		Point pt = initialWhites[unsolvedWhites[i]];
		auto region = Region(&board, pt)
			.ExpandAllInline([](const Point& pt, Square& square)
				{
					return square.GetState() == SquareState::White;
				});

		if (board.GetRequiredSize(pt) == region.GetSquareCount())
		{
			// if region is of correct size, mark it as complete
			region
				.Neighbours()
				.SetState(SquareState::Black);

			unsolvedWhites.erase(unsolvedWhites.begin() + i);
			i--;
			continue;
		}
	}
}

void Solver::SolveUnreachable()
{
	auto& initialWhites = this->initialWhites;
	auto& unsolvedWhites = this->unsolvedWhites;
	auto& board = this->board;

	board.ForEachSquare([&initialWhites, &unsolvedWhites, &board](const Point& pt, Square& sq)
	{
		if (sq.GetState() != SquareState::Unknown)
			return true;

		bool isReachable = false;
		for (int i = 0; i < unsolvedWhites.size() && !isReachable; i++)
		{
			auto& initialWhite = initialWhites[unsolvedWhites[i]];
			auto region = Region(&board, initialWhite)
				.ExpandAllInline([](const Point&, Square& sq) { return sq.GetState() == SquareState::White; });

			auto regionSquaresLeft = board.Get(initialWhite).GetSize() - region.GetSquareCount();
			
			region.ForEach([&pt, &isReachable, regionSquaresLeft](const Point& ptInner, Square& sq)
			{
				if (Rules::CanReach(ptInner, pt, regionSquaresLeft))
					isReachable = true;
				return !isReachable;
			});
		}

		if (!isReachable)
			sq.SetState(SquareState::Black);

		return true;
	});
}

void Solver::SolveGuess()
{
	// out of multiple possible continuations
	// pick one and go with it.

}

bool Solver::Solve()
{
	SolveInitial();
	board.Print(std::cout);

	int iteration = 0;
	int phase = 0;
	bool hasChangedInPrevLoop = true;
	while (!Rules::IsSolved(board))
	{
		Board boardIterationStart = board;

		int phasePrev = phase;
		int iterationPrev = iteration;
		iteration++;

		if (phase == 0)
		{
			if (hasChangedInPrevLoop)
			{
				SolvePerSquare();
				SolvePerUnsolvedWhite();
				SolveUnreachable();
			}
			else
			{
				phase++;
				iteration = 0;
			}
		}
		else
		{
			break;
		}

		std::cout << "Phase #" << phasePrev << " - Iteration #" << iterationPrev << std::endl;
		board.Print(std::cout);

		hasChangedInPrevLoop = (board != boardIterationStart);
	}
	return true;
}