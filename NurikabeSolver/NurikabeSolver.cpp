#include "NurikabeSolver.h"
#include <iostream>
#include <queue>

using namespace Nurikabe;


Square Solver::GetInitialWhite(int initialWhiteIndex)
{
	return board.Get(initialWhites[initialWhiteIndex]);
}

Solver::Solver(const Board& initialBoard)
	: board(initialBoard)
{

}

Solver::Solver(const Solver& other)
	: board(other.board)
	, initialWhites(other.initialWhites)
	, unsolvedWhites(other.unsolvedWhites)
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
				Region rColorContinuations = Region(&board, pt)
					.ExpandAllInline([square](const Point&, Square& squareInner) {
						return squareInner.GetState() == square.GetState();
					});
					
				auto rColorSize = rColorContinuations.GetSquareCount();

				rColorContinuations = rColorContinuations.Neighbours([](const Point&, Square& squareInner) {
						return squareInner.GetState() == SquareState::Unknown;
					});

				if (rColorContinuations.GetSquareCount() == 1)
				{
					// If there's only one way to go, then go there

					rColorContinuations.SetState(square.GetState());
					if (square.GetState() == SquareState::White)
						rColorContinuations.SetSize(square.GetSize());
				}
				else if (rColorContinuations.GetSquareCount() == 2)
				{
					if (square.GetState() == SquareState::White && square.GetSize() - rColorSize == 1)
					{
						// Solve black on corner of white that's missing only 1 more square

						Region rSquareContinuations = Region(&board, pt).Neighbours([](const Point&, Square& squareInner) {
							return squareInner.GetState() == SquareState::Unknown;
						});
						if (rSquareContinuations.GetSquareCount() == 2)
						{
							auto rSquareContinuationsDiagonal = rSquareContinuations.Neighbours([&pt](const Point& ptInner, Square& squareInner) {
								if (Distance(ptInner.x, pt.x) != 1 || Distance(ptInner.y, pt.y) != 1)
									return false;
								return squareInner.GetState() == SquareState::Unknown;
							});

							if (rSquareContinuationsDiagonal.GetSquareCount() == 1)
							{
								rSquareContinuationsDiagonal.SetState(SquareState::Black);
							}
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

bool Solver::Solve(Solver& solver)
{
	solver.SolveInitial();
	solver.board.Print(std::cout);

	std::queue<Solver> solverStack;
	solverStack.push(solver);

	int iteration = 0;
	while (true)
	{
		int phase = 0;
		bool hasChangedInPrevLoop = true;

		if (solverStack.size() == 0)
		{
			std::cout << "Unable to solve provided board" << std::endl;
			return false;
		}

		Solver currentSolver = solverStack.front();
		solverStack.pop();

		while (!Rules::IsSolved(currentSolver.board))
		{
			Board boardIterationStart = currentSolver.board;

			int phasePrev = phase;
			int iterationPrev = iteration;
			iteration++;

			if (phase == 0)
			{
				currentSolver.SolvePerSquare();
				currentSolver.board.Print(std::cout);
				currentSolver.SolvePerUnsolvedWhite();
				currentSolver.board.Print(std::cout);
			}
			else if (phase == 1)
			{
				currentSolver.SolveUnreachable();
				currentSolver.board.Print(std::cout);
			}
			else
			{
				break;
			}

			std::cout << "Phase #" << phasePrev << " - Iteration #" << iterationPrev << std::endl;
			currentSolver.board.Print(std::cout);

			hasChangedInPrevLoop = (currentSolver.board != boardIterationStart);
			if (!hasChangedInPrevLoop)
				phase++;
			else
				phase = 0;
		}

		if (Rules::IsSolved(currentSolver.board))
		{
			std::cout << "   --- Starting board ---   " << std::endl;
			solver.board.Print(std::cout);
			std::cout << "   ---  Solved board  ---   " << std::endl;
			currentSolver.board.Print(std::cout);
			break;
		}
		else if (currentSolver.unsolvedWhites.size())
		{
			// Push boards with multiple continuation options to stack and continue trying to solve those

			std::vector<Region> unsolvedWhiteRegions;

			for (int i = 0; i < currentSolver.unsolvedWhites.size(); i++)
			{
				unsolvedWhiteRegions.push_back(
					Region(&currentSolver.board, currentSolver.initialWhites[i])
					.ExpandAllInline([](const Point&, Square& sq){
						return sq.GetState() == SquareState::White;
					})
					.Neighbours([](const Point&, Square& sq){
						return sq.GetState() == SquareState::Unknown;
					})
				);
			}

			int min = 0;
			for (int i = 1; i < unsolvedWhiteRegions.size(); i++)
			{
				if (unsolvedWhiteRegions[i].GetSquareCount() < unsolvedWhiteRegions[min].GetSquareCount())
					min = i;
			}

			auto& bestRegion = unsolvedWhiteRegions[min];

			for (int i = 0; i < bestRegion.GetSquareCount(); i++)
			{
				auto solverCopy = Solver(currentSolver);
				solverCopy.board.SetWhite(bestRegion.GetSquares()[i]);
				solverCopy.board.SetSize(bestRegion.GetSquares()[i], currentSolver.board.GetRequiredSize(currentSolver.initialWhites[min]));
				solverStack.push(std::move(solverCopy));
			}
		}
	}
	return true;
}