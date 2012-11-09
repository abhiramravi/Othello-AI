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
#define PLY_DEPTH 6
#define MAX_NUM -600
#define MIN_NUM 600

typedef pair< int, list<Move>* > pil;

Turn ourTurn;
static Turn strtTurn = EMPTY;
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

        int evalValue;
        Node* parent;
        Turn nodeType;

		Node(OthelloBoard board, Node* parent, Turn type);
        void ModifyBoard(Move moveMade);
		~Node();
};

Node::Node(OthelloBoard board, Node* parent, Turn type){
    this->nodeBoard = board;
	this->evalValue = -1;
    this->parent = parent;
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
    //cout<<myTurn<<endl;
    
    int count = 0;
    int i, j;

    for( i = 0; i < 8; ++i){
        for(j = 0; j < 8; ++j){
            if( curBoard.get(i, j) == myTurn)
               count +=  score[i+1][j+1];
            else if(curBoard.get(i, j) == otherTurn)
                count -= score[i+1][j+1];
        }
    }

    count += curBoard.getValidMoves(myTurn).size() - curBoard.getValidMoves(otherTurn).size(); 
    int tmp = curBoard.getRedCount() - curBoard.getBlackCount();
    count += (myTurn == RED ? tmp : -1*tmp); 
    return count;
}

bool deepEnough(Node* root, int depth){
    //TODO: Implement the check for Horizon effect
    return depth == PLY_DEPTH;
}

list<Move>* getCopy(list<Move>* lst){
    list<Move>* to_ret = new list<Move>;
    list<Move>::iterator it;
    for( it = lst->begin(); it != lst->end(); ++it){
        to_ret->PB(*it);
    }
    return to_ret;
}

/**
 * Recursive procedure implementing MinMax Tree Construction, with alpha - beta pruning
 * Pruning is done while constructing with the help of Evaluation function
 * List of all the children of a current Node can be obtained from the .getValidMoves method of OthelloBoard object
 */
pil alphabetaMiniMax(Node* root, int depth, int alpha, int beta){
    if( deepEnough(root, depth)){
        return MP(evaluationFunc(root), new list<Move> );
    }

//    printf("Level %d alpha : %d beta %d\n", depth, alpha, beta);
     
    list<Move> successors = (root->nodeBoard).getValidMoves(root->nodeType);
    if( successors.empty() ){
        return MP(evaluationFunc(root), new list<Move>);
    }
    
    list<Move>::iterator it;
    pil resultSucc;
    Node* tmp = NULL;
    int newValue = 0;
    list<Move>* bestPath = new list<Move>;
    for(it = successors.begin(); it != successors.end(); ++it){
  //      cout <<"Move is x = "<<it->x<<" y = "<<it->y<<endl;
        tmp = new Node(root->nodeBoard, root, other(root->nodeType)); 
        tmp->ModifyBoard(*it);
    //    cout<<"Modified"<<endl;
        resultSucc = alphabetaMiniMax(tmp, depth+1, -1*beta, -1*alpha);
    //    cout<<"Depth : "<<depth<<" value: "<<resultSucc.ff<<endl;
        
        newValue = -1*resultSucc.ff;
        if( newValue > beta ){
            beta = newValue;
            bestPath = getCopy(resultSucc.ss);
            bestPath->PF(*it);
        }
        if( beta >= alpha){
            return MP(beta, bestPath);
        }
    }
    
    return MP(beta, bestPath);
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
    if(strtTurn == EMPTY){
        strtTurn = ourTurn;
    }
//    cout<<"Whose turn: "<<ourTurn<<endl;
	Node* root = new Node( OthelloBoard(board), NULL, ourTurn);
    pil tmp = alphabetaMiniMax( root, 0, MIN_NUM, MAX_NUM);
//    cout<<"Size of list : "<<(tmp.ss)->size()<<endl;
//    cout<<"Start turn: "<<strtTurn<<endl;
    return (tmp.ss)->front();
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
