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
		std::vector<Point> startOfUnconnectedWhite;

	public:
		Square GetInitialWhite(int initialWhiteIndex);
		const Board& GetBoard() const { return board; }
		Board& GetBoard() { return board; }

	public:
		Solver(const Board& initialBoard);
		Solver(const Solver& other);

	private:
		void Initialize();

	private:
		/// @brief Solves with narrow-sighted, simple per square rules
		bool SolvePerSquare();

		/// @brief Solves squares that cannot be reached by any white
		void SolveUnreachable();

		/// @brief Solves standalone islands by finding which squares are forced to white
		bool SolveUnfinishedWhiteIsland();

		/// @brief Solves whites which expand and then guaranteed contract into a single line
		bool SolveBalloonWhite();

        bool SolveBalloonUnconnectedWhite();

        bool SolveBalloonUnconnectedWhiteSimple();

        bool SolveWhiteAtClosedBlack();
        bool SolveBlackAtPredictableCorner();

        void SolveBalloonBlack();

		/// @brief Solves black+unknown neighbour when there is only 1 way out from unknown region. This is not a guaranteed working rule.
		bool SolveGuessBlackToUnblock(int minSize);

	private:
		/// @brief Removes any solved white that is still in @p unsolvedWhites .
		bool CheckForSolvedWhites();

	private:
		static bool SolveDivergeBlack(Solver& solver, std::vector<Solver>& solverStack, int maxDiverges, float blackToUnknownRatio);
		static bool SolveDivergeWhite(Solver& solver, std::vector<Solver>& solverStack, int maxDiverges, int maxSizeOfWhiteToDiverge);
		static void SolveDiverge(Solver& solver, std::vector<Solver>& solverStack);
		static int SolveWithRules(Solver& solver);

		
		void ForEachRegion(const RegionDelegate& callback);

	public:
		static bool Solve(Solver& solver);
	};
}