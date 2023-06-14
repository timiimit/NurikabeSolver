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
{
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

	return *this;
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

bool Solver::SolveInflateTrivial(SquareState state)
{
	if (*iteration == 110)
	{
		int a = 0;
	}

	bool ret = true;
	ForEachRegion([this, &ret, state](const Region& r)
	{
		if (r.GetState() != state)
			return true;

		if (r.GetSquareCount() == 1 && r.GetSquares()[0] == Point{4,1})
		{
			int a = 0;
		}

		Square sq;
		if (!r.StartNeighbourSpill(sq))
			return true;

		Region rContinuations = r.NeighbourSpill(sq);

		if (rContinuations.GetSquareCount() == 1)
		{
			// If there's only one way to go, then go there

			rContinuations.SetState(state);

			if (state == SquareState::White)
			{
				if (!CheckForSolvedWhites())
				{
					ret = false;
					return false;
				}
			}
			return false;
		}

		return true;
	});
	return ret;
}

bool Solver::SolvePerSquare()
{
	bool ret = true;
	board.ForEachSquare([this, &ret](const Point& pt, Square& square)
		{
			if (pt == Point{5, 13})
			{
				int a = 0;
			}

			if (square.GetState() == SquareState::Unknown)
			{
				Region whiteNeighbours = Region(&board, pt)
					.Neighbours([](const Point& pt, Square& square) {
						return square.GetState() == SquareState::White;
					});


				bool isSameOrigin = true;
				uint8_t firstOrigin = (uint8_t)~0;
				uint8_t firstSize = 0;
				uint8_t longestUnconnectedWhite = 0;
				whiteNeighbours.ForEachContiguousRegion([&isSameOrigin, &firstOrigin, &firstSize, &longestUnconnectedWhite](const Region& r)
				{
					if (r.GetSameOrigin() == (uint8_t)~0)
					{
						auto len = Region(r).ExpandAllInline([&longestUnconnectedWhite](const Point&, const Square& sq){
							return sq.GetState() == SquareState::White;
						}).GetSquareCount();

						if (len > longestUnconnectedWhite)
							longestUnconnectedWhite = len;
					}
					else
					{
						if (firstOrigin == (uint8_t)~0)
						{
							firstOrigin = r.GetSameOrigin();
							firstSize = r.GetSameSize();
						}
						else if (firstOrigin != r.GetSameOrigin())
						{
							isSameOrigin = false;
							return false;
						}
					}

					return true;
				});

				if (isSameOrigin && firstOrigin != (uint8_t)~0 && longestUnconnectedWhite > firstSize)
				{
					isSameOrigin = false;
				}

				if (!isSameOrigin)
				{
					board.SetBlack(pt);
					return false;
				}

// NOTE: only used for testing, should be zero
#define HAVE_BROKEN_EDGE_DETECTION 0

#if !HAVE_BROKEN_EDGE_DETECTION
				bool areAllNeighboursBlack = true;
				Region(&board, pt).Neighbours([&areAllNeighboursBlack](const Point&, Square& square) {
					if (square.GetState() != SquareState::Black)
					{
						areAllNeighboursBlack = false;
						return false;
					}
					return true;
				});

				if (areAllNeighboursBlack)
				{
					board.SetBlack(pt);
					return false;
				}
#endif

				Region blackNeighbours = Region(&board, pt).Neighbours([](const Point& pt, Square& square) {
					return square.GetState() == SquareState::Black;
				});

#if HAVE_BROKEN_EDGE_DETECTION
				if (blackNeighbours.GetSquareCount() == 4)
				{
					board.SetBlack(pt);
					return false;
				}
#endif

				// handle case where there are 3 black and 1 unknown square in 2x2
				Region blackDiagonalNeighbours = blackNeighbours.Neighbours([&pt](const Point& ptInner, Square& square) {
					return
						Distance(pt.x, ptInner.x) == 1 &&
						Distance(pt.y, ptInner.y) == 1 &&
						square.GetState() == SquareState::Black;
				});

				blackDiagonalNeighbours.ForEach([&pt, &square, this](const Point& ptInner, Square&) {
					Point delta = pt - ptInner;
					if (board.IsBlack(Point{ ptInner.x + delta.x, ptInner.y }) &&
						board.IsBlack(Point{ ptInner.x, ptInner.y + delta.y }))
					{
						// we found a lone white square, we need to connect it logically
						square.SetState(SquareState::White);
						startOfUnconnectedWhite.push_back(pt);
						return false;
					}
					return true;
				});

				if (square.GetState() == SquareState::White)
				{
					if (pt == Point{4,10})
					{
						int a = 0;
					}
					if (!CheckForSolvedWhites())
					{
						ret = false;
						return false;
					}
					return false;
				}
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

				if (rColorContinuations.GetSquareCount() == 2)
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
								return false;
							}
						}
					}
				}
			}

			return true;
		});
	return ret;
}

