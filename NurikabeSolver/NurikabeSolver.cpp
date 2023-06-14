#include "NurikabeSolver.h"
#include <iostream>
#include <assert.h>
#include <cmath>

using namespace Nurikabe;


Square Solver::GetInitialWhite(int initialWhiteIndex)
{
	return board.Get(initialWhites[initialWhiteIndex]);
}

static int nextSolverID = 0;

Solver::Solver(const Board& initialBoard, int* iteration)
	: board(initialBoard)
	, iteration(iteration)
	, depth(0)
	, id(nextSolverID++)
	, focusPointX(0)
	, focusPointY(0)
	, focusPointIteration(-1)
{
	Initialize();
}

Solver::Solver(const Solver& other)
	: board(other.board)
	, initialWhites(other.initialWhites)
	, unsolvedWhites(other.unsolvedWhites)
	, startOfUnconnectedWhite(other.startOfUnconnectedWhite)

	//, solverStack(other.solverStack)
	//, solutions(other.solutions)

	, iteration(other.iteration)
	, depth(other.depth)
	, id(nextSolverID++)

	// We want to carry focus
	, focusPointX(other.focusPointX)
	, focusPointY(other.focusPointY)
	, focusPointIteration(other.focusPointIteration)
{
}

Solver& Solver::operator=(const Solver& other)
{
	board = other.board;
	initialWhites = other.initialWhites;
	unsolvedWhites = other.unsolvedWhites;
	startOfUnconnectedWhite = other.startOfUnconnectedWhite;

	solverStack = other.solverStack;
	solutions = other.solutions;

	iteration = other.iteration;
	depth = other.depth;
	id = other.id;

	focusPointX = other.focusPointX;
	focusPointY = other.focusPointY;
	focusPointIteration = other.focusPointIteration;

	return *this;
}

void Solver::Initialize()
{
	board.ForEachSquare([this](const Point& pt, const Square& square)
		{
			if (square.GetSize() != 0)
			{
				// determine this white's ID
				board.SetOrigin(pt, initialWhites.size());

				// add this white to our lists
				unsolvedWhites.push_back((int)initialWhites.size());
				initialWhites.push_back(pt);
			}

			return true;
		});
}

void Solver::UpdateContiguousRegions()
{
	// TODO: can be further optimized by updating only relevant regions instead of full rebuild
	contiguousRegions.clear();

	Region handled(&board);

	board.ForEachSquare([this, &handled](const Point& pt, const Square& sq)
	{
		if (handled.Contains(pt))
			return true;

		Region region = Region(&board, pt).ExpandAllInline(
			[&sq](const Point&, const Square& sqInner) { return sqInner.GetState() == sq.GetState(); }
		);

		handled = Region::Union(handled, region);
		contiguousRegions.push_back(region);

		return true;
	});
}

void Solver::ForEachRegion(const std::function<bool(const Region &)>& callback)
{
	for (int i = 0; i < contiguousRegions.size(); i++)
	{
		if (!callback(contiguousRegions[i]))
			break;
	}
}

bool Solver::SolveHighLevelRecursive(const SolveSettings& settings)
{
	if (settings.maxDepth == 0)
		return true;

	// find optimal ("human choice") region which should
	// be searched for good options.

	Region best;
	double bestFactor = 0.0;
	double bestFocusX = 0.0;
	double bestFocusY = 0.0;

	ForEachRegion([this, &best, &bestFactor, &bestFocusX, &bestFocusY](const Region& r)
	{
		double focusX;
		double focusY;
		double factor = EstimateRegionSearchQuality(r, focusX, focusY);
		if (factor != INFINITY)
		{
			if (best.GetSquareCount() == 0 || factor < bestFactor)
			{
				best = r;
				bestFactor = factor;
				bestFocusX = focusX;
				bestFocusY = focusY;
			}
		}

		return true;
	});

	if (best.GetSquareCount() < 1)
		return true;

	// Update focus point to approach active part of the board
	if (focusPointIteration >= 0)
	{
		focusPointX = (focusPointX + bestFocusX) / 2.0;
		focusPointY = (focusPointY + bestFocusY) / 2.0;
	}
	else
	{
		focusPointX = bestFocusX;
		focusPointY = bestFocusY;
	}
	focusPointIteration = *iteration;

	// optimal region was chosen. do a breadth search to narrow down possibilities, by eliminating unsolvable paths.
	return SolveHighLevelRecursive(settings, best);
}

