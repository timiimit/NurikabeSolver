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
	iteration = 0;
}

Solver::Solver(const Solver& other)
	: board(other.board)
	, initialWhites(other.initialWhites)
	, unsolvedWhites(other.unsolvedWhites)
	, startOfUnconnectedWhite(other.startOfUnconnectedWhite)
	, iteration(0)
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

bool Solver::SolveInflateTrivial(SquareState state)
{
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

			GetBoard().Print(std::cout);
			rContinuations.SetState(state);
			GetBoard().Print(std::cout);

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

				Region blackNeighbours = Region(&board, pt).Neighbours([](const Point& pt, Square& square) {
					return square.GetState() == SquareState::Black;
				});

				if (blackNeighbours.GetSquareCount() == 4)
				{
					board.SetBlack(pt);
					return false;
				}

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

		
		Region unknown = black.Neighbours(SquareState::Unknown);
		Region around = unknown.Neighbours();

		bool foundSolved = false;

		unknown.ForEach([this, &foundSolved, &unknown, &around](const Point& pt, const Square& sq)
		{
			if (foundSolved)
				return true;

			Solver solver = Solver(*this);
			solver.board.Get(pt).SetState(SquareState::Black);
			
			if (!solver.SolveWithRules())
				return true;

			auto eval = solver.Evaluate();

			if (eval.IsSolved())
			{
				// adopt board
				board = solver.GetBoard();
				iteration += solver.iteration;
				foundSolved = true;
				return false;
			}

			if (eval.IsSolvable())
			{
				solverStack.push_back(solver);
			}

			return true;
		});

		return true;

		// Region unknown = Region((Board*)black.GetBoard());
		// Region spill;
		// do
		// {
		// 	spill = black.Neighbours(SquareState::Unknown);
		// 	unknown = Region::Union(unknown, spill);
		// }
		// while (spill.GetSquareCount() > 1);

		// Region blackAndUnknown = Region::Union(black, unknown);

		// auto whites = black.Neighbours(SquareState::White);
		// whites.ForEachContiguousRegion([&blackAndUnknown](const Region& whitePart)
		// {
		// 	auto white = Region(whitePart).ExpandAllInline(SquareState::White);
		// 	auto whiteNeighbours = white.Neighbours();

		// 	if (Region::Intersection(whiteNeighbours, blackAndUnknown) == whiteNeighbours)
		// 	{
		// 		// found an isolated white
		// 		// ensure that is has enough space
		// 	}

		// 	return true;
		// });
		
		// return true;
	});

	return true;
}

