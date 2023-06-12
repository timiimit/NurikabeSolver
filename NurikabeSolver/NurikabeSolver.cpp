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
	, startOfUnconnectedWhite(other.startOfUnconnectedWhite)
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

bool Solver::SolvePerSquare()
{
	bool ret = true;
	board.ForEachSquare([this, &ret](const Point& pt, Square& square)
		{
			if (pt == Point{8, 7})
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
					return true;
				}

				// if (whiteNeighbours.GetSquareCount() > 1 && !whiteNeighbours.IsSameOrigin())
				// {
				// 	board.SetBlack(pt);
				// 	return true;
				// }

				Region blackNeighbours = Region(&board, pt).Neighbours([](const Point& pt, Square& square) {
					return square.GetState() == SquareState::Black;
				});

				if (blackNeighbours.GetSquareCount() == 4)
				{
					board.SetBlack(pt);
					return true;
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
					return true;
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

				if (rColorContinuations.GetSquareCount() == 1)
				{
					// If there's only one way to go, then go there

					rColorContinuations.SetState(square.GetState());
					if (square.GetState() == SquareState::White)
					{
						rColorContinuations.SetSize(square.GetSize());
						rColorContinuations.SetOrigin(square.GetOrigin());
						if (!CheckForSolvedWhites())
						{
							ret = false;
							return false;
						}
					}
					return true;
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
			GetBoard().Print(std::cout);

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
			sq.SetState(SquareState::Black);

		return true;
	});
}

bool Solver::SolveUnfinishedWhiteIsland()
{
	bool ret = true;
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
			region.SetSize(sourceSize);
			region.SetOrigin(sourceOrigin);
			if (!CheckForSolvedWhites())
				return false;
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
						expandedWhite.SetOrigin(sourceOrigin);
						expandedWhite.SetSize(sourceSize);

						// I don't believe this operation can ever finish white, but lets check just in case
						if (!CheckForSolvedWhites())
							return false;

						// Exit out of boths loops
						pi = pathsOutSquareCount.size();
						break;
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

bool Solver::SolveBalloonWhite()
{
	for (int i = 0; i < unsolvedWhites.size(); i++)
	{
		auto white = Region(&board, initialWhites[unsolvedWhites[i]]);
		auto origin = white.GetSameOrigin();
		auto expectedSize = white.GetSameSize();
		auto actualSize = white.GetSquareCount();

		white.ExpandAllInline([](const Point& pt, Square& square) { return square.GetState() == SquareState::White; });

		auto unknownNeighbours = white.Neighbours([](const Point& pt, Square& square) { return square.GetState() == SquareState::Unknown; });

		if (!unknownNeighbours.IsContiguous())
			continue;

		while (true)
		{
			auto unknownNeighboursNew = Region::Subtract(
				unknownNeighbours.Neighbours([](const Point&, Square& square) { return square.GetState() == SquareState::Unknown; }),
				unknownNeighbours
			);
			auto sizeDiff = unknownNeighboursNew.GetSquareCount();

			if (sizeDiff == 0)
				break;

			if (unknownNeighboursNew.GetSquareCount() + unknownNeighbours.GetSquareCount() + actualSize > expectedSize)
				break;

			if (sizeDiff > 1)
			{
				unknownNeighbours = Region::Union(unknownNeighbours, unknownNeighboursNew);
				continue;
			}
			
			if (sizeDiff == 1)
			{
				auto guaranteedWhite = Region::Union(
					unknownNeighboursNew,
					unknownNeighboursNew.Neighbours([&unknownNeighbours](const Point& pt, Square& square) { return square.GetState() == SquareState::Unknown && unknownNeighbours.Contains(pt); })
				);

				guaranteedWhite.SetState(SquareState::White);
				guaranteedWhite.SetOrigin(origin);
				guaranteedWhite.SetSize(expectedSize);

				if (!CheckForSolvedWhites())
					return false;
				
				break;
			}
		}
	}

	return true;
}

bool Solver::SolveBalloonUnconnectedWhite()
{
	for (int i = 0; i < startOfUnconnectedWhite.size(); i++)
	{
		auto white = Region(&board, startOfUnconnectedWhite[i]);
		auto actualSize = white.GetSquareCount();

		auto origin = white.GetSameOrigin();
		auto expectedSize = white.GetSameSize();

		white.ExpandAllInline([](const Point& pt, Square& square) { return square.GetState() == SquareState::White; });

		auto unknownNeighbours = white.Neighbours([](const Point& pt, Square& square) { return square.GetState() == SquareState::Unknown; });
		unknownNeighbours.ExpandSingleInline([](const Point& pt, Square& square) { return square.GetState() == SquareState::Unknown; });

		if (!unknownNeighbours.IsContiguous())
			continue;

		while (true)
		{
			auto unknownNeighboursNew = Region::Subtract(
				unknownNeighbours.Neighbours([](const Point&, Square& square) { return square.GetState() == SquareState::Unknown; }),
				unknownNeighbours
			);
			auto sizeDiff = unknownNeighboursNew.GetSquareCount();

			if (sizeDiff == 0)
				break;

			if (unknownNeighboursNew.GetSquareCount() + unknownNeighbours.GetSquareCount() + actualSize > expectedSize)
				break;

			if (sizeDiff > 1)
			{
				unknownNeighbours = Region::Union(unknownNeighbours, unknownNeighboursNew);
				continue;
			}
			
			if (sizeDiff == 1)
			{
				auto guaranteedWhite = Region::Union(
					unknownNeighboursNew,
					unknownNeighboursNew.Neighbours([&unknownNeighbours](const Point& pt, Square& square) { return square.GetState() == SquareState::Unknown && unknownNeighbours.Contains(pt); })
				);

				guaranteedWhite.SetState(SquareState::White);
				guaranteedWhite.SetOrigin(origin);
				guaranteedWhite.SetSize(expectedSize);

				if (!CheckForSolvedWhites())
					return false;
				
				break;
			}
		}
	}

	return true;
}

bool Solver::SolveBalloonUnconnectedWhiteSimple()
{
	for (int i = 0; i < startOfUnconnectedWhite.size(); i++)
	{
		auto white = Region(&board, startOfUnconnectedWhite[i]);
		auto actualSize = white.GetSquareCount();

		white.ExpandAllInline([](const Point& pt, Square& square) { return square.GetState() == SquareState::White; });

		Square sq;
		if (!white.StartNeighbourSpill(sq))
			continue;

		GetBoard().Print(std::cout);

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

			GetBoard().Print(std::cout);
			if (!CheckForSolvedWhites())
				return false;

			continue;
		}

		if (spill.GetSquareCount() > 1)
		{
			spill = inflated.NeighbourSpill(sq);
			if (spill.GetSquareCount() == 1)
			{
				auto toAdd = Region::Union(spill, Region::Intersection(inflated, spill.NeighbourSpill(sq)));
				toAdd.SetState(SquareState::White);

				GetBoard().Print(std::cout);
				if (!CheckForSolvedWhites())
					return false;
			}

			continue;
		}
		
		break;
	}

	return true;
}

bool Solver::SolveWhiteAtClosedBlack()
{
	for (int i = 0; i < startOfUnconnectedWhite.size(); i++)
	{
		auto white = Region(&board, startOfUnconnectedWhite[i])
			.ExpandAllInline([](const Point& pt, Square& square) { return square.GetState() == SquareState::White; });
		
		auto newlyAdded = Region(&board);

		auto unknownSkip = white.Neighbours([](const Point& pt, Square& square) { return square.GetState() == SquareState::Unknown; });
		auto unknownPrev = Region(unknownSkip);
		while (true)
		{
			auto unknown = unknownPrev.Neighbours(
				[&unknownSkip](const Point& pt, Square& square) { return square.GetState() == SquareState::Unknown && !unknownSkip.Contains(pt); }
			);

			if (unknown.GetSquareCount() != 1)
				break;

			auto before = newlyAdded.GetSquareCount();
			newlyAdded = Region::Union(newlyAdded, unknown);

			if (newlyAdded.GetSquareCount() == before)
				break;

			unknownPrev = unknown;
		}

		if (newlyAdded.GetSquareCount() > 1)
		{
			auto black = white
				.Neighbours([](const Point& pt, Square& square) { return square.GetState() == SquareState::Black; })
				.ExpandAllInline([](const Point& pt, Square& square) { return square.GetState() == SquareState::Black; });

			if (black.IsContiguous())
			{
				if (unknownSkip == black.Neighbours([](const Point& pt, Square& square) { return square.GetState() == SquareState::Unknown; }))
				{
					black = Region::Union(black, unknownSkip.Neighbours([&black](const Point& pt, Square& square) { return square.GetState() == SquareState::Black && !black.Contains(pt); }));

					Region blackNew = Region(&board);
					bool isValid = true;
					unknownSkip.ForEachContiguousRegion([&blackNew, &isValid](const Region& r)
					{
						if (r.Neighbours([](const Point& pt, Square& square) { return square.GetState() == SquareState::Black; }).GetSquareCount() == 2 &&
							r.Neighbours([](const Point& pt, Square& square) { return square.GetState() == SquareState::White; }).GetSquareCount() == 1 &&
							r.Neighbours([](const Point& pt, Square& square) { return square.GetState() == SquareState::Unknown; }).GetSquareCount() == 1)
						{
							blackNew = Region(r);
							return true;
						}
						else if (r.Neighbours([](const Point& pt, Square& square) { return square.GetState() == SquareState::Black; }).GetSquareCount() == 1 &&
							r.Neighbours([](const Point& pt, Square& square) { return square.GetState() == SquareState::White; }).GetSquareCount() == 1 &&
							r.Neighbours([](const Point& pt, Square& square) { return square.GetState() == SquareState::Unknown; }).GetSquareCount() == 1)
						{
							return true;
						}
						else
						{
							isValid = false;
							return false;
						}
					});

					if (isValid)
					{
						blackNew.SetState(SquareState::Black);
						Region::Subtract(unknownSkip, blackNew).SetState(SquareState::White);
						newlyAdded.SetState(SquareState::White);

						if (!CheckForSolvedWhites())
							return false;
					}
				}
			}
		}
	}

	return true;
}

bool Nurikabe::Solver::SolveBlackAtPredictableCorner()
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
				GetBoard().Print(std::cout);
				toFillWithWhite.SetState(SquareState::White);
				GetBoard().Print(std::cout);
				if (!CheckForSolvedWhites())
				{
					ret = false;
					return false;
				}
			}
		}

		return true;
	});
	return ret;
}