bool Nurikabe::Solver::SolveHighLevelRecursive(const SolveSettings& settings, const Region& searchRegion)
{
	auto state = searchRegion.GetState();
	assert(state == SquareState::Black || state == SquareState::White);

	Region unknown = searchRegion.Neighbours(SquareState::Unknown);

	int solvableFound = 0;

	unknown.ForEach([this, &settings, &solvableFound, &unknown, &state](const Point& pt, const Square& sq)
	{
		//if (solvableFound > 1)
		//	return false;

		// Do a breadth search over all possible placements of black squares
		Solver solver = Solver(*this);
		solver.depth++;

		if (state == SquareState::Black)
			solver.board.SetBlack(pt);
		else
			solver.board.SetWhite(pt);

		solver.board.Print(std::cout);
		std::cout << "Depth: " << solver.depth << std::endl;
		std::cout << "Iteration: " << *solver.iteration << std::endl;
		
		auto settingsNext = settings.Next();
		settingsNext.maxDepth = 0;
		if (!solver.SolveWithRules(settingsNext))
			return true;

		solver.board.Print(std::cout);
		std::cout << "Depth: " << solver.depth << std::endl;
		std::cout << "Iteration: " << *solver.iteration << std::endl;

		auto eval = solver.Evaluate();

		if (eval.IsSolved())
		{
			solvableFound = 1;
			solverStack.clear();
			solverStack.push_back(solver);
			return false;
		}

		if (solver.solverStack.size() > 0)
		{
			solvableFound += solver.solverStack.size();
			solverStack.insert(solverStack.end(), solver.solverStack.begin(), solver.solverStack.end());
			solver.solverStack.clear();
		}

		if (eval.IsSolvable())
		{
			if (solver.GetBoard() != GetBoard())
			{
				solvableFound++;
				solverStack.push_back(solver);
			}
		}

		// if (solvableFound > 0)
		// {

		// 	solver.board.Print(std::cout);
		// 	std::cout << "Depth: " << depth << std::endl;
		// 	std::cout << "Iteration: " << *iteration << std::endl;

		// 	int depthCount = (int)std::round(eval.progress * 3.0);

		// 	for (int depthPass = 0; depthPass < depthCount; depthPass++)
		// 	{
		// 		int initialStackSize = solverStack.size();
		// 		for (int i = 0; i < initialStackSize; i++)
		// 		{
		// 			Solver& innerSolver = solverStack[i];

		// 			solver.board.Print(std::cout);
		// 			std::cout << "Depth: " << depth << std::endl;
		// 			std::cout << "Iteration: " << *iteration << std::endl;

		// 			bool isSolvable = innerSolver.SolveHighLevelRecursive(true);

		// 			solver.board.Print(std::cout);
		// 			std::cout << "Depth: " << depth << std::endl;
		// 			std::cout << "Iteration: " << *iteration << std::endl;

		// 			if (isSolvable)
		// 			{
		// 				auto eval = innerSolver.Evaluate();
		// 				if (eval.IsSolved())
		// 				{
		// 					Solver s = innerSolver;
		// 					solvableFound = 1;
		// 					solverStack.clear();
		// 					solverStack.push_back(s);
		// 					return false;
		// 				}

		// 				if (!eval.IsSolvable())
		// 				{
		// 					isSolvable = false;
		// 				}
		// 			}

		// 			if (!isSolvable)
		// 			{
		// 				solverStack.erase(solverStack.begin() + i);
		// 				i--;
		// 				continue;
		// 			}

		// 			for (int j = 0; j < innerSolver.solverStack.size(); j++)
		// 			{
		// 				solverStack.push_back(innerSolver.solverStack[j]);
		// 			}
		// 		}
		// 	}
		// }

		return true;
	});

	if (solvableFound == 0)
	{
		// Since black is guaranteed to be contiguous, there needs to be a solution among chosen paths
		return false;
	}

	if (solvableFound == 1)
	{
		*this = this->solverStack.back();
		depth--;
	}

	return true;
}

