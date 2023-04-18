#ifndef ACTOR_H_
#define ACTOR_H_

#include "GraphObject.h"

class StudentWorld;

//ACTIVITY CONSTANTS
const int STATUS_ALIVE = 0;
const int STATUS_DEAD = 1;
const int STATUS_WAITING_TO_ROLL = 2;
const int STATUS_WALKING = 3;
const int STATUS_IMPACTED = 4;

class PlayerAvatar; //to allow landOn_moveOver to compile
//ACTOR
class Actor: public GraphObject
{
    public: 
        Actor (StudentWorld* world, int imageID, int startX, int startY, int dir, int depth, double size);

        //PURE VIRTUAL FUNCTIONS
        virtual void doSomething() = 0;
        virtual bool isSquare() = 0;
        virtual bool isImpactable() = 0;

        //GETTERS AND SETTERS
        StudentWorld* getWorld() const;
        void setActivityStatus (int newStatus);
        int getActivityStatus() const;

        //landing on
        bool landOn_moveOver(PlayerAvatar* player, bool (*helper) (int));

    private:
        StudentWorld* m_currentWorld;
        int m_status;
};

//MOVING AVATAR CONSTANTS
const int PLAYER_1 = 1;
const int PLAYER_2 = 2;
const int PIXELS_TO_MOVE = 2;
//MOVING AVATAR
class MovingAvatar: public Actor
{
    public:
        MovingAvatar(StudentWorld* world, int imageID, int startX, int startY, int depth);

        virtual bool isSquare()
        {
            return false;
        }
        virtual void doSomething() = 0;
        void beingTeleported();

        //GETTERS/ SETTERS
        void setWalkDirection(int newDir);
        int getWalkDirection() const;
        void setDirectionalStatus(bool status);
        bool getDirectionalStatus() const;
        void setTeleportStatus(bool status);
        bool getTeleportStatus() const;
    

    protected:
        //MOVEMENT
        bool canMove(int) const;
        int findNewDirection();
        bool checkAtFork() const;
        bool onSquare() const;
        void updateDir(int actionType, int& newDir);
        bool verifyDir(int possibleDir);
        bool newForkDir(int possibleDir);
        
    private: 
        int m_walk_direction;
        bool m_onDirectional;
        bool m_isTeleported;
};

//PLAYER AVATAR
class PlayerAvatar: public MovingAvatar
{
    public:
        PlayerAvatar(int playerNum, StudentWorld* world, int imageID, int startX, int startY);

        //GETTERS/ SETTERS
        int getCoins() const;
        void setCoins(int newAmt);
        int getStars() const;
        void setStars(int newAmt);
        int getRollNum() const;
        void setRollNum(int newAmt);
        int getTicksLeft() const;
        void setTicksLeft(int newAmt);
        bool getVortex() const;
        void setVortex(bool status);

        //FOR DEALING WITH OTHER OBJECTS
        int getStationaryTicks() const;
        void setStationaryTicks(int newAmt);

        //DEALING WITH COINS
        void affectCoins(int amount);

        //DEALING WITH EVENTS
        void swap(PlayerAvatar& other);

        //DEALING WITH BADDIES
        void swapContents(PlayerAvatar& other, int action);

        virtual void doSomething();
        virtual bool isImpactable()
        {
            return false;
        }

    private:
        //helper functions
        int actionResult(int actionType);
        void assignVortexCoords(int& x, int& y);

        //variables
        int m_playerNum;
        int m_ticks_to_move;
        int m_die_roll;        
        int m_coins;
        int m_stars;
        bool m_vortex;
        int m_stationary_ticks;
};

//BADDIES CONSTANTS
const bool WALKING = true;
const bool PAUSED = false;
//BADDIES
class Baddies: public MovingAvatar
{
    public:
        Baddies(StudentWorld* world, int imageID, int startX, int startY);

        //for each Baddie to do something
        virtual void doSomething();
        virtual bool isImpactable()
        {
            return true;
        }

