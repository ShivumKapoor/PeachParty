#include "StudentWorld.h"
#include "GameConstants.h"
#include "Actor.h"
#include <string>

GameWorld* createStudentWorld(string assetPath)
{
	return new StudentWorld(assetPath);
}

StudentWorld::StudentWorld(string assetPath)
: GameWorld(assetPath)
{
    //initialize variables
    m_bank_balance = 0;
}

StudentWorld::~StudentWorld()
{
    delete m_peach;
    delete m_yoshi;
    delete m_curr_board;

    cleanUp();
}

void StudentWorld::populate()
{
    for (int height = 0; height < BOARD_HEIGHT; height++)
    {
        for (int width = 0; width < BOARD_WIDTH; width++)
        {
            Board::GridEntry ge = m_curr_board->getContentsOf(width, height);

            switch (ge)
            {
                case Board::empty:
                    break;
                case Board::player:
                    m_actor_list.push_back(new CoinSquare(GIVE_COINS, this, IID_BLUE_COIN_SQUARE, width, height));
                    m_yoshi = new PlayerAvatar (2, this, IID_YOSHI, width, height);
                    m_peach = new PlayerAvatar (1, this, IID_PEACH, width, height);
                    break;
                case Board::blue_coin_square:
                    m_actor_list.push_back(new CoinSquare(GIVE_COINS, this, IID_BLUE_COIN_SQUARE, width, height));
                    break;
                case Board::red_coin_square:
                    m_actor_list.push_back(new CoinSquare(TAKE_COINS, this, IID_RED_COIN_SQUARE, width, height));
                    break;
                case Board::left_dir_square:
                    m_actor_list.push_back(new DirectionalSquare(this, IID_DIR_SQUARE, width, height, 180));
                    break;
                case Board::right_dir_square:
                    m_actor_list.push_back(new DirectionalSquare(this, IID_DIR_SQUARE, width, height, 0));
                    break;
                case Board::up_dir_square:
                    m_actor_list.push_back(new DirectionalSquare(this, IID_DIR_SQUARE, width, height, 90));
                    break;
                case Board::down_dir_square:
                    m_actor_list.push_back(new DirectionalSquare(this, IID_DIR_SQUARE, width, height, 270));
                    break;
                case Board::event_square:
                    m_actor_list.push_back(new EventSquare(this, IID_EVENT_SQUARE, width, height));
                    break;
                case Board::bank_square:
                    m_actor_list.push_back(new BankSquare(this, IID_BANK_SQUARE, width, height));
                    break;
                case Board::star_square:
                    m_actor_list.push_back(new StarSquare(this, IID_STAR_SQUARE, width, height));
                    break;
                case Board::bowser:
                    m_actor_list.push_back(new CoinSquare(GIVE_COINS, this, IID_BLUE_COIN_SQUARE, width, height));
                    m_actor_list.push_back(new Bowser(this, IID_BOWSER, width, height));
                    break;
                case Board::boo:
                    m_actor_list.push_back(new CoinSquare(GIVE_COINS, this, IID_BLUE_COIN_SQUARE, width, height));
                    m_actor_list.push_back(new Boo(this, IID_BOO, width, height));
                    break;
            }
        }
    }
}

int StudentWorld::init()
{
    //loading chosen board
    m_curr_board = new Board();
    string fileName = assetPath() + "board0";
    m_boardNum = getBoardNumber();
    fileName += to_string(m_boardNum);
    fileName += ".txt";
    Board::LoadResult checkIfLoaded = m_curr_board->loadBoard(fileName);

    if (checkIfLoaded == Board::load_fail_file_not_found || checkIfLoaded == Board::load_fail_file_not_found)
    {
        return GWSTATUS_BOARD_ERROR;
    }

    populate();

	startCountdownTimer(99);

    return GWSTATUS_CONTINUE_GAME;
}

int StudentWorld::move()
{   
    //PEACH AND YOSHI doSOMETHING()
    m_peach->doSomething();
    m_yoshi->doSomething();

    //ALL ACTORS doSOMETHING
    vector<Actor*>::iterator it = m_actor_list.begin();
    while (it != m_actor_list.end())
    {
        if ((*it)->getActivityStatus() == STATUS_ALIVE || (*it)->getActivityStatus() == STATUS_IMPACTED)
        {
            (*it)->doSomething();
            it++;
        } else if ((*it)->getActivityStatus() == STATUS_DEAD)
        {
            delete *it;
            it = m_actor_list.erase(it);
        }
    }

    //UPDATE GAME TEXT
    setGameStatText(generateGameStat());

    if (timeRemaining() <= 0)
    {
        playSound(SOUND_GAME_FINISHED);

        //determining who won
        PlayerAvatar* winner = determineWinner();
        if (winner == m_peach)
        {
            setFinalScore(m_peach->getStars(), m_peach->getCoins());
            return GWSTATUS_PEACH_WON;
        } else
        {
            setFinalScore(m_yoshi->getStars(), m_yoshi->getCoins());
            return GWSTATUS_YOSHI_WON;
        }

    } else
    {
        return GWSTATUS_CONTINUE_GAME;
    }
}   

void StudentWorld::cleanUp()
{

    vector<Actor*>::iterator it = m_actor_list.begin();

    while (it != m_actor_list.end())
    {
        delete *it;
        it = m_actor_list.erase(it);
    }
    
}

