/**
 * @file Diablo.cpp
 * @author Arun Tejasvi Chaganty <arunchaganty@gmail.com>
 * @date 2010-02-04
 * Bot that randomly chooses a move to play with you
 */

#include "Othello.h"
#include "OthelloBoard.h"
#include "OthelloPlayer.h"
#include <cstdlib>
#include <vector>
#include <list>
#include <ctime>

#define MAXDEPTH 6

using namespace std;
using namespace Desdemona;

/**
 * A class that maintains the result of the Alfa-Beta Pruned Minimax tree
 * @field value - the evaluated value of the ABPM tree
 * @field path - a vector of pointers to nodes that contains the path to leaf(ply pruned leaf) that has the given value
 *
 */
class NodeInfo
{
public:
	int value;
	list<Move> path;
	NodeInfo(int ,list<Move> );
};
NodeInfo::NodeInfo(int value, list<Move> path)
{
	this->value = value;
	this->path = path;
}
/**
 A secondary function that must do some work to reduce the horizon effect based on the stability
 of the board position.
 */
bool deepEnough(OthelloBoard& board, int depth)
{
	if (depth >= MAXDEPTH)
		return true;
	else
		return false;
}
int evaluation(OthelloBoard& board, Turn turn)
{
	return 10;
}
/**
 * The Alfa-Beta Minimax function
 * @param : Position : the current othello
 */
NodeInfo* AlfaBetaMinimax(OthelloBoard& board, int depth, Turn player, int alfa,
		int beta)
{
	OthelloBoard posn = OthelloBoard(board);
	if (deepEnough(posn, depth))
	{
		list<Move> s;
		return new NodeInfo(evaluation(posn, player), s);
	}

	list<Move> successors = posn.getValidMoves(player);
	if (successors.empty())
	{
		list<Move> s;
		return new NodeInfo(evaluation(posn, player), s);
	}
	list<Move> bestPath;

	list<Move>::iterator it;
	for (it = successors.begin(); it != successors.end(); ++it)
	{
		//bestPath = new list<Move>;
		Move s = *it;
		posn.makeMove(player,s);
		NodeInfo* resultSucc = AlfaBetaMinimax(posn,depth + 1,other(player) , -beta, -alfa);
		int newValue = -resultSucc->value;
		if (newValue > beta)
		{
			beta = newValue;
			(resultSucc->path).push_front( *it );
			bestPath = resultSucc->path;
		}
		if (beta >= alfa)
		{
			return new NodeInfo(beta, bestPath);
		}
	}
	return new NodeInfo(beta, bestPath);

}

class Diablo: public OthelloPlayer
{
public:
	/**
	 * Initialisation routines here
	 */
	Diablo(Turn turn);
	virtual ~Diablo()
	{
	}

	/**
	 * Play something
	 */
	virtual Move play(const OthelloBoard& board);
private:
};

Diablo::Diablo(Turn turn) :
		OthelloPlayer(turn)
{
	time_t t;
	time(&t);
	srand(t);
}

Move Diablo::play(const OthelloBoard& board)
{

	list<Move> moves = board.getValidMoves(turn);
	int randNo = rand() % moves.size();
	list<Move>::iterator it = moves.begin();

	for (int i = 0; i < randNo; it++, i++);

	return *it;
}

// The following lines are _very_ important to create a bot module for Desdemona

extern "C"
{
OthelloPlayer* createBot(Turn turn)
{
	return new Diablo(turn);
}

void destroyBot(OthelloPlayer* bot)
{
	delete bot;
}
}

