/*
 * @file botTemplate.cpp
 * @author Arun Tejasvi Chaganty <arunchaganty@gmail.com>
 * @date 2010-02-04
 * @modified-by Sabarinath, Abhiram
 * @modified-date 2012-11-10
 * Template for users to create their own bots
 */

#include "Othello.h"
#include "OthelloBoard.h"
#include "OthelloPlayer.h"
#include <cstdlib>
#include <algorithm>
#include <vector>
#include <list>
#include <cstring>
#include <climits>
#include <pthread.h>
using namespace std;
using namespace Desdemona;

#define ll long long
#define ull unsigned long long

#define ff first
#define ss second
#define PB push_back
#define PF push_front
#define MP make_pair

/*Always keep the depth to be an even number*/
#define PLY_DEPTH 6

/*Simulate for different values and choose the best*/
#define STABILITY_WEIGHT 10

#define PIECE_WEIGHT 10
#define CORNER_WEIGHT 800
#define CORNER_CLOSE_WEIGHT 380
#define DISC_WEIGHT 10
#define MOBILIITY_WEIGHT 7.8
#define FRONTIER_WEIGHT 7.4

#define MAX_NUM INT_MAX
#define MIN_NUM INT_MIN

/*Global Variables*/
Move FinalMove(-1, -1);
Turn ourTurn;
static OthelloBoard PrevBoard;
Move PrevMove(-1, -1);
static bool strtGame = true;

int score[10][10] = {
    {0,  0,  0,  0,  0,  0,  0,  0,  0,  0},
    {0, 20, -3,  11,  8,  8,  11, -3, 20,  0},
    {0, -3, -7, -4,  1,  1,  -4, -7, -3,  0},
    {0,  11,  -4,  2,  2,  2,  2,  -4,  11,  0},
    {0,  8,  1,  2,  -3,  -3,  2,  1,  8,  0},
    {0,  8,  1,  2,  -3,  -3,  2,  1,  8,  0},
    {0,  11,  -4,  2,  2,  2,  2,  -4,  11,  0},
    {0, -3, -7, -4,  1,  1,  -4, -7, -3,  0},
    {0, 20, -3,  11,  8,  8,  11, -3, 20,  0},
    {0,  0,  0,  0,  0,  0,  0,  0,  0,  0}
};

/**
 *  Each Node of the MinMax Tree is represented by the 'Node' Class
 */
class Node {
public:
	OthelloBoard nodeBoard;
	Turn nodeType;

	Node(OthelloBoard board, Turn type);
	void ModifyBoard(Move moveMade);
	~Node();
};

Node::Node(OthelloBoard board, Turn type) {
	this->nodeBoard = board;
	this->nodeType = type;
}

void Node::ModifyBoard(Move moveMade) {
	(this->nodeBoard).makeMove(other(this->nodeType), moveMade);
}

double evaluationFunc(Node* curNode) {
	//TODO: Return a value for the given Node based on different strategies.
	Turn myTurn = curNode->nodeType;
	Turn otherTurn = other(myTurn);

	int finalVal = 0;

	/*Coin position weight*/
	int discEvalVal = 0;
	int i, j;

	for (i = 0; i < 8; ++i) {
		for (j = 0; j < 8; ++j) {
			if ((curNode->nodeBoard).get(i, j) == myTurn)
				discEvalVal += score[i + 1][j + 1];
			else if ((curNode->nodeBoard).get(i, j) == otherTurn)
				discEvalVal -= score[i + 1][j + 1];
		}
	}

	//finalVal += DISC_WEIGHT*discEvalVal;
	finalVal = discEvalVal;
	/*Add weights to the mobility count*/
	int a = (curNode->nodeBoard).getValidMoves(myTurn).size();
	int b = (curNode->nodeBoard).getValidMoves(otherTurn).size();

	//finalVal += a-b;
	/*
	double tmp;
	if(a != b){
		tmp = 100 * ( a /(1.0 * (a+b)) );
		finalVal += MOBILIITY_WEIGHT *  ( a < b ? -1*(100-tmp) : tmp );
	}*/
	/*Add weights to the Piecewise count of Coins*/
	a = (curNode->nodeBoard).getRedCount();
	b = (curNode->nodeBoard).getBlackCount();

	//finalVal += (myTurn == RED ?  a-b : b-a);

	/*
	if(a != b){
		tmp = 100 * ( myTurn == RED ? ( a / ( 1.0 * (a+b)) ) : ( b / ( 1.0 * (a+b)) ) );
		finalVal += PIECE_WEIGHT *  ( a < b ? -1*(100-tmp) : tmp);
	}
	*/
	/*Corner occupancy check
	tmp += (curNode->nodeBoard).get(0,0) == myTurn ? 25.0 : -1*25.0;
	tmp += (curNode->nodeBoard).get(7,0) == myTurn ? 25.0 : -1*25.0;
	tmp += (curNode->nodeBoard).get(0,7) == myTurn ? 25.0 : -1*25.0;
	tmp += (curNode->nodeBoard).get(7,7) == myTurn ? 25.0 : -1*25.0;

	finalVal += CORNER_WEIGHT*tmp;

	/*Corner Closeness occupancy
	tmp += (curNode->nodeBoard).get(0,1) == myTurn ? -1*12.5 : 12.5;
	tmp += (curNode->nodeBoard).get(1,0) == myTurn ? -1*12.5 : 12.5;
	tmp += (curNode->nodeBoard).get(7,1) == myTurn ? -1*12.5 : 12.5;
	tmp += (curNode->nodeBoard).get(1,7) == myTurn ? -1*12.5 : 12.5;
	tmp += (curNode->nodeBoard).get(7,6) == myTurn ? -1*12.5 : 12.5;
	tmp += (curNode->nodeBoard).get(6,7) == myTurn ? -1*12.5 : 12.5;
	tmp += (curNode->nodeBoard).get(6,0) == myTurn ? -1*12.5 : 12.5;
	tmp += (curNode->nodeBoard).get(0,6) == myTurn ? -1*12.5 : 12.5;

	finalVal += CORNER_CLOSE_WEIGHT*tmp;
	*/
	/*Add weights to frontier cells*/
	return finalVal;
}

