#ifndef STUDENTWORLD_H_
#define STUDENTWORLD_H_

//include statements for GameWorld and Board
#include "GameWorld.h"
#include "Board.h"
#include <string>

//include statement for STL Collection of type list
#include <algorithm>
#include <vector>

//include statements for Actor
#include "Actor.h"

//other includes
#include <sstream> 
#include <iostream>
#include <iomanip> 
using namespace std;

class PlayerName;

const int GIVE_COINS = 3;
const int TAKE_COINS = -3;
const int COINS = 0;
const int STARS = 1;

class StudentWorld : public GameWorld
{	
	public:
		StudentWorld(std::string assetPath);
		~StudentWorld();

		virtual int init();
		virtual int move();
		virtual void cleanUp();

		//to be used my Actor class/subclasses
		Board* getBoard() const;
		bool continueMoving(int x, int y);
		void findRandomSquareCoords(int& x, int& y);
		void swapPlayers(PlayerAvatar* player);
		void swapPlayerContents(int action);
		
		//for vortex
		bool checkOverlap(int x, int y);
		void addVortex(int x, int y, int firingDir);

		//for Bowser
		void createDroppingSquare(int x, int y);

		//GETTERS/ SETTERS
		PlayerAvatar* getPeach() const;
		PlayerAvatar* getYoshi() const;
		int getBankBalance() const;
		void setBankBalance(int amt);

	private:
		//pointers to PlayerAvatar and list of actors
		PlayerAvatar* m_peach;
		PlayerAvatar* m_yoshi;
		std::vector<Actor*> m_actor_list;

		//vector helpers
		bool checkOverlapHelper(int vX, int vY, int objX, int objY);

		//bank account
		int m_bank_balance;

		//current board
		void populate();
		Board* m_curr_board;
		int m_boardNum;

		//other helper
		PlayerAvatar* determineWinner();
		string generateGameStat();
		string statForPlayer(const PlayerAvatar& player);
};

#endif // STUDENTWORLD_H_