void Solver::SolveBalloonBlack()
{
	ForEachRegion([](const Region& black)
	{
		if (black.GetState() != SquareState::Black)
			return true;

		auto unknowns = black.Neighbours([](const Point&, const Square& sq)
		{
			return sq.GetState() == SquareState::Unknown;
		});

		std::vector<Region> unknownRegions;
		unknowns.ForEachContiguousRegion([&unknownRegions](const Region& r){ unknownRegions.push_back(r); return true; });

		for (int i = 0; i < unknownRegions.size(); i++)
		{
			Region(unknownRegions[i]).ExpandAllInline([&black, &unknownRegions, i](const Point& pt, const Square& sq)
			{
				if (sq.GetState() == SquareState::Black)
				{
					if (black.Contains(pt))
						return false;

					return true;
				}
				else if (sq.GetState() == SquareState::Unknown)
				{
					for (int j = 0; j < unknownRegions.size(); j++)
					{
						if (j == i)
							continue;

						if (unknownRegions[j].Contains(pt))
							return false;
					}
					return true;
				}
				return false;
			});

			unknownRegions[i].ExpandSingleInline([](const Point&, const Square& sq)
			{
				return sq.GetState() == SquareState::Unknown;
			});
		}

		return true;
	});
}

bool Solver::SolveGuessBlackToUnblock(int minSize)
{
	bool ret = false;
	auto& board = this->board;

	Region handled(&board);

	board.ForEachSquare([&board, minSize, &handled, &ret](const Point& pt, const Square& sq)
	{
		if (sq.GetState() != SquareState::Black)
			return true;

		if (handled.Contains(pt))
			return true;

		Region black = Region(&board, pt).ExpandAllInline(
			[](const Point&, const Square& sqInner) { return sqInner.GetState() == SquareState::Black; }
		);

		handled = Region::Union(handled, black);

		if (black.GetSquareCount() < minSize)
			return true;

		Region unknownNeighbours = black.Neighbours(
			[](const Point&, const Square& sqInner) { return sqInner.GetState() == SquareState::Unknown; }
		);
		Region blackContinuation = Region::Subtract(
			unknownNeighbours.Neighbours(
				[](const Point&, const Square& sqInner) { return sqInner.GetState() == SquareState::Black; }
			),
			black
		);

		if (blackContinuation.GetSquareCount() != 1)
			return true;

		Region skippableUnknownNeighbours = Region::Intersection(
			blackContinuation.Neighbours(
				[](const Point&, const Square& sqInner) { return sqInner.GetState() == SquareState::Unknown; }
			),
			unknownNeighbours
		);

		if (skippableUnknownNeighbours.GetSquareCount() > 0)
		{
			skippableUnknownNeighbours.SetState(SquareState::Black);
			ret = true;
			return false;
		}

		return true;
	});

	return ret;
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

bool Solver::SolveWithRules(Solver& solver, int& iteration)
{
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
			if (!solver.SolvePerSquare())
				return false;
		}
		else if (phase == 1)
		{
			solver.SolveUnreachable();
		}
		else if (phase == 2)
		{
			if (!solver.SolveUnfinishedWhiteIsland())
				return false;
		}
		else if (phase == 3)
		{
			if (!solver.SolveWhiteAtClosedBlack())
				return false;

			if (!solver.SolveBlackAtPredictableCorner())
				return false;

			if (iteration == 10)
			{
				int a = 0;
			}
			if (!solver.SolveBalloonUnconnectedWhiteSimple())
				return false;
		}
		else
		{
			break;
		}

		hasChangedInPrevLoop = (solver.board != boardIterationStart);
		if (!hasChangedInPrevLoop)
			phase++;
		else
		{
			phase = 0;
		}
	}

	return true;
}