bool Solver::CheckForSolvedWhites()
{
	for (int i = 0; i < unsolvedWhites.size(); i++)
	{
		Point pt = initialWhites[unsolvedWhites[i]];
		auto region = Region(&board, pt)
			.ExpandAllInline([](const Point& pt, Square& square)
				{
					return square.GetState() == SquareState::White;
				});
		region.FixWhites();

		if (region.GetSquareCount() == board.GetRequiredSize(pt))
		{
			// if region is of correct size, mark it as complete
			region
				.Neighbours()
				.SetState(SquareState::Black);

			unsolvedWhites.erase(unsolvedWhites.begin() + i);
			i--;
			continue;
		}
		if (region.GetSquareCount() > board.GetRequiredSize(pt))
		{
			return false;
		}
		if (region.GetSquareCount() < board.GetRequiredSize(pt))
		{
			bool hasNonBlackNeighbour = false;
			region.Neighbours([&hasNonBlackNeighbour](const Point&, const Square& sq)
			{
				if (sq.GetState() != SquareState::Black)
				{
					hasNonBlackNeighbour = true;
					return false;
				}
				return true;
			});

			if (!hasNonBlackNeighbour)
				return false;
		}
	}

	for (int i = 0; i < startOfUnconnectedWhite.size(); i++)
	{
		Point pt = startOfUnconnectedWhite[i];
		Region r = Region(&board, pt).ExpandAllInline([](const Point& pt, Square& square)
		{
			return square.GetState() == SquareState::White && square.GetOrigin() == (uint8_t)~0;
		});

		for (int j = 0; j < startOfUnconnectedWhite.size(); j++)
		{
			if (j == i)
				continue;

			if (r.Contains(startOfUnconnectedWhite[j]))
			{
				startOfUnconnectedWhite.erase(startOfUnconnectedWhite.begin() + j);

				if (j < i)
					i--; // TODO: untested condition

				j--;
			}
		}

		if (board.Get(pt).GetOrigin() != (uint8_t)~0)
		{
			startOfUnconnectedWhite.erase(startOfUnconnectedWhite.begin() + i);
			i--;
			continue;
		}
	}

	SolveUnfinishedWhiteIsland();
	return true;
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
		{
			sq.SetState(SquareState::Black);
			return false;
		}

		return true;
	});
}

