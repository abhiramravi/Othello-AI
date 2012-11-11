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

/*Simulate for different values and choose the best*/
#define STABILITY_WEIGHT 100

#define PIECE_WEIGHT 1
#define DISC_WEIGHT 25
#define MOBILIITY_WEIGHT 1

#define MAX_NUM LONG_MAX
#define MIN_NUM LONG_MIN

/*Global Variables*/

/*Always keep the depth to be an even number ( preferably) */
int PLY_DEPTH = 6;

static int gameMovesDone = 2;

/* The best that can be made by our bot*/
Move FinalMove(-1, -1);

/* Coin Color of our Move*/
static Turn ourTurn;

/* The Board copy after making the FinalMove*/
static OthelloBoard PrevBoard;

/* Move made by the opponent*/
Move PrevMove(-1, -1);

/* Flag to check whether the first move is done*/
static bool strtGame = true;

/*Number of immediate Successors Explored*/
static int threadDone;

/*Number of level 2 successors explored completely*/
static int threadDone2;

/*Flag for level 2 Successors Explored*/
static bool thread2Done[32];

/*List of Immediate Moves*/
static list<Move> immediateSucc;

/*List of lists : of all level 1 Successors */
static list<Move> moveLst1[32];

/*List of lists : of all level 2 Successors */
static list<Move> moveLst2[32][32];

/* The thread that RGs the opponent */
static pthread_t RGThread;
static int RGThreadStatus;

/*Check for Thread completeness*/
bool threadSuccessfull;

/*Position of the Move Made by the opponent*/
int pos;

/*Move is made by opponent or also not passed by me*/
bool OpponentMoveDone;

