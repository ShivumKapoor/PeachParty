#include "Actor.h"
#include "StudentWorld.h"

//HELPER FUNCTIONS
static bool landOnHelper(int roll)
{
    return roll == 0;
}
static bool moveOverHelper(int roll)
{   
    return roll > 0;
}

//ACTOR
Actor::Actor(StudentWorld* world, int imageID, int startX, int startY, int dir, int depth, double size)
    :GraphObject(imageID, startX * SPRITE_WIDTH, startY * SPRITE_HEIGHT, dir, depth, size)
{
    m_currentWorld = world;
    m_status = -1;
}

StudentWorld* Actor::getWorld() const
{
    return m_currentWorld;
}

void Actor::setActivityStatus(int newStatus)
{
    m_status = newStatus;
}

int Actor::getActivityStatus() const
{
    return m_status;
}

bool Actor::landOn_moveOver(PlayerAvatar* player, bool (*helper) (int))
{
    int x = player->getX();
    int y = player->getY();
    int roll = player->getRollNum();

    if (helper(roll) && getX() == x && getY() == y)
    {
        return true;
    }

    return false;
}

//MOVING AVATAR
MovingAvatar::MovingAvatar(StudentWorld* world, int imageID, int startX, int startY, int depth)
    :Actor(world,imageID, startX, startY, right, depth, 1.0)
{
    m_walk_direction = right;
    m_onDirectional = false;
    m_isTeleported = false;
}

bool MovingAvatar::canMove(int walkDir) const
{
    int newX = getX();
    int newY = getY();
    getPositionInThisDirection(walkDir, SPRITE_WIDTH, newX, newY);
    return getWorld()->continueMoving(newX, newY);
}

int MovingAvatar::findNewDirection()
{
    if (m_walk_direction == up || m_walk_direction == down)
    {
        //check going right
        if (canMove(right))
        {
            setDirection(right);
            return right;
        } else //moving left
        {
            setDirection(left);
            return left;
        }
    } else
    {
        //check moving up
        if (canMove(up))
        {
            setDirection(right);
            return up;
        } else //moving down
        {
            setDirection(right);
            return down;
        }
    }


}

void MovingAvatar::setWalkDirection(int newDir)
{
    m_walk_direction = newDir;
}

int MovingAvatar::getWalkDirection() const
{
    return m_walk_direction;
}

void MovingAvatar::setDirectionalStatus(bool status)
{
    m_onDirectional = status;
}

bool MovingAvatar::getDirectionalStatus() const
{
    return m_onDirectional;
}

bool MovingAvatar::onSquare() const
{
    if (getX() % SPRITE_WIDTH == 0 && getY() % SPRITE_HEIGHT == 0)
    {
        return true;
    }

    return false;
}

bool MovingAvatar::checkAtFork() const
{
    if (onSquare())
    {
        int numPaths = 0;

        int newX;
        int newY;

        for (int i = 0; i < 360; i += 90)
        {
            newX = getX();
            newY = getY();
            getPositionInThisDirection(i, SPRITE_WIDTH, newX, newY);

            if (getWorld()->continueMoving(newX, newY))
            {
                numPaths++;
            }
            if (numPaths > 2)
            {
                return true;
            }
        }
    }

    return false;
}

void MovingAvatar::updateDir(int actionType, int& newDir)
{
    switch(actionType)
    {
        case ACTION_RIGHT:
            newDir = right;
            break;
        case ACTION_UP:
            newDir = up;
            break;
        case ACTION_LEFT:
            newDir = left;
            break;
        case ACTION_DOWN:
            newDir = down;
            break;
    }
}

bool MovingAvatar::verifyDir(int possibleDir)
{
    int baackWardsDir = possibleDir;
    baackWardsDir -= 180;

    if (baackWardsDir >= 360)
    {
        baackWardsDir -= 360;
    } else if (baackWardsDir < 0)
    {
        baackWardsDir += 360;
    }

    if (baackWardsDir != getWalkDirection())
    {
        int newX = getX();
        int newY = getY();
        getPositionInThisDirection(possibleDir, SPRITE_WIDTH, newX, newY);
        if (getWorld()->continueMoving(newX, newY))
        {
            return true;
        }
    } 
    
    return false;
}

bool MovingAvatar::newForkDir(int possibleDir)
{
    if (verifyDir(possibleDir)) //verifying potential walking direction
    {
        setWalkDirection(possibleDir);
        if (getWalkDirection() == left)
        {
            setDirection(left);
        } else
        {
            setDirection(right);
        }

        return true;
    } else
    {
        return false; //invalid walking direction
    }
}