void Solver::ForEachRegion(const std::function<bool(const Region &)>& callback)
{
	auto& board = this->board;

	Region handled(&board);

	board.ForEachSquare([&board, &handled, &callback](const Point& pt, const Square& sq)
	{
		if (sq.GetState() != SquareState::Black)
			return true;

		if (handled.Contains(pt))
			return true;

		Region black = Region(&board, pt).ExpandAllInline(
			[&sq](const Point&, const Square& sqInner) { return sqInner.GetState() == sq.GetState(); }
		);

		handled = Region::Union(handled, black);

		return callback(black);
	});
}


bool Solver::Solve(Solver& initialSolver)
{
	srand(1);

	initialSolver.Initialize();
	if (!initialSolver.CheckForSolvedWhites())
		return false;

	std::vector<Solver> solverStack;
	solverStack.push_back(initialSolver);

	int iteration = 0;
	while (true)
	{
		if (solverStack.size() == 0)
		{
			std::cout << "Unable to solve provided board" << std::endl;
			return false;
		}

		int	solverIndex;

		if (false) //solverStack.size() < 5)
			solverIndex = rand() % solverStack.size();
		else
			solverIndex = solverStack.size() - 1;

		Solver solver = solverStack[solverIndex];
		solverStack.erase(solverStack.begin() + solverIndex);

		if (!SolveWithRules(solver, iteration))
			continue;

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

		std::cout << "Iteration #" << iteration << std::endl;
		solver.board.Print(std::cout);
		std::cout << std::endl;
		
		if (!Rules::IsSolvable(solver.board) || !solver.unsolvedWhites.size())
			continue;

		SolveDiverge(solver, solverStack);
	}
	return true;
}