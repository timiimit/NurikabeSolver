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
		std::vector<Region> contiguousRegions;

		std::vector<Solver> solverStack;
		std::vector<Solver> solutions;
		int* iteration;
		int depth;
		int id;

	public:
		struct SolveSettings
		{
			int maxDepth = -1;
			int stopAtIteration = -1;
			bool stopAtFirstSolution = true;

			SolveSettings Next() const
			{
				SolveSettings ret = *this;
				if (ret.maxDepth > 0)
					ret.maxDepth--;
				return ret;
			}

			static SolveSettings StopAtIteration(int iteration)
			{
				SolveSettings ret;
				ret.stopAtIteration = iteration;
				return ret;
			}

			static SolveSettings FindAllSolutions()
			{
				SolveSettings ret;
				ret.stopAtFirstSolution = false;
				return ret;
			}

			static SolveSettings NoRecursion()
			{
				SolveSettings ret;
				ret.maxDepth = 0;
				return ret;
			}
		};

	public:
		struct Evaluation
		{
			bool existsBlackRegion = false;
			bool existsMoreThanOneBlackRegion = false;
			bool existsClosedBlack = false;
			bool existsBlack2x2 = false;

			bool existsUnknownRegion = false;

			bool existsUnconnectedWhite = false;
			bool existsUnconnectedClosedWhite = false;
			bool existsTooLargeWhite = false;
			double progress = 0.0;
			//bool existsWhiteTouchingAnother = false;

			bool IsSolved() const
			{
				bool common = !existsUnknownRegion && !existsUnconnectedWhite && !existsTooLargeWhite && !existsUnconnectedClosedWhite && progress == 1.0;
				
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
					!existsTooLargeWhite && !existsUnconnectedClosedWhite;
			}
		};

		Evaluation Evaluate();

	public:
		Square GetInitialWhite(int initialWhiteIndex);

		const Board& GetBoard() const { return board; }
		Board& GetBoard() { return board; }

		int GetIteration() const { return *iteration; }

	public:
		Solver(const Board& initialBoard, int* iteration);
		Solver(const Solver& other);

		Solver& operator=(const Solver& other);

	private:
		void Initialize();

		void UpdateContiguousRegions();
		
		void ForEachRegion(const RegionDelegate& callback);

	private:

		bool SolveInflateTrivial(SquareState state);

		bool SolvePerSquare();

		bool SolvePerUnknownSquare();

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

		bool SolveUnconnectedWhiteHasOnlyOnePossibleOrigin();

        void SolveBalloonBlack();

		void SolveBlackInCorneredWhite2By3();

		void SolveDisjointedBlack();

		/// @brief Solves black+unknown neighbour when there is only 1 way out from unknown region. This is not a guaranteed working rule.
		bool SolveGuessBlackToUnblock(int minSize);

	private:
		/// @brief Removes any solved white that is still in @p unsolvedWhites .
		bool CheckForSolvedWhites();

	private:

        bool SolveWhiteAtPredictableCorner(const SolveSettings& settings);
		bool SolveHighLevelRecursive(const SolveSettings& settings);

		int SolvePhase(int phase, const SolveSettings& settings);
		bool SolveWithRules(const SolveSettings& settings);

	public:
		bool Solve(const SolveSettings& settings = SolveSettings{-1, -1, true});
		
	};
}