    protected:
        //stuff to do when paused
        virtual void doSpecialPausedActivity(PlayerAvatar* player) = 0;

        //stuff to do when walking
        void whenWalking();
        virtual int baddieRollAmount() = 0; 
        virtual void doSpecialWalkingActivity() = 0;
        
    private:
        bool m_isWalking;
        int m_pause_counter;
        int m_squares_to_move;
        int m_ticks_to_move;

        void checkPlayerOverlap(PlayerAvatar* player);

};

//BOWSERS
class Bowser: public Baddies
{
    public: 
        Bowser(StudentWorld* world, int imageID, int startX, int startY);

    private:
        virtual void doSpecialPausedActivity(PlayerAvatar* player);
        virtual int baddieRollAmount();
        void doSpecialWalkingActivity();
};

//BOOS
class Boo: public Baddies
{
    public: 
        Boo(StudentWorld* world, int imageID, int startX, int startY);

    private:
        virtual void doSpecialPausedActivity(PlayerAvatar* player);
        virtual int baddieRollAmount();
        void doSpecialWalkingActivity();
};

//VORTEX
class Vortex: public MovingAvatar
{
    public:
        Vortex(int firingDir, StudentWorld* world, int imageID, int startX, int startY);

        virtual void doSomething();
        virtual bool isImpactable()
        {
            return false;
        }

    private:
        bool offScreen();
};

//SQUARES
class Square: public Actor
{
    public:
        Square(StudentWorld* world, int imageID, int startX, int startY, int dir);

        virtual bool isSquare()
        {
            return true;
        }
        virtual bool isImpactable()
        {
            return false;
        }
         
        //for each square to do something
        void doSomething();
        virtual void doSpecialActivity(PlayerAvatar* player) = 0;

    private:
};

//COIN SQUARE
class CoinSquare: public Square
{
    public:
        CoinSquare(int grantAmount, StudentWorld* world, int imageID, int startX, int startY);
    
    private:
        //helper functions
        void doSpecialActivity(PlayerAvatar* player);
        
        //variables
        int m_grant_amount;
};

//DIRECTIONAL SQUARE
class DirectionalSquare: public Square
{
    public:
        DirectionalSquare(StudentWorld* world, int imageID, int startX, int startY, int dir);

    private:
        void doSpecialActivity(PlayerAvatar* player);

        int m_forcingDir;
};

//STAR SQUARE CONSTANTS
const int STAR_AMOUNT = 20;
//STAR SQUARE
class StarSquare: public Square
{
    public:
        StarSquare(StudentWorld* world, int imageID, int startX, int startY);

    private:
        void doSpecialActivity(PlayerAvatar* player);
};

//BANK SQUARE CONSTANTS
const int BANK_DEDUCTION_AMOUNT = 5;
//BANK SQUARE
class BankSquare: public Square
{
    public:
        BankSquare(StudentWorld* world, int imageID, int startX, int startY);

    private:
        void doSpecialActivity(PlayerAvatar* player);
        void giveAllCoins(PlayerAvatar* player);
        void takeSomeCoins(PlayerAvatar* player);
};

//EVENT SQUARE CONSTANTS
const int TELEPORT = 1;
const int SWAP = 2;
const int GIVE_VORTEX = 3;
//EVENT SQUARE
class EventSquare: public Square
{
    public:
        EventSquare(StudentWorld* world, int imageID, int startX, int startY);

    private:
        void doSpecialActivity(PlayerAvatar* player);
        void givePlayerVortex(PlayerAvatar* player);
};

//DROPPING SQUARE CONSTANTS
const int DROPPING_DEDUCTION_AMOUNT = 10;
//DROPPING SQUARE
class DroppingSquare: public Square
{
    public:
        DroppingSquare(StudentWorld* world, int imageID, int startX, int startY);
    
    private:
        void doSpecialActivity(PlayerAvatar* player);
        void deductCoins(PlayerAvatar* player);
        void deductStar(PlayerAvatar* player);
};
#endif // ACTOR_H_