/**
 * Recursive procedure implementing MinMax Tree Construction, with alpha - beta pruning
 * Pruning is done while constructing with the help of Evaluation function
 * List of all the children of a current Node can be obtained from the .getValidMoves method of OthelloBoard object
 */
int alphabetaMiniMax(Node* root, int depth, int alpha, int beta) {
	if (depth == PLY_DEPTH) {
		return evaluationFunc(root);
	}

	list<Move> successors = (root->nodeBoard).getValidMoves(root->nodeType);
	/*Mobility Rule for inner nodes*/
	if (successors.empty()) {
		return evaluationFunc(root);
	}

	Node* tmp = NULL;
	int newValue = 0;

	for (list<Move>::iterator it = successors.begin(); it != successors.end();
			++it) {
		tmp = new Node(root->nodeBoard, other(root->nodeType));
		tmp->ModifyBoard(*it);
		newValue = -1 * alphabetaMiniMax(tmp, depth + 1, -1 * beta, -1 * alpha);
		if (newValue > alpha) {
			alpha = newValue;
			if (depth == 0) {
				FinalMove.x = it->x;
				FinalMove.y = it->y;
			}
		}
		if (alpha >= beta) {
			return alpha;
		}
	}
	return alpha;
}

void getPrevMove(const OthelloBoard& board) {
	int i, j;
	for (i = 0; i < 8; ++i) {
		for (j = 0; j < 8; ++j) {
			if (PrevBoard.get(i, j) == EMPTY && board.get(i, j) != PrevBoard.get(i, j)) {
				PrevMove.x = i, PrevMove.y = j;
				return;
			}
		}
	}
}

class MyBot: public OthelloPlayer {
public:
	/**
	 * Initialization routines here
	 * This could do anything from open up a cache of "best moves" to
	 * spawning a background processing thread.
	 */
	MyBot(Turn turn);

	/**
	 * Play something
	 */
	virtual Move play(const OthelloBoard& board);
private:
};

MyBot::MyBot(Turn turn) : OthelloPlayer(turn) {

}

Move MyBot::play(const OthelloBoard& board) {
	ourTurn = turn;

	if (!strtGame) {
		getPrevMove(board);
	} else if (ourTurn == BLACK) {
		strtGame = false;
		PrevBoard = OthelloBoard(board);

		list<Move> moveLst = PrevBoard.getValidMoves(ourTurn);
		list<Move>::iterator it = moveLst.begin();
		int randNo = (rand() % 4);
		for (int i = 0; i < randNo - 1; ++it, ++i);

		FinalMove.x = it->x, FinalMove.y = it->y;
		PrevBoard.makeMove(ourTurn, FinalMove);

		return FinalMove;
	}
	strtGame = false;
	PrevBoard = OthelloBoard(board);
	Node* root = new Node(PrevBoard, ourTurn);
	alphabetaMiniMax(root, 0, MIN_NUM, MAX_NUM);
	PrevBoard.makeMove(ourTurn, FinalMove);
	return FinalMove;
}

// The following lines are _very_ important to create a bot module for Desdemona

extern "C" {
OthelloPlayer* createBot(Turn turn) {
	return new MyBot(turn);
}

void destroyBot(OthelloPlayer* bot) {
	delete bot;
}
}