bool Solver::SolveUnfinishedWhiteIsland()
{
	std::vector<int> badOrigins;

	for (int i = 0; i < unsolvedWhites.size(); i++)
	{
		Point pt = initialWhites[unsolvedWhites[i]];

		auto region = Region(&board, pt);

		bool reachedAnotherOrigin = false;
		uint8_t sourceOrigin = region.GetSameOrigin();
		uint8_t sourceSize = region.GetSameSize();

		if (std::find(badOrigins.begin(), badOrigins.end(), sourceOrigin) != badOrigins.end())
			continue;

		region.ExpandAllInline([&reachedAnotherOrigin, sourceOrigin, &badOrigins](const Point& pt, Square& square)
		{
			if (reachedAnotherOrigin)
				return false;

			if (square.GetState() == SquareState::White)
			{
				if (square.GetOrigin() != (uint8_t)~0 && square.GetOrigin() != sourceOrigin)
				{
					reachedAnotherOrigin = true;

					if (std::find(badOrigins.begin(), badOrigins.end(), sourceOrigin) == badOrigins.end())
						badOrigins.push_back(square.GetOrigin());

					return true;
				}
				return true;
			}
			else if (square.GetState() == SquareState::Unknown)
			{
				return true;
			}

			return false;
		});

		if (reachedAnotherOrigin)
			continue;

		if (region.GetSquareCount() == sourceSize)
		{
			region.SetState(SquareState::White);
			if (!CheckForSolvedWhites())
				return false;
			return true;
		}
		else if (region.GetSquareCount() > sourceSize)
		{
			auto white = Region(&board, pt).ExpandAllInline([](const Point& pt, Square& square) { return square.GetState() == SquareState::White; });

			bool isContiguous = true;
			for (int si = 0; si < white.GetSquareCount(); si++)
			{
				auto whiteSingle = Region(&board, white.GetSquares()[si]);
				if (!Region::Subtract(Region(whiteSingle).ExpandAllInline([](const Point& pt, Square& square) { return square.GetState() == SquareState::Unknown; }), whiteSingle).IsContiguous())
				{
					isContiguous = false;
					break;
				}
			}

			if (isContiguous)
				continue;

			auto tmp = Region::Subtract(region, white);
			auto tmpRet = tmp.IsContiguous();

			auto pathsOut = white.Neighbours([](const Point& pt, Square& square) { return square.GetState() == SquareState::Unknown; });

			std::vector<int> pathsOutSquareCount;
			for (int pi = 0; pi < pathsOut.GetSquareCount(); pi++)
			{
				pathsOutSquareCount.push_back(
					Region(&board, pathsOut.GetSquares()[pi])
					.ExpandAllInline([](const Point& pt, Square& square) { return square.GetState() == SquareState::Unknown; })
					.GetSquareCount()
				);
			}
			std::vector<int> compute(pathsOutSquareCount.size(), 0);

			auto missingSquares = sourceSize - white.GetSquareCount();

			bool isExpansionPlausible = false;
			for (int pi = 0; pi < pathsOutSquareCount.size(); pi++)
			{
				if (pathsOutSquareCount[pi] < missingSquares)
				{
					isExpansionPlausible = true;
					break;
				}
			}

			if (!isExpansionPlausible)
				continue;

			for (int pi = 0; pi < pathsOutSquareCount.size(); pi++)
			{
				auto remainingSquares = missingSquares;
				for (int pj = 0; pj < pathsOutSquareCount.size(); pj++)
				{
					auto index = (pi + pj) % pathsOutSquareCount.size();

					if (pathsOutSquareCount[index] > remainingSquares)
					{
						compute[index] = remainingSquares;

						auto expandedWhite = Region(&board, pathsOut.GetSquares()[index]);
						expandedWhite.SetState(SquareState::White);

						// I don't believe this operation can ever finish white, but lets check just in case
						if (!CheckForSolvedWhites())
							return false;

						return true;
					}
					else
					{
						remainingSquares -= pathsOutSquareCount[index];
						compute[index] = pathsOutSquareCount[index];
					}
				}
			}
		}
	}

	return true;
}

bool Solver::SolveBalloonWhiteSimple()
{
	for (int i = 0; i < startOfUnconnectedWhite.size(); i++)
	{
		auto res = SolveBalloonWhiteSimpleSingle(startOfUnconnectedWhite[i]);
		if (res == 0)
			return false;
		if (res != 1)
			return true;
	}
	for (int i = 0; i < unsolvedWhites.size(); i++)
	{
		auto res = SolveBalloonWhiteSimpleSingle(initialWhites[unsolvedWhites[i]]);
		if (res == 0)
			return false;
		if (res != 1)
			return true;
	}
	return true;
}

int Solver::SolveBalloonWhiteSimpleSingle(Point pt)
{
	auto white = Region(&board, pt);
	auto expectedSize = white.GetSameSize();

	white.ExpandAllInline([](const Point& pt, Square& square) { return square.GetState() == SquareState::White; });

	auto actualSize = white.GetSquareCount();

	if (white.Contains(Point{3,2}))
	{
		int a = 0;
	}

	Square sq;
	if (!white.StartNeighbourSpill(sq))
		return 1;

	auto spill = white.NeighbourSpill(sq);
	auto inflated = Region::Union(white, spill);
	auto toAdd = Region(&board);

	if (spill.GetSquareCount() == 1)
	{
		do
		{
			toAdd = Region::Union(toAdd, spill);
			inflated = Region::Union(inflated, spill);

			spill = inflated.NeighbourSpill(sq);
			sq.SetSize(sq.GetSize() + 1);
		}
		while (spill.GetSquareCount() == 1);

		toAdd.SetState(SquareState::White);

		if (!CheckForSolvedWhites())
			return 0;

		return 2;
	}

	if (spill.GetSquareCount() > 1)
	{
		spill = inflated.NeighbourSpill(sq);
		if (spill.GetSquareCount() == 1)
		{
			if (expectedSize > 0)
			{
				if (expectedSize < actualSize + inflated.GetSquareCount() + spill.GetSquareCount())
					return 1;
			}

			auto pathToSpill = Region::Intersection(inflated, spill.NeighbourSpill(sq));
			if (pathToSpill.GetSquareCount() == 1)
				pathToSpill.SetState(SquareState::White);

			spill.SetState(SquareState::White);

			if (!CheckForSolvedWhites())
				return 0;

			return 2;
		}
	}
	
	return 1;
}