Board* StudentWorld::getBoard() const
{
    return m_curr_board;
}

PlayerAvatar* StudentWorld::getPeach() const
{
    return m_peach;
}

PlayerAvatar* StudentWorld::getYoshi() const
{
    return m_yoshi;
}

bool StudentWorld::continueMoving(int x, int y)
{
    if (x % SPRITE_WIDTH == 0 && y % SPRITE_HEIGHT == 0)
    {
        vector<Actor*>::iterator it = m_actor_list.begin();
        while(it != m_actor_list.end())
        {
            if ((*it)->getX() == x && (*it)->getY() == y) //checking if an object exists at this location
            {
                return true;
            }
            it++;
        }

        return false;
    }

    return true;
}

PlayerAvatar* StudentWorld::determineWinner()
{
    PlayerAvatar* winner = m_yoshi;
    if (m_peach->getStars() > m_yoshi->getStars()) //peach has more stars
    {
        winner = m_peach;
    } else if (m_yoshi->getStars() > m_peach->getStars()) //yoshi has more stars
    {
        winner = m_yoshi;
    } else if (m_peach->getCoins() > m_yoshi->getCoins()) //peach has more coins
    {
        winner = m_peach;
    } else if (m_yoshi->getCoins() > m_peach->getCoins()) //yoshi has more coins
    {
        winner = m_yoshi;
    } else
    {
        int random = randInt(1, 2);
        if (random == 1)
        {
            winner = m_peach;
        }
    }

    return winner;
}

string StudentWorld::statForPlayer(const PlayerAvatar& player)
{
    ostringstream stats;
    stats << player.getRollNum(); //roll
    stats << " Stars: " << player.getStars(); //stars
    stats << " $$: " << player.getCoins(); //coins

    if (player.getVortex()) //check if player has a vortex
    {
        stats << " VOR";
    }
    return stats.str();
}

string StudentWorld::generateGameStat()
{
    ostringstream stats;
    stats << "P1 Roll: ";
    stats << statForPlayer(*m_peach); //get Peach's stats

    stats << " | Time: " << timeRemaining(); //time remaining
    stats << " | Bank: " << m_bank_balance << " | "; //bank total

    stats << "P2 Roll: ";
    stats << statForPlayer(*m_yoshi); //get Yoshi's stats

    return stats.str();
}

int StudentWorld::getBankBalance() const
{
    return m_bank_balance;
}

void StudentWorld::setBankBalance(int amt)
{
    m_bank_balance = amt;
}

void StudentWorld::findRandomSquareCoords(int& x, int& y)
{
    int randIndex;
    for(;;) //keep looping until you find a random square to move to
    {
        randIndex = randInt(0, m_actor_list.size() - 1);

        if (m_actor_list[randIndex]->isSquare())
        {
            x = m_actor_list[randIndex]->getX();
            y = m_actor_list[randIndex]->getY();
            return;
        }
    }
}

void StudentWorld::swapPlayers(PlayerAvatar* player)
{
    //swap peach with yoshi
    if (player == m_peach)
    {
        m_peach->swap(*m_yoshi);
    } else
    {
        m_yoshi->swap(*m_peach);
    }
}

void StudentWorld::swapPlayerContents(int action)
{
    m_peach->swapContents(*m_yoshi, action);
}

void StudentWorld::createDroppingSquare(int x, int y)
{
    //find and delete the square at bowser's coordinates
    vector<Actor*>::iterator it = m_actor_list.begin();
    while(it != m_actor_list.end())
    {
        if ((*it)->isSquare() && (*it)->getX() == x && (*it)->getY() == y)
        {
            (*it)->setActivityStatus(STATUS_DEAD);
            
            //create and add the new dropping square to the actor list
            m_actor_list.push_back(new DroppingSquare(this, IID_DROPPING_SQUARE, x/SPRITE_WIDTH, y/SPRITE_HEIGHT));
            return;
        }
        it++;
    }
}

void StudentWorld::addVortex(int x, int y, int firingDir)
{
    m_actor_list.push_back(new Vortex(firingDir, this, IID_VORTEX, x/ SPRITE_WIDTH, y/ SPRITE_HEIGHT));
}

bool StudentWorld::checkOverlap(int x, int y)
{
    //find first impactable object that overlaps
    vector<Actor*>::iterator it = m_actor_list.begin();
    while(it != m_actor_list.end())
    {
        if((*it)->isImpactable())
        {
            int objX = (*it)->getX();
            int objY = (*it)->getY();

            if (checkOverlapHelper(x, y, objX, objY))
            {
                
                (*it)->setActivityStatus(STATUS_IMPACTED);
                return true;
            }
        }

        it++;
    }

    return false;
}

bool StudentWorld::checkOverlapHelper(int x1, int y1, int x2, int y2)
{
    if ((x2 <= x1 + SPRITE_WIDTH - 1 && x2 >= x1) || (x1 <= x2 + SPRITE_WIDTH - 1 && x1 >= x2))
    {
        if ((y2 <= y1 + SPRITE_WIDTH - 1 && y2 >= y1) || (y1 <= y2 + SPRITE_WIDTH - 1 && y1 >= y2))
        {
            return true;
        }
    }

    return false;
}
