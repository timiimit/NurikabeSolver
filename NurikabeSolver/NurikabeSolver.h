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

		std::vector<Solver> solverStack;
		int iteration;

	public:
		Square GetInitialWhite(int initialWhiteIndex);

		const Board& GetBoard() const { return board; }
		Board& GetBoard() { return board; }

		int GetIteration() const { return iteration; }

	public:
		Solver(const Board& initialBoard);
		Solver(const Solver& other);

	private:
		void Initialize();

	private:
		bool SolveInflateTrivial(SquareState state);

		bool SolvePerSquare();

		/// @brief Solves squares that cannot be reached by any white
		void SolveUnreachable();

		/// @brief Solves standalone islands by finding which squares are forced to white
		bool SolveUnfinishedWhiteIsland();

		/// @brief Solves whites which expand and then guaranteed contract into a single line
		bool SolveBalloonWhite();

        bool SolveBalloonUnconnectedWhite();

        bool SolveBalloonWhiteSimple();

        int SolveBalloonWhiteSimpleSingle(Point pt);

		bool SolveBalloonWhiteFillSpaceCompletely();

        bool SolveBlackAroundWhite();
		bool SolveHighLevelRecursive(bool allowRecursion);

        bool SolveWhiteAtPredictableCorner();

        void SolveBalloonBlack();

		void SolveBlackInCorneredWhite2By3();

		void SolveDisjointedBlack();

	public:
		struct Evaluation
		{
			bool existsBlackRegion = false;
			bool existsMoreThanOneBlackRegion = false;
			bool existsClosedBlack = false;
			bool existsBlack2x2 = false;

			bool existsUnknownRegion = false;

			bool existsUnconnectedWhite = false;
			bool existsTooLargeWhite = false;
			//bool existsWhiteTouchingAnother = false;

			bool IsSolved() const
			{
				bool common = !existsUnknownRegion && !existsUnconnectedWhite && !existsTooLargeWhite;
				
				if (!existsBlackRegion)
					return common;

				return !existsMoreThanOneBlackRegion && existsClosedBlack && !existsBlack2x2 && common;
			}

			bool IsSolvable() const
			{
				if (IsSolved())
					return true;

				return
					!existsClosedBlack && !existsBlack2x2 &&
					!existsTooLargeWhite;
			}
		};

		Evaluation Evaluate();

		/// @brief Solves black+unknown neighbour when there is only 1 way out from unknown region. This is not a guaranteed working rule.
		bool SolveGuessBlackToUnblock(int minSize);

	private:
		/// @brief Removes any solved white that is still in @p unsolvedWhites .
		bool CheckForSolvedWhites();

	private:
		static bool SolveDivergeBlack(Solver& solver, std::vector<Solver>& solverStack, int maxDiverges, float blackToUnknownRatio);
		static bool SolveDivergeWhite(Solver& solver, std::vector<Solver>& solverStack, int maxDiverges, int maxSizeOfWhiteToDiverge);
		static void SolveDiverge(Solver& solver, std::vector<Solver>& solverStack);

		
		void ForEachRegion(const RegionDelegate& callback);

	public:
		int SolvePhase(int phase, bool allowRecursion);
		bool SolveWithRules(bool allowRecursion);
		bool Solve();
	};
}