void MovingAvatar::beingTeleported()
{
    int newX;
    int newY;

    getWorld()->findRandomSquareCoords(newX, newY); //find coordinates of a random square
    moveTo(newX, newY);

    m_isTeleported = true;
}

void MovingAvatar::setTeleportStatus(bool status)
{
    m_isTeleported = status;
}

bool MovingAvatar::getTeleportStatus() const
{
    return m_isTeleported;
}

//PLAYER AVATAR
PlayerAvatar::PlayerAvatar(int playerNum, StudentWorld* world, int imageID, int startX, int startY)
    :MovingAvatar(world, imageID, startX, startY, 0)
{
    m_playerNum = playerNum;
    m_ticks_to_move = 0;
    m_die_roll = 0;
    m_stars = 0;
    m_coins = 0;
    m_stationary_ticks = 0;
    m_vortex = false;
    setActivityStatus(STATUS_WAITING_TO_ROLL);
}

int PlayerAvatar::actionResult (int actionType)
{
    switch (actionType)
    {
        case ACTION_LEFT:
            return ACTION_LEFT;
            break; 

        case ACTION_RIGHT:
            return ACTION_RIGHT;
            break; 

        case ACTION_UP:
            return ACTION_UP;
            break;

        case ACTION_DOWN:
            return ACTION_DOWN;
            break;

        case ACTION_ROLL:
            m_die_roll = randInt(1, 10);
            m_ticks_to_move = m_die_roll * 8;
            setActivityStatus(STATUS_WALKING);
            return ACTION_ROLL;
            break;

        case ACTION_FIRE:
            return ACTION_FIRE;
            break;
    }

    return ACTION_NONE;
}

void PlayerAvatar::assignVortexCoords(int& x, int& y)
{
    switch(getWalkDirection())
    {
        case right:
            x += SPRITE_WIDTH;
            break;
        case up:
            y += SPRITE_HEIGHT;
            break;
        case left:
            x -= SPRITE_WIDTH;
            break;
        case down:
            y -= SPRITE_HEIGHT;
            break;
    }
}

void PlayerAvatar::doSomething()
{
    if (getActivityStatus() == STATUS_WAITING_TO_ROLL)
    {
        m_die_roll = 0;
        m_stationary_ticks++;

        //CHECK IF USER HAS INVALID DIRECTION DUE TO TELEPORTATION
        if (getTeleportStatus() && !canMove(getWalkDirection())) //can't go forward in the right direction
        {
            setWalkDirection(findNewDirection());
            setTeleportStatus(false);
        }

        //CHECK WHAT THE USER'S ACTION IS
        int actionType = getWorld()->getAction(m_playerNum);
        int actionRes = actionResult(actionType);
        if (actionRes == ACTION_FIRE && m_vortex == true)
        {
            //create vortex coords
            int vortexX = getX();
            int vortexY = getY();
            assignVortexCoords(vortexX, vortexY);

            //add a vortex to the board and remove from player
            getWorld()->addVortex(vortexX, vortexY, getWalkDirection());
            getWorld()->playSound(SOUND_PLAYER_FIRE);
            setVortex(false);

        } else if (actionRes == ACTION_NONE)
        {
            return;
        }
    }

    if (getActivityStatus() == STATUS_WALKING)
    {
        m_stationary_ticks = 0;

        //CHECK IF USER IS AT A FORK
        if (checkAtFork() && !getDirectionalStatus()) //check at a fork
        {
            int actionType = getWorld()->getAction(m_playerNum);

            if (actionType != ACTION_NONE && actionType != ACTION_FIRE && actionType != ACTION_ROLL ) //if the user picked a valid action
            {
                int possibleDir;
                updateDir(actionType, possibleDir);

                if (!newForkDir(possibleDir)) //update walking direction/ sprite direction
                {
                    return;
                }
            } else
            {
                return; //invalid action choice
            }
        } else
        {
            //CHECK IF THE USER CAN'T MOVE IN CURRENT DIRECTION 
            if (!canMove(getWalkDirection())) //if you can't move forward in the right direction
            {
                setWalkDirection(findNewDirection());
            }
        }

        //THE USER MOVES FORWARD
        moveAtAngle(getWalkDirection(), PIXELS_TO_MOVE);

        if (onSquare())
        {
            m_die_roll--;
            if (m_die_roll < 0)
            {
                m_die_roll = 0;
            }
        }
        m_ticks_to_move--;
        if (m_ticks_to_move == 0 || m_die_roll == 0)
        {
            setActivityStatus(STATUS_WAITING_TO_ROLL);
            m_ticks_to_move = 0;
        }

        setDirectionalStatus(false);

    }
}

