#pragma once
#include "NurikabeRules.h"
#include "NurikabeBoard.h"
#include <vector>
#include <stack>

namespace Nurikabe
{
	class Solver
	{
		// current state of the board we are trying to solve
		Board board;

		// flags of current state of the board where isBlack means isFinal
		// and size means which startingWhite current square is a part of
		//Board flags;

		std::vector<Point> initialWhites;
		std::vector<int> unsolvedWhites;

	public:
		Square GetInitialWhite(int initialWhiteIndex);

	public:
		Solver(const Board& initialBoard);
		Solver(const Solver& other);

	private:
		void Initialize();

	private:
		void SolvePerSquare();
		void CheckForSolvedWhites();
		void SolveUnreachable();

	private:
		static void SolveDiverge(Solver& solver, std::stack<Solver>& solverStack);
		static int SolveWithRules(Solver& solver);

	public:
		static bool Solve(Solver& solver);
	};
}