#include "NurikabeSolver.h"
#include <iostream>
#include <assert.h>

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

void Solver::Initialize()
{
	board.ForEachSquare([this](const Point& pt, Square& square)
		{
			if (square.GetSize() != 0)
			{
				// determine this white's ID
				board.Get(pt).SetOrigin((uint8_t)initialWhites.size());

				// add this white to our lists
				unsolvedWhites.push_back((int)initialWhites.size());
				initialWhites.push_back(pt);
			}

			return true;
		});
}

void Solver::SolvePerSquare()
{
	board.ForEachSquare([this](const Point& pt, Square& square)
		{
			if (pt == Point{3, 1})
			{
				int a = 0;
			}

			if (square.GetState() == SquareState::Unknown)
			{
				Region whiteNeighbours = Region(&board, pt)
					.Neighbours([](const Point& pt, Square& square) {
						return square.GetState() == SquareState::White;
					});

				if (whiteNeighbours.GetSquareCount() > 1 && !whiteNeighbours.IsSameOrigin())
				{
					board.SetBlack(pt);
					return true;
				}


				// Region diagonals = Region(&board, pt)
				// 	.Neighbours([](const Point& pt, Square& square) {
				// 		return square.GetState() == SquareState::Black;
				// 	})
				// 	.Neighbours([&pt](const Point& ptDiag, Square& square) {
				// 		if (Distance(ptDiag.x, pt.x) > 1)
				// 			return false;

				// 		return square.GetState() == SquareState::Black;
				// 	});

				// diagonals.ForEach([this, &pt](const Point& ptDiag, Square& square) {
				// 	Point delta = pt - ptDiag;
				// 	if (board.IsBlack(Point{ ptDiag.x + delta.x, ptDiag.y }) &&
				// 		board.IsBlack(Point{ ptDiag.x, ptDiag.y + delta.y }))
				// 	{
				// 		board.SetWhite(pt);
				// 		CheckForSolvedWhites();
				// 		return false;
				// 	}
				// 	return true;
				// });
				
				// if (square.GetState() != SquareState::Unknown)
				// 	return true;
			}
			else
			{
				Region rColor = Region(&board, pt)
					.ExpandAllInline([square](const Point&, Square& squareInner) {
						return squareInner.GetState() == square.GetState();
					});
					
				auto rColorSize = rColor.GetSquareCount();

				Region rColorContinuations = rColor.Neighbours([](const Point&, Square& squareInner) {
						return squareInner.GetState() == SquareState::Unknown;
					});

				if (rColorContinuations.GetSquareCount() == 1)
				{
					// If there's only one way to go, then go there

					rColorContinuations.SetState(square.GetState());
					if (square.GetState() == SquareState::White)
					{
						rColorContinuations.SetSize(square.GetSize());
						assert(square.GetOrigin() != 127);
						rColorContinuations.SetOrigin(square.GetOrigin());
						CheckForSolvedWhites();
					}
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
								return true;
							}
						}
					}
				}
			}

			return true;
		});
}

void Solver::CheckForSolvedWhites()
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

void Solver::SolveDiverge(Solver& solver, std::stack<Solver>& solverStack)
{
	std::vector<Region> unsolvedWhiteRegions;

	for (int i = 0; i < solver.unsolvedWhites.size(); i++)
	{
		unsolvedWhiteRegions.push_back(
			Region(&solver.board, solver.initialWhites[solver.unsolvedWhites[i]])
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
		auto solverCopy = Solver(solver);
		auto& sq = solverCopy.board.Get(bestRegion.GetSquares()[i]);
		sq.SetState(SquareState::White);
		sq.SetSize(solver.board.GetRequiredSize(solver.initialWhites[solver.unsolvedWhites[min]]));
		sq.SetOrigin(solver.unsolvedWhites[min]);
		solverStack.push(std::move(solverCopy));
	}
}

int Solver::SolveWithRules(Solver& solver)
{
	int iteration = 0;
	int phase = 0;
	bool hasChangedInPrevLoop = true;

	while (!Rules::IsSolved(solver.board))
	{
		Board boardIterationStart = solver.board;

		int phasePrev = phase;
		int iterationPrev = iteration;
		iteration++;

		if (phase == 0)
		{
			solver.SolvePerSquare();
			solver.board.Print(std::cout);
		}
		else if (phase == 1)
		{
			solver.SolveUnreachable();
			solver.board.Print(std::cout);
		}
		else
		{
			break;
		}

		hasChangedInPrevLoop = (solver.board != boardIterationStart);
		if (!hasChangedInPrevLoop)
			phase++;
		else
			phase = 0;
	}

	return iteration;
}

bool Solver::Solve(Solver& initialSolver)
{
	initialSolver.Initialize();

	std::stack<Solver> solverStack;
	solverStack.push(initialSolver);

	int iteration = 0;
	while (true)
	{
		if (solverStack.size() == 0)
		{
			std::cout << "Unable to solve provided board" << std::endl;
			return false;
		}

		Solver solver = std::move(solverStack.top());
		solverStack.pop();

		std::cout << "Iteration #" << iteration << std::endl;
		solver.board.Print(std::cout);
		std::cout << std::endl;

		iteration += SolveWithRules(solver);

		if (Rules::IsSolved(solver.board))
		{
			std::cout << std::endl;
			std::cout << "   --- Starting board ---   " << std::endl;
			initialSolver.board.Print(std::cout);
			std::cout << "   ---  Solved board  ---   " << std::endl;
			solver.board.Print(std::cout);
			std::cout << std::endl;
			break;
		}
		
		if (!Rules::IsSolvable(solver.board) || !solver.unsolvedWhites.size())
		{
			std::cout << "Iteration #" << iteration << std::endl;
			solver.board.Print(std::cout);
			std::cout << std::endl;
			continue;
		}

		SolveDiverge(solver, solverStack);
	}
	return true;
}