void PlayerAvatar::swap(PlayerAvatar& other)
{
    //swap positions
    int x = getX();
    int y = getY();
    moveTo(other.getX(), other.getY());
    other.moveTo(x, y);

    //swap ticks to move
    int i = getTicksLeft();
    setTicksLeft(other.getTicksLeft());
    other.setTicksLeft(i);

    //swap walking directions
    i = getWalkDirection();
    setWalkDirection(other.getWalkDirection());
    other.setWalkDirection(i);

    //swap sprite directions
    i = getDirection();
    setDirection(other.getDirection());
    other.setDirection(i);

    //swap statuses
    setActivityStatus(other.getActivityStatus());
    other.setActivityStatus(STATUS_WAITING_TO_ROLL);

    //swap roll num
    setRollNum(other.getRollNum());

    //swap stationary ticks
    setStationaryTicks(other.getStationaryTicks());
    other.setStationaryTicks(2);
}

void PlayerAvatar::swapContents(PlayerAvatar& other, int action)
{
    if (action == COINS)
    {
        int c = getCoins();
        setCoins(other.getCoins());
        other.setCoins(c);
    } else if (action == STARS)
    {
        int s = getStars();
        setStars(other.getStars());
        other.setStars(s);
    }
}

int PlayerAvatar::getCoins() const
{
    return m_coins;
}

void PlayerAvatar::setCoins(int newAmt)
{
    m_coins = newAmt;
}

int PlayerAvatar::getStars() const
{
    return m_stars;
}

void PlayerAvatar::setStars(int newAmt)
{
    m_stars = newAmt;
}

int PlayerAvatar::getRollNum() const
{
    return m_die_roll;
}

void PlayerAvatar::setRollNum(int newAmt)
{
    m_die_roll = newAmt;
}

int PlayerAvatar::getTicksLeft() const
{
    return m_ticks_to_move;
}

int PlayerAvatar::getStationaryTicks() const
{
    return m_stationary_ticks;
}

void PlayerAvatar::setStationaryTicks(int newAmt)
{
    m_stationary_ticks = newAmt;
}

void PlayerAvatar::setTicksLeft(int newAmt)
{
    m_ticks_to_move = newAmt;
}

bool PlayerAvatar::getVortex() const
{
    return m_vortex;
}

void PlayerAvatar::setVortex(bool status)
{
    m_vortex = status;
}

void PlayerAvatar::affectCoins(int amount)
{
    m_coins += amount;
}

//BADDIES
Baddies::Baddies(StudentWorld* world, int imageID, int startX, int startY)
    :MovingAvatar(world, imageID, startX, startY, 0)
{
    m_pause_counter = 180;
    m_squares_to_move = 0;
    m_ticks_to_move = 0;
    setActivityStatus(STATUS_ALIVE);
    m_isWalking = PAUSED;
}

void Baddies::doSomething()
{
    if (getActivityStatus() == STATUS_IMPACTED)
    {
        beingTeleported();
        setWalkDirection(right);
        setDirection(0);
        m_isWalking = PAUSED;
        setActivityStatus(STATUS_ALIVE);
        m_ticks_to_move = 180;        
    }
    
    if (m_isWalking == PAUSED)
    {
        checkPlayerOverlap(getWorld()->getPeach());
        checkPlayerOverlap(getWorld()->getYoshi());
        
        m_pause_counter--;
        
        //is ready to move now
        if (m_pause_counter <= 0)
        {
            m_squares_to_move = baddieRollAmount();
            m_ticks_to_move = m_squares_to_move * 8;

            setWalkDirection(findNewDirection());
            m_isWalking = WALKING;
        }
    }
    
    if (m_isWalking == WALKING)
    {
        whenWalking();
    }
}

void Baddies::checkPlayerOverlap(PlayerAvatar* player)
{
    if (landOn_moveOver(player, &landOnHelper) && player->getActivityStatus() == STATUS_WAITING_TO_ROLL && player->getStationaryTicks() == 1)
    {
        doSpecialPausedActivity(player); //do the baddies action
    }
}

