#include "NurikabeSolver.h"
#include <iostream>
#include <assert.h>
#include <cmath>

using namespace Nurikabe;

bool Solver::SolveInflateTrivial(SquareState state)
{
	if (*iteration >= 485 && state == SquareState::Black)
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

		if (state == SquareState::White && r.GetSquareCount() + 1 == r.GetSameSize())
		{
			// if only 1 white is missing
			Region diagonalBlack = Region(&GetBoard());
			bool wasAssigned = false;
			rContinuations.ForEachContiguousRegion([&diagonalBlack, &wasAssigned](const Region& r)
			{
				auto expanded = Region(r).ExpandSingleInline();
				if (diagonalBlack.GetSquareCount() == 0)
				{
					if (wasAssigned)
						return false;

					diagonalBlack = expanded;
					wasAssigned = true;
				}
				else
				{
					diagonalBlack = Region::Intersection(diagonalBlack, expanded);
				}
				return true;
			});

			diagonalBlack = Region::Subtract(diagonalBlack, r);
			
			if (diagonalBlack.GetSquareCount() == 1)
			{
				if (diagonalBlack.GetState() != SquareState::Black)
				{
					diagonalBlack.SetState(SquareState::Black);
					return RETURN_AFTER_FILLING_BLACK;
				}
			}
		}

		return true;
	});
	return ret;
}

