/*
 * @file botTemplate.cpp
 * @author Arun Tejasvi Chaganty <arunchaganty@gmail.com>
 * @date 2010-02-04
 * @modified-by Sabarinath, Abhiram
 * @modified-date 2012-11-05
 * Template for users to create their own bots
 */

#include "Othello.h"
#include "OthelloBoard.h"
#include "OthelloPlayer.h"
#include <cstdio>
#include <vector>
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

/*
 * Simulate for different values and choose the best one
 */
#define PLY_DEPTH 5
#define MAX_NUM 600
#define MIN_NUM -600

Move FinalMove(-1,-1);
Turn ourTurn;
static OthelloBoard PrevBoard;
static bool strtGame = true;
Move PrevMove(-1,-1);

int score[10][10] = {
    {0,  0,  0,  0,  0,  0,  0,  0,  0,  0},
    {0, 65, -3,  6,  4,  4,  6, -3, 65,  0},
    {0, -3, -29, 3,  1,  1,  3, -29,-3,  0},
    {0,  6,  3,  5,  3,  3,  5,  3,  6,  0},
    {0,  4,  1,  3,  1,  1,  3,  1,  4,  0},
    {0,  4,  1,  3,  1,  1,  3,  1,  4,  0},
    {0,  6,  3,  5,  3,  3,  5,  3,  6,  0},
    {0, -3, -29, 3,  1,  1,  3, -29,-3,  0},
    {0, 65, -3,  6,  4,  4,  6, -3, 65,  0},
    {0,  0,  0,  0,  0,  0,  0,  0,  0,  0}
};

/**
 *  Each Node of the MinMax Tree is represented by the 'Node' Class
 */
class Node{
    public:
        OthelloBoard nodeBoard; 
        Turn nodeType;

        Node(OthelloBoard board, Turn type);
        void ModifyBoard(Move moveMade);
        ~Node();
};

Node::Node(OthelloBoard board, Turn type){
    this->nodeBoard = board;
    this->nodeType = type;
}

void Node::ModifyBoard(Move moveMade){
    (this->nodeBoard).makeMove(other(this->nodeType), moveMade);
}

int evaluationFunc(Node* curNode){
    //TODO: Return a value for the given Node based on different strategies.
    OthelloBoard& curBoard = curNode->nodeBoard;
    Turn myTurn = curNode->nodeType;
    Turn otherTurn = other(myTurn);

    int evalValue = 0;
    int i, j;

    for( i = 0; i < 8; ++i){
        for(j = 0; j < 8; ++j){
            if( curBoard.get(i, j) == myTurn)
                evalValue +=  score[i+1][j+1];
            else if(curBoard.get(i, j) == otherTurn)
                evalValue -= score[i+1][j+1];
        }
    }

    evalValue += curBoard.getValidMoves(myTurn).size() - curBoard.getValidMoves(otherTurn).size(); 
    int tmp = curBoard.getRedCount() - curBoard.getBlackCount();
    evalValue += (myTurn == RED ? tmp : -1*tmp); 
    return evalValue;
}

bool deepEnough(Node* root, int depth){
    //TODO: Implement the check for Horizon effect
    return depth == PLY_DEPTH;
}

/**
 * Recursive procedure implementing MinMax Tree Construction, with alpha - beta pruning
 * Pruning is done while constructing with the help of Evaluation function
 * List of all the children of a current Node can be obtained from the .getValidMoves method of OthelloBoard object
 */
int alphabetaMiniMax(Node* root, int depth, int alpha, int beta){
    if( deepEnough(root, depth)){
        return evaluationFunc(root);
    }

    list<Move> successors = (root->nodeBoard).getValidMoves(root->nodeType);
    if( successors.empty() ){
        int tmp =  (root->nodeBoard).getRedCount() - (root->nodeBoard).getBlackCount();
        return (root->nodeType == RED ? tmp : -1*tmp);  
    }

    list<Move>::iterator it;
    Node* tmp = NULL;
    int newValue = 0;

    for(it = successors.begin(); it != successors.end(); ++it){
        tmp = new Node(root->nodeBoard, other(root->nodeType)); 
        tmp->ModifyBoard(*it);
        newValue = -1*alphabetaMiniMax(tmp, depth+1, -1*beta, -1*alpha);
        if( newValue > alpha){
            alpha = newValue;
            if(depth == 0){
                FinalMove.x = it->x;
                FinalMove.y = it->y;
            }
        }
        if( alpha >= beta){
            return alpha;
        }
    }
    return alpha;
}

void getPrevMove(const OthelloBoard& board){
    int i, j;
    for( i = 0; i < 8; ++i){
        for( j = 0; j < 8; ++j){
            if(PrevBoard.get(i,j) == EMPTY && board.get(i,j) != PrevBoard.get(i,j)){
                PrevMove.x = i, PrevMove.y = j;
                return;
            }
        }
    }
}

class MyBot: public OthelloPlayer
{
    public:
        /**
         * Initialisation routines here
         * This could do anything from open up a cache of "best moves" to
         * spawning a background processing thread. 
         */
        MyBot( Turn turn );

        /**
         * Play something 
         */
        virtual Move play( const OthelloBoard& board );
    private:
};

MyBot::MyBot( Turn turn ) : OthelloPlayer( turn )
{

}

Move MyBot::play( const OthelloBoard& board )
{
    ourTurn = turn;
    if(!strtGame){
        getPrevMove(board);
        cout<<"Move by opponent : x = "<<PrevMove.x<<" y = "<<PrevMove.y<<endl;
    }
    else if(ourTurn == BLACK){
        strtGame = false;
        PrevBoard = OthelloBoard(board);
        list<Move> moveLst = PrevBoard.getValidMoves(ourTurn);
        list<Move>::iterator it = moveLst.begin();
        int randNo = (rand()%4);
        for( int i = 0; i < randNo-1; it++, i++);
        FinalMove.x = it->x, FinalMove.y = it->y;
        PrevBoard.makeMove(ourTurn, FinalMove);
        return FinalMove;
    }
    strtGame = false;
    PrevBoard = OthelloBoard(board);
    Node* root = new Node( PrevBoard, ourTurn);
    alphabetaMiniMax( root, 0, MIN_NUM, MAX_NUM);
    PrevBoard.makeMove(ourTurn, FinalMove);
    return FinalMove; 
}

// The following lines are _very_ important to create a bot module for Desdemona

extern "C" {
    OthelloPlayer* createBot( Turn turn )
    {
        return new MyBot( turn );
    }

    void destroyBot( OthelloPlayer* bot )
    {
        delete bot;
    }
}