void Baddies::whenWalking()
{
    if (checkAtFork())
    {
        int possibleDir;

        do
        {
            possibleDir = randInt(0, 3);
            possibleDir *= 90;
        } while(!verifyDir(possibleDir));

        newForkDir(possibleDir); //update the direction of the Baddie
    } else
    {
        if (!canMove(getWalkDirection())) //if you can't move forward in the right direction
        {
            setWalkDirection(findNewDirection());
        }
    }

    //THE USER MOVES FORWARD
    moveAtAngle(getWalkDirection(), PIXELS_TO_MOVE);

    if (onSquare())
    {
        m_squares_to_move--;
    }

    m_ticks_to_move--;

    if (m_ticks_to_move == 0 && m_squares_to_move == 0)
    {
        m_isWalking = PAUSED; 
        m_pause_counter = 180;

        //special activity only for Bowser
        doSpecialWalkingActivity();
    }

}

//BOWSERS
Bowser::Bowser(StudentWorld* world, int imageID, int startX, int startY)
    :Baddies(world, imageID, startX, startY)
{    
}

void Bowser::doSpecialPausedActivity(PlayerAvatar* player)
{
    if (randInt(0, 1) == 1)
    {
        player->setCoins(0);
        getWorld()->playSound(SOUND_BOWSER_ACTIVATE);
    }
}

int Bowser::baddieRollAmount()
{
    return randInt(1, 10);
}

void Bowser::doSpecialWalkingActivity()
{
    //25% chance of activating
    if (randInt(1, 4) == 1)
    {
        getWorld()->createDroppingSquare(getX(), getY());
        getWorld()->playSound(SOUND_DROPPING_SQUARE_CREATED);
    }
}

//BOOS
Boo::Boo(StudentWorld* world, int imageID, int startX, int startY)
    :Baddies(world, imageID, startX, startY)
{
}

void Boo::doSpecialPausedActivity(PlayerAvatar* player)
{
    if(randInt(0,1) == 0)
    {
        getWorld()->swapPlayerContents(COINS); //swap both players's coins
    } else
    {
        getWorld()->swapPlayerContents(STARS); //swap both players's stars
    }
    getWorld()->playSound(SOUND_BOO_ACTIVATE);
}

int Boo::baddieRollAmount()
{
    return randInt(1, 3);
}

void Boo::doSpecialWalkingActivity()
{
    //boo doesn't do anything
}

//VORTEX
Vortex::Vortex(int firingDir, StudentWorld* world, int imageID, int startX, int startY)
    :MovingAvatar(world, imageID, startX, startY, 0)
{
    setWalkDirection(firingDir);
    setActivityStatus(STATUS_ALIVE);
}

void Vortex::doSomething()
{
    if(getActivityStatus() == STATUS_ALIVE)
    {
        //move forward
        moveAtAngle(getWalkDirection(), PIXELS_TO_MOVE);

        //check if it is off the screen
        if (offScreen())
        {
            setActivityStatus(STATUS_DEAD);
            return;
        } else if (getWorld()->checkOverlap(getX(), getY()))
        {
            setActivityStatus(STATUS_DEAD);
            getWorld()->playSound(SOUND_HIT_BY_VORTEX);
        }
    }
}

bool Vortex::offScreen()
{
    return (getX() < 0 || getX() >= VIEW_WIDTH || getY() < 0 || getY() >= VIEW_HEIGHT);
}

//SQUARES
Square::Square(StudentWorld* world, int imageID, int startX, int startY, int dir)
    :Actor(world,imageID, startX, startY, dir, 1, 1.0)
{
    setActivityStatus(STATUS_ALIVE);
}

void Square::doSomething()
{
    if (getActivityStatus() == STATUS_ALIVE)
    {
        doSpecialActivity(getWorld()->getPeach());
        doSpecialActivity(getWorld()->getYoshi());
    }
}

//COIN SQUARE
CoinSquare::CoinSquare(int grantAmount, StudentWorld* world, int imageID, int startX, int startY)
    :Square(world,imageID, startX, startY, right)
{
    m_grant_amount = grantAmount;
}

void CoinSquare::doSpecialActivity(PlayerAvatar* player)
{
    if (landOn_moveOver(player, &landOnHelper) && player->getStationaryTicks() == 1) //if they player has 0 rolls left
    {
        if (m_grant_amount > 0)
        {
            player->affectCoins(m_grant_amount);
            getWorld()->playSound(SOUND_GIVE_COIN);
        } else if (player->getCoins() >= 3){
            player->affectCoins(m_grant_amount);
            getWorld()->playSound(SOUND_TAKE_COIN);
        }
    }
}

//DIRECTIONAL SQUARE
DirectionalSquare::DirectionalSquare(StudentWorld* world, int imageID, int startX, int startY, int dir)
    :Square(world,imageID, startX, startY, dir)
{
    m_forcingDir = dir;
}