bool Solver::SolveBalloonWhiteFillSpaceCompletely()
{
	for (int i = 0; i < unsolvedWhites.size(); i++)
	{
		auto white = Region(&board, initialWhites[unsolvedWhites[i]]);
		auto actualSize = white.GetSquareCount();
		auto expectedSize = white.GetSameSize();

		white.ExpandAllInline([](const Point& pt, Square& square) { return square.GetState() == SquareState::White; });

		Square sq;
		if (!white.StartNeighbourSpill(sq))
			continue;

		auto spill = white.NeighbourSpill(sq);
		auto inflated = Region::Union(white, spill);

		do
		{
			inflated = Region::Union(inflated, spill);

			sq.SetSize(sq.GetSize() + spill.GetSquareCount());
			spill = inflated.NeighbourSpill(sq);
		}
		while (spill.GetSquareCount() > 0);

		if (expectedSize == inflated.GetSquareCount())
		{
			inflated.SetState(SquareState::White);

			if (!CheckForSolvedWhites())
				return false;

			return true;
		}

		if (expectedSize > inflated.GetSquareCount())
		{
			return false;
		}
	}

	return true;
}

bool Solver::SolveBlackAroundWhite()
{
	ForEachRegion([this](const Region& black)
	{
		if (black.GetState() != SquareState::Black)
			return true;
	
		bool hasBlackBeenFilled = false;
		Region unknowns = black.Neighbours(SquareState::Unknown);
		unknowns.ForEachContiguousRegion([&unknowns, &black, &hasBlackBeenFilled](Region& unknown)
		{
			auto unknownNot = Region::Subtract(unknowns, unknown);

			// find nearest black and path to it
			bool isBlackReached = false;
			Point nearestBlack;
			auto path = Region(unknown).ExpandAllInline([&isBlackReached, &nearestBlack, &unknownNot, &black](const Point& pt, const Square& sq)
			{
				if (isBlackReached)
					return false;

				if (sq.GetState() == SquareState::Unknown)
				{
					if (unknownNot.Contains(pt))
						return false;

					return true;
				}

				if (sq.GetState() == SquareState::Black)
				{
					if (black.Contains(pt))
						return false;

					isBlackReached = true;
					nearestBlack = pt;
					return false;
				}

				return false;
			});
			
			if (!isBlackReached)
				return true;

			// the nearest other black
			auto otherBlack = Region(unknown.GetBoard(), nearestBlack).ExpandAllInline([](const Point&, const Square& sq) { return sq.GetState() == SquareState::Black; });

			// Find path around with `unknownNot`
			isBlackReached = false;
			auto pathAround = Region(unknownNot).ExpandAllInline([&isBlackReached, &unknown, &black](const Point& pt, const Square& sq)
			{
				if (isBlackReached)
					return false;

				if (sq.GetState() == SquareState::Unknown)
				{
					if (unknown.Contains(pt))
						return false;

					return true;
				}

				if (sq.GetState() == SquareState::Black)
				{
					if (black.Contains(pt))
						return false;

					isBlackReached = true;
					return false;
				}

				return false;
			});

			if (!isBlackReached)
				return true;

			auto relevantRegion = Region::Union(Region::Union(black, pathAround), otherBlack);

			// Get only "origin" whites that are fully contained in the `relevantRegion`
			auto whites = relevantRegion.Neighbours([&relevantRegion](const Point& pt, const Square& sq)
			{
				if (sq.GetState() != SquareState::White)
					return false;

				if (sq.GetOrigin() == (uint8_t)~0)
					return false;

				bool isContainedInRelevantRegion = true;

				Region(relevantRegion.GetBoard(), pt)
				.ExpandAllInline([](const Point&, const Square& sq) { return sq.GetState() == SquareState::White; })
				.Neighbours([&relevantRegion, &isContainedInRelevantRegion](const Point& pt, const Square& sq)
				{
					if (!isContainedInRelevantRegion)
						return false;

					if (!relevantRegion.Contains(pt))
					{
						isContainedInRelevantRegion = false;
						return false;
					}
					return true;
				});

				return isContainedInRelevantRegion;
			});

			if (whites.GetSquareCount() == 0)
				return true;

			if (!whites.IsContiguous())
			{
				// NOTE: this condition might not be needed (untested).
				//       surely other rules can take care of it
				//       when there's multiple regions here.
				return true;
			}
			
			// make sure to get whole regions of whites
			whites.ExpandAllInline([](const Point&, const Square& sq) { return sq.GetState() == SquareState::White; });

			// Check if all `whites` have space to expand when `unknown` is restricted
			bool whitesHaveEnoughSpace = true;
			whites.ForEachContiguousRegion([&unknown, &whitesHaveEnoughSpace](Region& white)
			{
				Square sq;
				if (!white.StartNeighbourSpill(sq))
					return true;

				auto spill = white.NeighbourSpill(sq);
				auto inflated = Region::Union(white, spill);

				while (spill.GetSquareCount() > 0)
				{
					if (inflated.GetSquareCount() >= sq.GetSize())
					{
						return true;
					}

					spill = inflated.NeighbourSpill(sq);
					spill = Region::Subtract(spill, unknown);

					inflated = Region::Union(inflated, spill);
				}

				whitesHaveEnoughSpace = false;
				return false;
			});

			if (whitesHaveEnoughSpace)
			{
				unknown.SetState(SquareState::Black);
				hasBlackBeenFilled = true;
				return false;
			}

			return true;
		});

		if (hasBlackBeenFilled)
			return false;
		
		return true;
	});

	return true;
}