int score[10][10] = {
    {0,  0,  0,  0,  0,  0,  0,  0,  0,  0},
    {0, 30, -3,  21,  8,  8,  21, -3, 30,  0},
    {0, -3, -17, -4,  1,  1,  -4, -17, -3,  0},
    {0,  21,  -4,  2,  2,  2,  2,  -4,  21,  0},
    {0,  8,  1,  2,  -3,  -3,  2,  1,  8,  0},
    {0,  8,  1,  2,  -3,  -3,  2,  1,  8,  0},
    {0,  21,  -4,  2,  2,  2,  2,  -4,  21,  0},
    {0, -3, -17, -4,  1,  1,  -4, -17, -3,  0},
    {0, 30, -3,  21,  8,  8,  21, -3, 30,  0},
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

bool isDiagonalsFilled(int i, int j, OthelloBoard& curBoard )
{
	int k = i, l = j;
	while(0 <= k && k < 8 && 0 <= l && l < 8){
		if(curBoard.get(k,l) == EMPTY)
			return false;
		k++, l++;
	}
	k = i, l = j;
	while(0 <= k && k < 8 && 0 <= l && l < 8){
		if(curBoard.get(k,l) == EMPTY)
			return false;
		k++, l--;
	}
	k = i, l = j;
	while(0 <= k && k < 8 && 0 <= l && l < 8){
		if(curBoard.get(k,l) == EMPTY)
			return false;
		k--, l++;
	}
	k = i, l = j;
	while(0 <= k && k < 8 && 0 <= l && l < 8){
		if(curBoard.get(k,l) == EMPTY)
			return false;
		k--, l--;
	}
	return true;
}

/* Currently implementing stability as a separate function. Later integrate into evaluation to save time */
int stabilityFactor(Node* curNode)
{
	OthelloBoard& curBoard = curNode->nodeBoard;
	Turn myTurn = curNode->nodeType;
	Turn otherTurn = other(myTurn);

	bool filledRows[8];
	bool filledCols[8];

	bool check ;
	for (int i = 0; i < 8; i++){
		check = true;
		for (int j = 0; j < 8; j++){
			if (curBoard.get(i, j) == EMPTY)
				check = false;
		}
		if (check)
			filledRows[i] = true;
	}
	for (int j = 0; j < 8; j++){
		check = true;
		for (int i = 0; i < 8; i++){
			if (curBoard.get(i, j) == EMPTY)
				check = false;
		}
		if (check)
			filledCols[j] = true;
	}
	int stability = 0;
	for( int i = 0; i < 8; i ++){
		for(int j = 0; j < 8; j ++){
			if(filledRows[i] && filledCols[j] && isDiagonalsFilled(i,j,curBoard) && myTurn == curBoard.get(i, j))
				stability++;
			if(filledRows[i] && filledCols[j] && isDiagonalsFilled(i,j,curBoard) && otherTurn == curBoard.get(i, j))
				stability--;
		}
	}
	if( filledRows[0] && myTurn == curBoard.get(0, 0) && curBoard.get(0, 0) == myTurn) stability += 8;
	if( filledRows[7] && myTurn == curBoard.get(7, 0) && curBoard.get(0, 0) == myTurn) stability += 8;
	if( filledCols[0] && myTurn == curBoard.get(0, 7) && curBoard.get(0, 0) == myTurn) stability += 8;
	if( filledCols[7] && myTurn == curBoard.get(7, 7) && curBoard.get(0, 0) == myTurn) stability += 8;
	if( filledRows[0] && myTurn == curBoard.get(0, 0) && curBoard.get(0, 0) == otherTurn) stability -= 8;
	if( filledRows[7] && myTurn == curBoard.get(7, 0) && curBoard.get(0, 0) == otherTurn) stability -= 8;
	if( filledCols[0] && myTurn == curBoard.get(0, 7) && curBoard.get(0, 0) == otherTurn) stability -= 8;
	if( filledCols[7] && myTurn == curBoard.get(7, 7) && curBoard.get(0, 0) == otherTurn) stability -= 8;

	return stability;
}

double evaluationFunc(Node* curNode) {
	//TODO: Return a value for the given Node based on different strategies.
	Turn myTurn = curNode->nodeType;
	Turn otherTurn = other(myTurn);

	double finalVal = 0.0;

	/*Coin position weight*/
	int discEvalVal = 0;
	int i, j, a, b;

	for (i = 0; i < 8; ++i) {
		for (j = 0; j < 8; ++j){
			if ((curNode->nodeBoard).get(i, j) == myTurn)
				discEvalVal += score[i + 1][j + 1];
			else if ((curNode->nodeBoard).get(i, j) == otherTurn)
				discEvalVal -= score[i + 1][j + 1];
		}
	}

	finalVal += DISC_WEIGHT*discEvalVal;

	/*Add weights to the mobility count
	a = (curNode->nodeBoard).getValidMoves(myTurn).size();
	b = (curNode->nodeBoard).getValidMoves(otherTurn).size();
	finalVal += MOBILIITY_WEIGHT*(a-b);*/

	/*Add weights to the Piecewise count of Coins*/
	a = (curNode->nodeBoard).getRedCount();
	b = (curNode->nodeBoard).getBlackCount();
	finalVal += PIECE_WEIGHT*( myTurn == RED ? (a-b) : (b-a) );

	/*Add weights to frontier cells*/
	return finalVal + (stabilityFactor(curNode) * STABILITY_WEIGHT);
}

/**
 * Recursive procedure implementing MinMax Tree Construction, with alpha - beta pruning
 * Pruning is done while constructing with the help of Evaluation function
 * List of all the children of a current Node can be obtained from the .getValidMoves method of OthelloBoard object
 */
double alphabetaMiniMax(Node* root, int depth, double alpha, double beta, int ind) {
	if (depth == PLY_DEPTH) {
		return evaluationFunc(root);
	}

	list<Move> successors = ( OpponentMoveDone && depth == 0 && threadSuccessfull) ? moveLst1[pos]:
			( ( OpponentMoveDone && depth == 1 && thread2Done[ind] && threadSuccessfull )? moveLst2[pos][ind] :
					(root->nodeBoard).getValidMoves(root->nodeType) );
	/*Mobility Rule for inner nodes*/
	if (successors.empty()) {
		return evaluationFunc(root);
	}

	Node* tmp = NULL;
	double newValue = 0.0;
	int i = 0;
	for (list<Move>::iterator it = successors.begin(); it != successors.end(); ++it, ++i) {
		tmp = new Node(root->nodeBoard, other(root->nodeType));
		tmp->ModifyBoard(*it);
		newValue = -1 * alphabetaMiniMax(tmp, depth + 1, -1 * beta, -1 * alpha, i);
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

void* threadFunc(void* ptr){
	int i, j;
	threadDone = threadDone2 = -1;
	memset(thread2Done,false, sizeof(thread2Done));
	Turn otherTurn = other(ourTurn);
	immediateSucc = PrevBoard.getValidMoves(otherTurn);
	list<Move>::iterator it, kj;
	OthelloBoard saveBoard = PrevBoard;

	for(i = 0, it = immediateSucc.begin(); it != immediateSucc.end(); ++it, ++i){
		PrevBoard.makeMove(otherTurn, *it);
		moveLst1[i] = PrevBoard.getValidMoves(ourTurn);
		PrevBoard = saveBoard;
		++threadDone;
	}

	for(i = 0, kj = immediateSucc.begin() ; kj != immediateSucc.end(); ++kj, ++i ){
		PrevBoard.makeMove(otherTurn, *kj);
		OthelloBoard saveBoard1 = PrevBoard;
		for( j = 0, it = moveLst1[i].begin(); it != moveLst1[i].end(); ++it, ++j){
			PrevBoard.makeMove(ourTurn, *it);
			moveLst2[i][j] = PrevBoard.getValidMoves(otherTurn);
			PrevBoard = saveBoard1;
		}
		thread2Done[i] = true;
		++threadDone2;
		PrevBoard = saveBoard;
	}
	return NULL;
}

int getIndex(){
	list<Move> moveLst = PrevBoard.getValidMoves(other(ourTurn));
	int i = 0;
	for(list<Move>::iterator it = moveLst.begin(); it != moveLst.end(); ++it, ++i){
		if(it->x == PrevMove.x && it->y == PrevMove.y)
			return i;
	}
	return -1;
}

void getPrevMove(const OthelloBoard& board) {
	int i, j;
	OpponentMoveDone = true;
	int numberOfMoves = 0;
	for (i = 0; i < 8; ++i) {
		for (j = 0; j < 8; ++j) {
			if (PrevBoard.get(i, j) == EMPTY && board.get(i, j) != PrevBoard.get(i, j)) {
				PrevMove.x = i, PrevMove.y = j;
				++numberOfMoves;
				if(numberOfMoves > 1){
					OpponentMoveDone = false;
					return;
				}
			}
		}
	}
	if(numberOfMoves == 0){
		OpponentMoveDone = false;
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
	gameMovesDone += 2;
	if(gameMovesDone < 10 || gameMovesDone > 50)
		PLY_DEPTH = 8;
	else if(gameMovesDone < 13 || gameMovesDone > 45)
		PLY_DEPTH = 7;
	else
		PLY_DEPTH = 6;
	if (!strtGame) {
		pthread_join(RGThread, NULL);
		getPrevMove(board);
		if(OpponentMoveDone){
			pos = getIndex();
			cout<<"Opponent Move : "<<pos<<" ThreadDone: "<<threadDone<<endl;
			cout<<"Level 2 : "<<threadDone2<<endl;
			if(pos > threadDone || pos == -1){
				/*TODO: make ordinary alpha-beta pruning*/
				threadSuccessfull = false;
			}
			else{
				threadSuccessfull = true;
				/*Use the moveLst[pos] for the immediate Successors*/
			}
		}
	}
	else if (ourTurn == BLACK) {
		strtGame = false;
		PrevBoard = OthelloBoard(board);

		list<Move> moveLst = PrevBoard.getValidMoves(ourTurn);
		list<Move>::iterator it = moveLst.begin();
		int randNo = (rand() % 4);
		for (int i = 0; i < randNo - 1; ++it, ++i);

		FinalMove.x = it->x, FinalMove.y = it->y;
		PrevBoard.makeMove(ourTurn, FinalMove);

		/* Initializing the thread to RG opponenet before returning the move */
		RGThreadStatus = pthread_create(&RGThread, NULL, threadFunc, (void*)NULL);

		return FinalMove;
	}
	strtGame = false;

	PrevBoard = OthelloBoard(board);
	Node* root = new Node(PrevBoard, ourTurn);
	alphabetaMiniMax(root, 0, MIN_NUM, MAX_NUM, 0);
	PrevBoard.makeMove(ourTurn, FinalMove);
	/* Initializing the thread to RG opponenet before returning the move */
	RGThreadStatus = pthread_create(&RGThread, NULL, threadFunc, (void*)NULL);

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