void DirectionalSquare::doSpecialActivity(PlayerAvatar* player)
{
    if (landOn_moveOver(player, &landOnHelper) || landOn_moveOver(player, &moveOverHelper))
    {
        player->setDirectionalStatus(true);
        player->setWalkDirection(m_forcingDir);
        
        if (m_forcingDir == left)
        {
            player->setDirection(left);
        } else
        {
            player->setDirection(right);
        }
    }
}

//STAR SQUARE
StarSquare::StarSquare(StudentWorld* world, int imageID, int startX, int startY)
    :Square(world,imageID, startX, startY, right)
{}

void StarSquare::doSpecialActivity(PlayerAvatar* player)
{
    if (((landOn_moveOver(player, &landOnHelper) && player->getStationaryTicks() == 1) || landOn_moveOver(player, &moveOverHelper)) && player->getCoins() >= STAR_AMOUNT) //if the player moves over/on Square with 20+ coins
    {
        int currCoins = player->getCoins();
        int currStars = player->getStars();

        player->setCoins(currCoins - STAR_AMOUNT);
        player->setStars(currStars + 1);
        getWorld()->playSound(SOUND_GIVE_STAR);
    }
}

//BANK SQUARE
BankSquare::BankSquare(StudentWorld* world, int imageID, int startX, int startY)
    :Square(world,imageID, startX, startY, right)
{}

void BankSquare::doSpecialActivity(PlayerAvatar* player)
{
    if (landOn_moveOver(player, &landOnHelper) && player->getStationaryTicks() == 1)
    {
        giveAllCoins(player);
    } else if (landOn_moveOver(player, &moveOverHelper))
    {
        takeSomeCoins(player);
    }
}

void BankSquare::giveAllCoins(PlayerAvatar* player)
{
    int newCoinAmt = player->getCoins() + getWorld()->getBankBalance();
    player->setCoins(newCoinAmt);
    getWorld()->setBankBalance(0);
    getWorld()->playSound(SOUND_WITHDRAW_BANK);
}

void BankSquare::takeSomeCoins(PlayerAvatar* player)
{
    int depositAmt = player->getCoins();
    int balanceAmt = getWorld()->getBankBalance();
    if (depositAmt < BANK_DEDUCTION_AMOUNT)
    {
        player->setCoins(0);
        getWorld()->setBankBalance(balanceAmt + depositAmt);
    } else
    {
        player->setCoins(depositAmt - BANK_DEDUCTION_AMOUNT);
        getWorld()->setBankBalance(balanceAmt + BANK_DEDUCTION_AMOUNT);
    }
    getWorld()->playSound(SOUND_DEPOSIT_BANK);
}

//EVENT SQUARE
EventSquare::EventSquare(StudentWorld* world, int imageID, int startX, int startY)
    :Square(world,imageID, startX, startY, right)
{}

void EventSquare::doSpecialActivity(PlayerAvatar* player)
{
    if(landOn_moveOver(player, &landOnHelper) && player->getStationaryTicks() == 1)
    {
        int action = randInt(1, 3);
        switch(action)
        {
            case TELEPORT:
                player->beingTeleported();
                break;
            case SWAP:
                getWorld()->swapPlayers(player);
                break;
            case GIVE_VORTEX:
                if (!player->getVortex())
                {
                    givePlayerVortex(player);
                }
                break;
        }
    }
}

void EventSquare::givePlayerVortex(PlayerAvatar* player)
{
    if(!player->getVortex()) //if they don't have a vortex
    {
        player->setVortex(true); //give them a vortex;
        getWorld()->playSound(SOUND_GIVE_VORTEX);
    }
}

//DROPPING SQUARE
DroppingSquare::DroppingSquare(StudentWorld* world, int imageID, int startX, int startY)
    :Square(world,imageID, startX, startY, right)
{}

void DroppingSquare::doSpecialActivity(PlayerAvatar* player)
{
    if(landOn_moveOver(player, &landOnHelper) && player->getStationaryTicks() == 1)
    {
        int action = randInt(1, 2);
        switch(action)
        {
            case 1: //deduct 10 coins
                deductCoins(player);
                break;
            case 2: //deduct 1 star
                deductStar(player);
                break;
        }
    }
}

void DroppingSquare::deductCoins(PlayerAvatar* player)
{
    player->setCoins(player->getCoins() - BANK_DEDUCTION_AMOUNT);

    if (player->getCoins() < 0)
    {
        player->setCoins(0);
    }
}

void DroppingSquare::deductStar(PlayerAvatar* player)
{
    player->setStars(player->getStars() - 1);

    if (player->getStars() < 0)
    {
        player->setStars(0);
    }
}