bool Solver::SolveUnconnectedWhiteHasOnlyOnePossibleOrigin()
{
	for (int i = 0; i < startOfUnconnectedWhite.size(); i++)
	{
		auto white = Region(&board, startOfUnconnectedWhite[i])
			.ExpandAllInline([](const Point&, const Square& sq) { return sq.GetState() == SquareState::White; });

		auto relevantRegion = Region(white).ExpandAllInline([](const Point&, const Square& sq)
		{
			return
				sq.GetState() == SquareState::Unknown ||
				(sq.GetState() == SquareState::White && sq.GetOrigin() == (uint8_t)~0);
		});

		auto relevantWhites = relevantRegion.Neighbours([](const Point&, const Square& sq) { return sq.GetState() == SquareState::White && sq.GetOrigin() != (uint8_t)~0; });

		auto obstructed = Region(&GetBoard());

		relevantWhites.ForEachContiguousRegion([&obstructed](Region& white)
		{
			Square sq;
			if (!white.StartNeighbourSpill(sq))
				return true;
			
			auto spill = white.NeighbourSpill(sq);
			obstructed = Region::Union(obstructed, spill);
			return true;
		});

		relevantRegion = Region::Subtract(relevantRegion, obstructed);

		uint8_t origin = ~0;
		uint8_t size = 0;
		bool hasMultiplePossibleOrigins = false;
		
		relevantRegion.ForEachContiguousRegion([&origin, &size, &hasMultiplePossibleOrigins, &obstructed, &white](const Region& r)
		{
			if (hasMultiplePossibleOrigins)
				return false;

			if (Region::Intersection(r, white) != white)
				return true;

			auto directWhites = Region::Intersection(obstructed, r.Neighbours());
			directWhites.ExpandAllInline([](const Point&, const Square& sq) { return sq.GetState() == SquareState::White && sq.GetOrigin() != (uint8_t)~0; });

			directWhites.ForEachContiguousRegion([&origin, &size, &hasMultiplePossibleOrigins](const Region& r)
			{
				if (origin == (uint8_t)~0)
				{
					origin = r.GetSameOrigin();
					size = r.GetSameSize();
					return true;
				}

				if (origin != r.GetSameOrigin())
				{
					hasMultiplePossibleOrigins = true;
					return false;
				}

				return true;
			});

			// one we reached and handled the right region, we can stop
			return false;
		});

		if (!hasMultiplePossibleOrigins)
		{
			if (origin == (uint8_t)~0)
			{
				// found unconnected white with no possible connection
				return false;
			}

			white.SetOrigin(origin);
			white.SetSize(size);

			if (!CheckForSolvedWhites())
				return false;
		}
	}

	return true;
}