bool Solver::SolveWhiteAtPredictableCorner()
{
	bool ret = true;
	ForEachRegion([this, &ret](const Region& r)
	{
		if (r.GetState() != SquareState::Black)
			return true;
		
		auto unknowns = r.Neighbours([](const Point& pt, Square& square) { return square.GetState() == SquareState::Unknown; });
		if (!unknowns.IsContiguous() || unknowns.GetSquareCount() != 2)
			return true;
		
		auto surrounding = unknowns.Neighbours();
		// if (surrounding.GetSquareCount() != 5)
		// 	return true;

		auto blackEntry = Region((Board*)r.GetBoard());
		auto whiteEntry = Region((Board*)r.GetBoard());
		auto unknownEntry = Region((Board*)r.GetBoard());
		bool isValid = true;

		surrounding.ForEachContiguousRegion([&isValid, &blackEntry, &whiteEntry, &unknownEntry](const Region& r)
		{
			if (!r.IsSameState())
				return true;

			if (r.GetState() == SquareState::Black && r.GetSquareCount() == 2 && blackEntry.GetSquareCount() == 0)
			{
				blackEntry = r;
				return true;
			}

			if (r.GetState() == SquareState::White && r.GetSquareCount() == 1 && whiteEntry.GetSquareCount() == 0)
			{
				whiteEntry = r;
				return true;
			}

			if (r.GetState() == SquareState::Unknown)
			{
				if ((r.GetSquareCount() == 2 || r.GetSquareCount() == 1) && unknownEntry.GetSquareCount() <= 3)
				{
					unknownEntry = Region::Union(unknownEntry, r);
					return true;
				}
			}

			isValid = false;
			return false;
		});

		if (isValid &&
			blackEntry.GetSquareCount() == 2 && whiteEntry.GetSquareCount() == 1 &&
			(unknownEntry.GetSquareCount() >= 2 && unknownEntry.GetSquareCount() <= 3))
		{
			auto toFillWithWhite = Region::Intersection(unknowns, whiteEntry.Neighbours());
			if (toFillWithWhite.GetSquareCount() == 1)
			{
				toFillWithWhite.SetState(SquareState::White);
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

	ForEachRegion([&eval](const Region& r)
	{
		if (r.GetState() == SquareState::Black)
		{
			if (eval.existsBlackRegion)
				eval.existsMoreThanOneBlackRegion = true;
			eval.existsBlackRegion = true;

			if (!eval.existsClosedBlack)
			{
				r.Neighbours([&eval](const Point&, const Square& sq)
				{
					if (sq.GetState() != SquareState::White)
					{
						eval.existsClosedBlack = true;
						return false;
					}
					return true;
				});
			}
		}
		else if (r.GetState() == SquareState::White)
		{
			if (r.GetSameOrigin() == (uint8_t)~0)
			{
				eval.existsUnconnectedWhite = true;
			}
			if (r.GetSquareCount() != r.GetSameSize())
			{
				eval.existsMissizedFinishedWhite = true;
			}
		}
		else if (r.GetState() == SquareState::Unknown)
		{
			eval.existsUnknownRegion = true;
		}
		return true;
	});

	return eval;
}

bool Solver::SolveDivergeBlack(Solver& solver, std::vector<Solver>& solverStack, int maxDiverges, float blackToUnknownRatio)
{
	bool ret = false;
	Region handled(&solver.board);

	solver.board.ForEachSquare([&solver, &solverStack, maxDiverges, blackToUnknownRatio, &handled, &ret](const Point& pt, const Square& sq)
	{
		if (sq.GetState() != SquareState::Black)
			return true;

		if (handled.Contains(pt))
			return true;

		Region black = Region(&solver.board, pt).ExpandAllInline(
			[](const Point&, const Square& sqInner) { return sqInner.GetState() == SquareState::Black; }
		);

		handled = Region::Union(handled, black);

		Region unknownNeighbours = black.Neighbours(
			[](const Point&, const Square& sqInner) { return sqInner.GetState() == SquareState::Unknown; }
		);

		if (unknownNeighbours.GetSquareCount() > maxDiverges)
			return true;

		if (black.GetSquareCount() / (float)unknownNeighbours.GetSquareCount() < blackToUnknownRatio)
			return true;

		if (unknownNeighbours.GetSquareCount() > 0)
		{
			unknownNeighbours.ForEach([&solver, &solverStack](const Point& pt, const Square&)
			{
				auto solverCopy = Solver(solver);
				solverCopy.GetBoard().SetBlack(pt);
				solverStack.push_back(solverCopy);
				return true;
			});
			ret = true;
			return false;
		}

		return true;
	});

	return ret;
}

bool Solver::SolveDivergeWhite(Solver& solver, std::vector<Solver>& solverStack, int maxDiverges, int maxSizeOfWhiteToDiverge)
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

	int min = -1;
	for (int i = 0; i < unsolvedWhiteRegions.size(); i++)
	{
		int iSize = solver.board.GetRequiredSize(solver.initialWhites[solver.unsolvedWhites[i]]);

		if (iSize > maxSizeOfWhiteToDiverge)
			continue;

		if (unsolvedWhiteRegions[i].GetSquareCount() > maxDiverges)
			continue;

		if (min < 0)
		{
			min = i;
			continue;
		}

		int minSize = solver.board.GetRequiredSize(solver.initialWhites[solver.unsolvedWhites[min]]);
		if (iSize < minSize)
		{
			min = i;
		}
		else if (
			iSize == minSize &&
			unsolvedWhiteRegions[i].GetSquareCount() < unsolvedWhiteRegions[min].GetSquareCount())
		{
			min = i;
		}
	}

	if (min < 0)
		return false;

	auto& bestRegion = unsolvedWhiteRegions[min];

	for (int i = 0; i < bestRegion.GetSquareCount(); i++)
	{
		auto solverCopy = Solver(solver);
		auto& sq = solverCopy.board.Get(bestRegion.GetSquares()[i]);
		sq.SetState(SquareState::White);
		sq.SetSize(solver.board.GetRequiredSize(solver.initialWhites[solver.unsolvedWhites[min]]));
		sq.SetOrigin(solver.unsolvedWhites[min]);
		if (!solverCopy.CheckForSolvedWhites())
			continue;
		
		if (Rules::IsSolvable(solverCopy.board))
			solverStack.push_back(std::move(solverCopy));
	}

	return true;
}

void Solver::SolveDiverge(Solver& solver, std::vector<Solver>& solverStack)
{
	float blackRatio = 10.0f;
	int divergeMax = 2;
	while (true)
	{
		for (int i = 2; i < 128; i += (i - 1))
		{
			if (SolveDivergeWhite(solver, solverStack, divergeMax, i))
				return;
		}

		for (int i = 0; i < 2; i++)
		{
			if (SolveDivergeBlack(solver, solverStack, 2, blackRatio))
				return;
			
			blackRatio /= 2.0f;
		}

		// if (divergeMax > 3)
		// {
		// 	auto solverCopy = Solver(solver);

		// 	if (solverCopy.SolveGuessBlackToUnblock(10))
		// 	{
		// 		solverStack.push(solverCopy);
		// 		return;
		// 	}
		// }

		divergeMax++;
	}
}

int Solver::SolvePhase(int phase)
{
	static std::function<bool()> phases[] =
	{
		[this](){ return SolveInflateTrivial(SquareState::White); },
		[this](){ return SolveInflateTrivial(SquareState::Black); },
		[this](){ return SolvePerSquare(); },
		[this](){ SolveUnreachable(); return true; },
		[this](){ return SolveWhiteAtPredictableCorner(); },
		[this](){ return SolveBalloonWhiteFillSpaceCompletely(); },
		[this](){ SolveDisjointedBlack(); return true; },
		[this](){ SolveBlackInCorneredWhite2By3(); return true; },
		[this](){ return SolveBalloonWhiteSimple(); },
		[this](){ return SolveBlackAroundWhite(); },
	};

	if (phase >= sizeof(phases) / sizeof(*phases))
		return -1;

	bool ret = phases[phase]();

	if (ret)
		return 1;

	return 0;
}

bool Solver::SolveWithRules()
{
	int phase = 0;
	bool hasChangedInPrevLoop = true;

	while (true)
	{
		Board boardIterationStart = board;

		if (iteration == 164)
		{
			int a = 0;
		}

		int ret = SolvePhase(phase);
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
			//boardIterationStart.Print(std::cout);
			board.Print(std::cout);

			phase = 0;
		}
		iteration++;
	}

	return true;
}

void Solver::ForEachRegion(const std::function<bool(const Region &)>& callback)
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


bool Solver::Solve()
{
	Initialize();
	if (!CheckForSolvedWhites())
		return false;

	solverStack.clear();
	solverStack.push_back(*this);

	int iteration = 0;
	while (true)
	{
		if (solverStack.size() == 0)
		{
			return false;
		}

		int	solverIndex;

		// if (false) //solverStack.size() < 5)
		// 	solverIndex = rand() % solverStack.size();
		// else
		solverIndex = solverStack.size() - 1;

		Solver solver = solverStack[solverIndex];
		solverStack.erase(solverStack.begin() + solverIndex);

		if (!solver.SolveWithRules())
			continue;

		auto eval = solver.Evaluate();

		if (eval.IsSolved())
		{
			board = solver.board;
			solverStack.clear();
			break;
		}
		
		if (!eval.IsSolvable())
			continue;

		// std::cout << "Iteration #" << iteration << std::endl;
		// solver.board.Print(std::cout);
		// std::cout << std::endl;

		SolveDiverge(solver, solverStack);
	}

	return true;
}