bool Solver::SolveWhiteAtPredictableCorner(const SolveSettings& settings)
{
	if (settings.maxDepth == 0)
		return true;

	bool ret = true;
	ForEachRegion([this, &settings, &ret](const Region& black)
	{
		if (black.GetState() != SquareState::Black)
			return true;
		
		auto unknowns = black.Neighbours(SquareState::Unknown);
		
		unknowns.ForEachContiguousRegion([this, &settings, &ret, &black](const Region& unknown)
		{
			if (unknown.GetSquareCount() != 2)
				return true;

			auto blackEntry = Region::Intersection(unknown.Neighbours(SquareState::Black), black);
			if (blackEntry.GetSquareCount() != 2)
				return true;

			auto whiteEntry = unknown.Neighbours(SquareState::White);
			if (whiteEntry.GetSquareCount() != 1)
				return true;

			auto blackWhiteCommonCorner = Region::Subtract(
				Region::Intersection(
					whiteEntry.Neighbours(),
					blackEntry.Neighbours()
				),
				unknown
			);
			if (blackWhiteCommonCorner.GetSquareCount() != 1)
				return true;

			auto whiteNew = Region::Intersection(unknown, whiteEntry.Neighbours());
			if (whiteNew.GetSquareCount() != 1)
				return true;

			// this rule works most of the time, but not always
			Solver solverCopy = Solver(*this);
			solverCopy.depth++;

			Region((Board*)&solverCopy.GetBoard(), whiteNew.GetSquares()[0]).SetState(SquareState::White);
			if (!solverCopy.CheckForSolvedWhites())
				return true;

			// because we are often confident that this path is the right one,
			// we try to solve it completely so we either succeed or find out
			// that this option was actually wrong.

			if (solverCopy.Solve(SolveSettings()))
			{
				*this = solverCopy;
				depth--;
			}
			else
			{
				// because we proved that this board is not solvable,
				// we can guarantee that `whiteNew` must be black.

				whiteNew.SetState(SquareState::Black);
			}
			
			return false;
		});

		return true;
	});
	return ret;
}

double Nurikabe::Solver::EstimateRegionSearchQuality(const Region& region, double& xCenter, double& yCenter)
{
	if (region.GetState() != SquareState::Black && region.GetState() != SquareState::White)
		return INFINITY;

	if (region.GetSquares()[0]== Point{0,13})
	{
		int a = 0;
	}

	double currentFactor = 0.0;

	if (region.GetState() == SquareState::White)
	{
		// Temporarily don't consider white
		return INFINITY;

		if (region.GetSameOrigin() == (uint8_t)~0)
		{
			// Let's rather not... Too many possibilities
			return INFINITY;
		}

		// We want to consider regions with as little remaining squares as possible
		int squaresRemaining = region.GetSameSize() - region.GetSquareCount();

		// No point in selecting solved white
		if (squaresRemaining == 0)
			return INFINITY;

		currentFactor += 0.2 * std::pow(squaresRemaining, 2);

		// // We want black regions to have some priority
		//currentFactor += 0.1;
	}
	else
	{
		// currentFactor += 0.5 * borders.GetSquareCount();

		// We want to give priority to regions of around 10 squares
		//currentFactor += std::pow(10 - region.GetSquareCount(), 1.0) * 0.1;
		currentFactor -= 0.2 * std::log(region.GetSquareCount());
	}

	auto continuations = region.Neighbours(SquareState::Unknown);
	auto borders = Region::Intersection(continuations.Neighbours(), region);

	xCenter = 0.0;
	yCenter = 0.0;
	borders.ForEach([&xCenter, &yCenter](const Point& pt, const Square&)
	{
		xCenter += pt.x;
		yCenter += pt.y;
		return true;
	});
	xCenter /= borders.GetSquareCount();
	yCenter /= borders.GetSquareCount();

	double borderDistances = 0;
	double focusDistances = 0;
	borders.ForEach([this, &xCenter, &yCenter, &borderDistances, &focusDistances](const Point& pt, const Square&)
	{
		double x = std::abs(xCenter - pt.x);
		double y = std::abs(yCenter - pt.y);
		borderDistances += std::sqrt(x * x + y * y);

		x = std::abs(focusPointX - pt.x);
		y = std::abs(focusPointY - pt.y);
		focusDistances += std::sqrt(x * x + y * y);

		return true;
	});
	focusDistances /= 5.0;

	if (focusPointIteration >= 0)
	{
		// We want to keep context of what we are trying to solve currently.
		// The more iterations ago we last used recursion, the less should
		// focus have an effect.
		//currentFactor += 0.5 * (focusDistances / focusPointIteration);
	}

	// We want as little continuations as possible
	currentFactor += 0.2 * continuations.GetSquareCount();

	if (region.GetState() == SquareState::Black)
	{
		// We want bordering squares to be as close as possible
		//currentFactor += 0.5 * borderDistances;
	}

    return currentFactor;
}