bool Solver::SolveHighLevelRecursive(const SolveSettings& settings)
{
	if (settings.maxDepth == 0)
		return true;

	Region optimalBlackRegion;
	double factor = 0.0f;

	ForEachRegion([this, &optimalBlackRegion, &factor](const Region& black)
	{
		if (black.GetState() != SquareState::Black)
			return true;

		auto continuations = black.Neighbours(SquareState::Unknown);
		auto borders = Region::Intersection(continuations.Neighbours(), black);

		double xCenter;
		double yCenter;
		borders.ForEach([&xCenter, &yCenter](const Point& pt, const Square&)
		{
			xCenter += pt.x;
			yCenter += pt.y;
			return true;
		});
		xCenter /= borders.GetSquareCount();
		yCenter /= borders.GetSquareCount();

		double borderDistances = 0;
		borders.ForEach([&xCenter, &yCenter, &borderDistances](const Point& pt, const Square&)
		{
			double x = std::abs(xCenter - pt.x);
			double y = std::abs(yCenter - pt.y);
			borderDistances += std::sqrt(x * x + y * y);
			return true;
		});


		double currentFactor = 0.0;
		currentFactor += 0.2 * continuations.GetSquareCount();
		//currentFactor += 0.5 * borders.GetSquareCount();
		currentFactor -= 0.2 * std::log(black.GetSquareCount());
		currentFactor += 0.5 * borderDistances;

		// size penalty
		if (black.GetSquareCount() <= 3)
			currentFactor += 1.0;

		if (optimalBlackRegion.GetSquareCount() == 0 || currentFactor < factor)
		{
			optimalBlackRegion = black;
			factor = currentFactor;
		}

		return true;
	});

	if (optimalBlackRegion.GetSquareCount() < 1)
		return true;

	Region unknown = optimalBlackRegion.Neighbours(SquareState::Unknown);

	int solvableFound = 0;

	unknown.ForEach([this, &settings, &solvableFound, &unknown](const Point& pt, const Square& sq)
	{
		if (solvableFound > 1)
			return false;

		if (id == 53 && GetIteration() == 187)
		{
			int a = 0;
		}

		if (GetIteration() > 92)
		{
			int a = 0;
		}

		Solver solver(Solver(*this));
		solver.depth++;
		solver.board.Get(pt).SetState(SquareState::Black);
		
		if (!solver.SolveWithRules(settings.Next()))
			return true;

		if (depth == 3)
		{
			int a = 0;
		}

		auto eval = solver.Evaluate();

		if (eval.IsSolved())
		{
			solvableFound = 1;
			solverStack.clear();
			solverStack.push_back(solver);
			return false;
		}

		if (eval.IsSolvable())
		{
			if (GetIteration() >= 107)
			{
				int a = 0;
			}

			solvableFound++;
			solverStack.push_back(solver);
		}

		if (solver.solverStack.size() > 0)
		{
			solvableFound += solver.solverStack.size();
			solverStack.insert(solverStack.end(), solver.solverStack.begin(), solver.solverStack.end());
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
		if (GetIteration() == 108)
		{
			int a = 0;
		}
		*this = this->solverStack.back();
		depth--;
	}

	if (solvableFound > 1)
	{
		int a = 0;
	}

	return true;
}

bool Solver::SolveWhiteAtPredictableCorner(const SolveSettings& settings)
{
	bool ret = true;
	ForEachRegion([this, &settings, &ret](const Region& black)
	{
		if (black.GetState() != SquareState::Black)
			return true;
		
		auto unknowns = black.Neighbours([](const Point& pt, Square& square) { return square.GetState() == SquareState::Unknown; });
		
		unknowns.ForEachContiguousRegion([this, &settings, &ret, &black](Region& unknown)
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

			if (!solverCopy.SolveWithRules(settings.Next()))
				return true;

			auto eval = solverCopy.Evaluate();

			if (eval.IsSolved())
			{
				*this = solverCopy;
				depth--;
			}
			else
			{
				if (solverCopy.solverStack.size())
				{
					// push possible solutions to stack
					solverStack.insert(solverStack.end(), solverCopy.solverStack.begin(), solverCopy.solverStack.end());
				}

				// continue by assuming there's black here
				whiteNew.SetState(SquareState::Black);
			}
			
			return false;
		});

		return true;
	});
	return ret;
}

void Solver::SolveBlackInCorneredWhite2By3()
{
	ForEachRegion([](const Region& r)
	{
		if (r.GetState() != SquareState::White)
			return true;

		Square sq;
		if (!r.StartNeighbourSpill(sq))
			return true;

		auto spill = r.NeighbourSpill(sq);

		if (spill.GetSquareCount() != 2)
			return true;

		auto a = Region(spill.GetBoard(), spill.GetSquares()[0]).Neighbours([](const Point&, const Square& sq) { return sq.GetState() == SquareState::Unknown; });
		auto b = Region(spill.GetBoard(), spill.GetSquares()[1]).Neighbours([](const Point&, const Square& sq) { return sq.GetState() == SquareState::Unknown; });

		auto possiblyBlack = Region::Intersection(a, b);

		if (possiblyBlack.GetSquareCount() != 1)
			return true;

		auto connectedWhite = possiblyBlack.Neighbours([&sq](const Point&, const Square& sqInner)
		{
			return
				sqInner.GetState() == SquareState::White &&
				sqInner.GetOrigin() != (uint8_t)~0 &&
				sqInner.GetOrigin() != sq.GetOrigin();
		});

		if (connectedWhite.GetSquareCount() != 1)
			return true;

		possiblyBlack.SetState(SquareState::Black);
		return false;
	});
}

void Solver::SolveDisjointedBlack()
{
	ForEachRegion([](const Region& black)
	{
		if (black.GetState() != SquareState::Black)
			return true;

		auto unknowns = black.Neighbours([](const Point&, const Square& sq) { return sq.GetState() == SquareState::Unknown; });

		bool endLoop = false;
		//unknowns.ForEachContiguousRegion([&unknowns, &black, &endLoop](const Region& unknownEdge)
		unknowns.ForEach([&unknowns, &black, &endLoop](const Point& pt, const Square& unknownEdgeSquare)
		{
			auto unknownEdge = Region((Board*)black.GetBoard(), pt);

			auto otherSideBlack = unknownEdge.Neighbours([&black](const Point& pt, const Square& sq) { return sq.GetState() == SquareState::Black && !black.Contains(pt); });

			Square sq;
			if (!otherSideBlack.StartNeighbourSpill(sq))
				return true;

			auto spill = Region::Subtract(otherSideBlack.NeighbourSpill(sq), unknownEdge);
			auto inflated = otherSideBlack;

			bool hasSingleJunction = true;
			while(spill.GetSquareCount() > 0)
			{
				inflated = Region::Union(inflated, spill);

				if (Region::Intersection(inflated, black).GetSquareCount() > 0)
				{
					hasSingleJunction = false;
					break;
				}

				spill = Region::Subtract(inflated.NeighbourSpill(sq), unknownEdge);
			}

			if (hasSingleJunction)
			{
				const_cast<Region&>(unknownEdge).SetState(SquareState::Black);
				endLoop = true;
				return false;
			}

			return true;
		});


		if (endLoop)
		{
			return false;
		}

		return true;
	});
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
		[this](){ return SolveUnconnectedWhiteHasOnlyOnePossibleOrigin(); },
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

	while (true)
	{
		Board boardIterationStart = board;

		if (*iteration == 164)
		{
			int a = 0;
		}

		int ret = SolvePhase(phase, settings);
		if (ret < 0)
			break;

		if (ret == 0)
			return false;

		hasChangedInPrevLoop = (board != boardIterationStart);
		if (!hasChangedInPrevLoop)
		{
			phase++;
		}
		else
		{
			//if (*iteration == 92)
			if (*iteration == 187 && depth == 5 && id == 58)
			{
				int a = 0;
			}

			board.Print(std::cout);
			std::cout << "Depth: " << depth << std::endl;
			std::cout << "Iteration: " << *iteration << std::endl;

			phase = 0;
			(*iteration)++;

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
	}

	return true;
}

void Solver::ForEachRegion(const std::function<bool(Region &)>& callback)
{
	auto& board = this->board;

	Region handled(&board);

	board.ForEachSquare([&board, &handled, &callback](const Point& pt, const Square& sq)
	{
		if (handled.Contains(pt))
			return true;

		Region region = Region(&board, pt).ExpandAllInline(
			[&sq](const Point&, const Square& sqInner) { return sqInner.GetState() == sq.GetState(); }
		);

		handled = Region::Union(handled, region);

		return callback(region);
	});
}


bool Solver::Solve(const SolveSettings& settings)
{
	Initialize();
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