bool Solver::SolvePerSquare()
{
	bool ret = true;
	board.ForEachSquare([this, &ret](const Point& pt, const Square& square)
	{
		if (pt == Point{5, 13})
		{
			int a = 0;
		}

		if (square.GetState() == SquareState::Unknown)
		{
			Region whiteNeighbours = Region(&board, pt)
				.Neighbours([](const Point& pt, const Square& square) {
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
				return RETURN_AFTER_FILLING_BLACK;
			}

			bool areAllNeighboursBlack = true;
			Region(&board, pt).Neighbours([&areAllNeighboursBlack](const Point&, const Square& square) {
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
				return RETURN_AFTER_FILLING_BLACK;
			}

			Region blackNeighbours = Region(&board, pt).Neighbours([](const Point& pt, const Square& square) {
				return square.GetState() == SquareState::Black;
			});

			// handle case where there are 3 black and 1 unknown square in 2x2
			Region blackDiagonalNeighbours = blackNeighbours.Neighbours([&pt](const Point& ptInner, const Square& square) {
				return
					Distance(pt.x, ptInner.x) == 1 &&
					Distance(pt.y, ptInner.y) == 1 &&
					square.GetState() == SquareState::Black;
			});

			blackDiagonalNeighbours.ForEach([&pt, &square, this](const Point& ptInner, const Square&) {
				Point delta = pt - ptInner;
				if (board.IsBlack(Point{ ptInner.x + delta.x, ptInner.y }) &&
					board.IsBlack(Point{ ptInner.x, ptInner.y + delta.y }))
				{
					// we found a lone white square, we need to connect it logically
					board.SetWhite(pt);
					startOfUnconnectedWhite.push_back(pt);
					return RETURN_AFTER_FILLING_WHITE;
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
			.ExpandAllInline([](const Point& pt, const Square& square)
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
		Region r = Region(&board, pt).ExpandAllInline([](const Point& pt, const Square& square)
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
	board.ForEachSquare([this](const Point& pt, const Square& sq)
	{
		if (sq.GetState() != SquareState::Unknown)
			return true;

		bool isReachable = false;
		for (int i = 0; i < unsolvedWhites.size() && !isReachable; i++)
		{
			auto& initialWhite = initialWhites[unsolvedWhites[i]];
			auto region = Region(&board, initialWhite)
				.ExpandAllInline([](const Point&, const Square& sq) { return sq.GetState() == SquareState::White; });

			auto regionSquaresLeft = board.Get(initialWhite).GetSize() - region.GetSquareCount();
			
			region.ForEach([&pt, &isReachable, regionSquaresLeft](const Point& ptInner, const Square& sq)
			{
				if (Rules::CanReach(ptInner, pt, regionSquaresLeft))
					isReachable = true;
				return !isReachable;
			});
		}

		if (!isReachable)
		{
			board.SetBlack(pt);
			return RETURN_AFTER_FILLING_BLACK;
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

		region.ExpandAllInline([&reachedAnotherOrigin, sourceOrigin, &badOrigins](const Point& pt, const Square& square)
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
			auto white = Region(&board, pt).ExpandAllInline([](const Point& pt, const Square& square) { return square.GetState() == SquareState::White; });

			bool isContiguous = true;
			for (int si = 0; si < white.GetSquareCount(); si++)
			{
				auto whiteSingle = Region(&board, white.GetSquares()[si]);
				if (!Region::Subtract(Region(whiteSingle).ExpandAllInline([](const Point& pt, const Square& square) { return square.GetState() == SquareState::Unknown; }), whiteSingle).IsContiguous())
				{
					isContiguous = false;
					break;
				}
			}

			if (isContiguous)
				continue;

			auto tmp = Region::Subtract(region, white);
			auto tmpRet = tmp.IsContiguous();

			auto pathsOut = white.Neighbours([](const Point& pt, const Square& square) { return square.GetState() == SquareState::Unknown; });

			std::vector<int> pathsOutSquareCount;
			for (int pi = 0; pi < pathsOut.GetSquareCount(); pi++)
			{
				pathsOutSquareCount.push_back(
					Region(&board, pathsOut.GetSquares()[pi])
					.ExpandAllInline([](const Point& pt, const Square& square) { return square.GetState() == SquareState::Unknown; })
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
	bool ret = true;
	ForEachRegion([this, &ret](const Region& r)
	{
		if (r.GetState() != SquareState::White)
			return true;

		auto result = SolveBalloonWhiteSimple(r);

		if (result > 1)
			return true;

		if (result == 0)
			ret = false;

		return false;
	});
	return ret;
}

int Solver::SolveBalloonWhiteSimple(const Region& r)
{
	auto expectedSize = r.GetSameSize();
	auto actualSize = r.GetSquareCount();

	if (r.Contains(Point{3,2}))
	{
		int a = 0;
	}

	Square sq;
	if (!r.StartNeighbourSpill(sq))
		return 2;

	auto spill = r.NeighbourSpill(sq);
	auto inflated = Region::Union(r, spill);
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

		return 1;
	}

	if (spill.GetSquareCount() > 1)
	{
		spill = inflated.NeighbourSpill(sq);
		if (spill.GetSquareCount() == 1)
		{
			if (expectedSize > 0)
			{
				if (expectedSize < actualSize + inflated.GetSquareCount() + spill.GetSquareCount())
					return 2;
			}

			auto pathToSpill = Region::Intersection(inflated, spill.NeighbourSpill(sq));
			if (pathToSpill.GetSquareCount() == 1)
				pathToSpill.SetState(SquareState::White);


			if (spill.GetState() != SquareState::White)
			{
				spill.SetState(SquareState::White);

				if (!CheckForSolvedWhites())
					return 0;

				return 1;
			}
		}
	}
	
	return 2;
}

bool Solver::SolveBalloonBlack()
{
	// find all black and unknown regions
	Region blackAndUnknown;
	ForEachRegion([&blackAndUnknown](const Region& r)
	{
		if (r.GetState() != SquareState::Black)
			return true;

		blackAndUnknown = r;
		return false;
	});

	if (blackAndUnknown.GetSquareCount() == 0)
		return true;
	
	blackAndUnknown.ExpandAllInline([](const Point&, const Square& sq) { return sq.GetState() == SquareState::Black || sq.GetState() == SquareState::Unknown; });

	// check unknowns if there is a passthrough anywhere and mark all of them
	ForEachRegion([this, &blackAndUnknown](const Region& unknown)
	{
		if (unknown.GetState() != SquareState::Unknown)
			return true;

		unknown.ForEach([this, &blackAndUnknown](const Point& pt, const Square&)
		{
			Region unknownPassthrough = Region(&GetBoard(), pt);

			Region next = Region::Subtract(blackAndUnknown, unknownPassthrough);
			if (next.IsContiguous())
				return true;

			bool allContiguousRegionsHaveBlack = true;
			next.ForEachContiguousRegion([&allContiguousRegionsHaveBlack](const Region& r)
			{
				if (!r.HasSquareWithState(SquareState::Black))
				{
					allContiguousRegionsHaveBlack = false;
					return false;
				}
				return true;
			});

			if (allContiguousRegionsHaveBlack)
			{
				unknownPassthrough.SetState(SquareState::Black);
				return RETURN_AFTER_FILLING_BLACK;
			}
			return true;
		});

		return true;
	});

	return true;
}

bool Solver::SolveBalloonWhiteFillSpaceCompletely()
{
	for (int i = 0; i < unsolvedWhites.size(); i++)
	{
		auto white = Region(&board, initialWhites[unsolvedWhites[i]]);
		auto actualSize = white.GetSquareCount();
		auto expectedSize = white.GetSameSize();

		white.ExpandAllInline([](const Point& pt, const Square& square) { return square.GetState() == SquareState::White; });

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
		unknowns.ForEachContiguousRegion([&unknowns, &black, &hasBlackBeenFilled](const Region& unknown)
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
			auto otherBlack = Region((Board*)unknown.GetBoard(), nearestBlack).ExpandAllInline([](const Point&, const Square& sq) { return sq.GetState() == SquareState::Black; });

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
			whites.ForEachContiguousRegion([&unknown, &whitesHaveEnoughSpace](const Region& white)
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
				return RETURN_AFTER_FILLING_BLACK;
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

		relevantWhites.ForEachContiguousRegion([&obstructed](const Region& white)
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

void Solver::SolveBlackInCorneredWhite2By3()
{
	ForEachRegion([](const Region& r)
	{
		if (r.GetState() != SquareState::White)
			return true;

		if (r.GetSameSize() == 1)
			return true;

		if (r.GetSameOrigin() == (uint8_t)~0)
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

		if (connectedWhite.GetSameSize() == 1)
			return true;

		possiblyBlack.SetState(SquareState::Black);
		return RETURN_AFTER_FILLING_BLACK;
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
				unknownEdge.SetState(SquareState::Black);
				endLoop = true;
				return RETURN_AFTER_FILLING_BLACK;
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
