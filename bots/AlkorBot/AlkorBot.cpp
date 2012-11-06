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
#define MP make_pair

#define PLY_DEPTH 8

void *print_message_function( void *ptr );

Turn ourTurn;

/**
 *  Each Node of the MinMax Tree is represented by the 'Node' Class
 */
class Node{
    public:
		OthelloBoard nodeBoard; 

        int evalValue
		int bestIndex;

        Node* parent;
        Turn nodeType;

        vector<Node*> children;
		Node* bestChild;

		Node(OthelloBoard& board, Node* parent, Turn type);
		~Node();
};

Node::Node(OthelloBoard& board, Node* parent, Turn type){
    this->nodeBoard = OthelloBoard(board);
	this->valValue = ( type == ourTurn ? INT_MIN: INT_MAX);
    this->parent = parent;
	this->nodeType = type;
	this->bestChild = NULL;
	this->bestIndex = -1;
}

/**
 * Recursive procedure implementing MinMax Tree Construction, with alpha - beta pruning
 * Pruning is done while constructing with the help of Evaluation function
 * List of all the children of a current Node can be obtained from the .getValidMoves method of OthelloBoard object
 */
void constructTree(Node* root, int alpha, int beta, int depth){
	//TODO: Implement the Recursive algorithm described in the class. ( DFS )

	if(depth == PLY_DEPTH){
		root->evalValue = evaluationFunc(root);
		return ;
	}

	list<Move> allMoves = (root->nodeBoard).getValidMoves(root->nodeType);
	(root->children).resize(allMoves.size());

	vector<Node*>& tmpChildren = root->children;
	list<Move>::iterator it;
	
	int i = 0;
	Turn otherTurn = other( root->nodeType );

	for( it = allMoves.begin(); it != allMoves.end(); ++it, ++i){

		Node* newNode =  new Node( root->nodeBoard, root, otherTurn);
		newNode->nodeBoard.makeMove( otherTurn, *it);

		tmpChildren[i] = newNode;
		constructTree(newNode, alpha, beta, depth+1);

		if(root->nodeType == ourTurn ){
			if(root->evalValue < newNode->evalValue ){
				root->evalValue = newMode->evalValue;
				root->bestIndex = i;
				root->bestChild = newNode;
			}
		}
		else{
			if(root->evalValue > newNode->evalValue ){
				root->evalValue = newMode->evalValue;
				root->bestIndex = i;
				root->bestChild = newNode;
			}
		}
	}
}

int evaluationFunc(Node* curNode){
	//TODO: Return a value for the given Node based on different strategies.
	return rand()%(INT_MAX-INT_MIN);
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
	pthread_t thread1, thread2, thread3;
    char *message1 = "Thread 1";
    char *message2 = "Thread 2";
    int  iret1, iret2, iret3 ;
    
    /* Create independent threads each of which will execute function */
    int i = 1;
    while(i--)
    {
        iret1 = pthread_create( &thread1, NULL, print_message_function, (void*) message1);
        iret2 = pthread_create( &thread2, NULL, print_message_function, (void*) message2);
        
//        iret3 = pthread_create( &thread3, NULL, print_message_function, (void*) message3);
    }
}

Move MyBot::play( const OthelloBoard& board )
{
	ourTurn = turn;
	Node* root = new Node( board, NULL, ourTurn);
	ConstructTree(root, INT_MIN, INT_MAX, 0);
	return ( (board.getValidMoves(ourTurn)).get(root->bestIndex) );

	/**
	 * Testing code
	 */
    list<Move> moves = board.getValidMoves( turn );
    int randNo = rand() % moves.size();
    list<Move>::iterator it = moves.begin();
    int i = 0;
    while(i++ < 700000000);
    return *(it+randNo);
}

void *print_message_function( void *ptr )
{
     char *message;
     message = (char *) ptr;
     while(true);
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