Solver::Evaluation Solver::Evaluate()
{
	Evaluation eval;

	eval.existsBlack2x2 = Rules::ContainsBlack2x2(board);

	eval.progress = 0;

	ForEachRegion([&eval](const Region& r)
	{
		if (r.GetState() == SquareState::Black)
		{
			if (eval.existsBlackRegion)
				eval.existsMoreThanOneBlackRegion = true;
			eval.existsBlackRegion = true;

			if (!eval.existsClosedBlack)
			{
				bool isClosed = true;
				r.Neighbours([&isClosed](const Point&, const Square& sq)
				{
					if (sq.GetState() != SquareState::White)
					{
						isClosed = false;
						return false;
					}
					return true;
				});

				if (isClosed)
					eval.existsClosedBlack = true;
			}
		}
		else if (r.GetState() == SquareState::White)
		{
			if (r.GetSameOrigin() == (uint8_t)~0)
			{
				eval.existsUnconnectedWhite = true;
				if (!eval.existsUnconnectedClosedWhite)
				{
					bool isClosed = true;
					r.Neighbours([&isClosed](const Point&, const Square& sq)
					{
						if (sq.GetState() != SquareState::Black)
						{
							isClosed = false;
							return false;
						}
						return true;
					});

					if (isClosed)
						eval.existsUnconnectedClosedWhite = true;
				}
			}
			else
			{
				if (r.GetSquareCount() > r.GetSameSize())
				{
					eval.existsTooLargeWhite = true;
				}
			}
		}
		else if (r.GetState() == SquareState::Unknown)
		{
			eval.progress += r.GetSquareCount();
			eval.existsUnknownRegion = true;
		}
		return true;
	});

	eval.progress = 1 - (eval.progress / (board.GetWidth() * board.GetHeight()));

	return eval;
}

int Solver::SolvePhase(int phase, const SolveSettings& settings)
{
	std::function<bool()> phases[] =
	{
		[this](){ return SolveInflateTrivial(SquareState::White); },
		[this](){ return SolveInflateTrivial(SquareState::Black); },
		[this](){ return SolvePerSquare(); },
		[this](){ SolveUnreachable(); return true; },
		[this](){ return SolveBalloonWhiteFillSpaceCompletely(); },
		[this](){ SolveDisjointedBlack(); return true; },
		[this](){ SolveBlackInCorneredWhite2By3(); return true; },
		[this](){ return SolveBalloonWhiteSimple(); },
		//[this](){ return SolveBalloonBlack(); },
		//[this](){ return SolveUnconnectedWhiteHasOnlyOnePossibleOrigin(); },
		[this, settings](){ return SolveWhiteAtPredictableCorner(settings); },
		[this, settings](){ return SolveHighLevelRecursive(settings); },
	};

	if (phase >= sizeof(phases) / sizeof(*phases))
		return -1;

	bool ret = phases[phase]();

	if (ret)
		return 1;

	return 0;
}

bool Solver::SolveWithRules(const SolveSettings& settings)
{
	int phase = 0;
	bool hasChangedInPrevLoop = true;

	const int checkFrequency = 1;
	int iterationNextCheck = *iteration + checkFrequency;

	UpdateContiguousRegions();

	while (true)
	{
		Board boardIterationStart = board;

		int ret = SolvePhase(phase, settings);

		if (ret < 0)
			break;

		if (ret == 0)
			return false;

		hasChangedInPrevLoop = (board != boardIterationStart);
		if (!hasChangedInPrevLoop)
		{
			(*iteration)++;
			phase++;
			continue;
		}
		
		UpdateContiguousRegions();

		board.Print(std::cout);
		std::cout << "Depth: " << depth << std::endl;
		std::cout << "Iteration: " << *iteration << std::endl;


		(*iteration)++;
		phase = 0;

		if (*iteration >= iterationNextCheck)
		{
			// std::cout << "Iteration #" << *iteration << std::endl;
			// board.Print(std::cout);

			iterationNextCheck = *iteration + checkFrequency;
			auto eval = Evaluate();
			if (eval.IsSolved())
				break;
			if (!eval.IsSolvable())
				break;
		}

		if (settings.stopAtIteration >= 0 && *iteration >= settings.stopAtIteration)
			break;
	}

	return true;
}

bool Solver::Solve(const SolveSettings& settings)
{
	if (settings.maxDepth == 0)
		return true;

	if (!CheckForSolvedWhites())
		return false;

	solverStack.clear();
	solverStack.push_back(*this);

	while (true)
	{
		if (solverStack.size() == 0)
			return false;

		int	solverIndex = solverStack.size() - 1;

		Solver solver = solverStack[solverIndex];
		solverStack.erase(solverStack.begin() + solverIndex);

		if (!solver.SolveWithRules(settings))
			continue;

		auto eval = solver.Evaluate();

		if (eval.IsSolved())
		{
			board = solver.board;
			solverStack.clear();
			break;
		}

		// std::cout << "Iteration #" << solver.GetIteration() << std::endl;
		// solver.board.Print(std::cout);
		// std::cout << std::endl;
		
		// if (!eval.IsSolvable())
		// 	continue;

		for (int i = 0; i < solver.solverStack.size(); i++)
		{
			solverStack.push_back(solver.solverStack[i]);
		}
	}

	return true;
}