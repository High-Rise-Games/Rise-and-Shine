//  GameController.cpp
//
//  This is the primary class file for running the game.
//
//  Author: High Rise Games
//
#include <cugl/cugl.h>
#include <iostream>
#include <sstream>
#include <random>

#include "GameplayController.h"


using namespace cugl;
using namespace std;

#pragma mark -
#pragma mark Level Layout

// Lock the screen size to fixed height regardless of aspect ratio
#define SCENE_HEIGHT 720

#pragma mark -
#pragma mark Constructors
/**
 * Initializes the controller contents, and starts the game
 *
 * The constructor does not allocate any objects or memory.  This allows
 * us to have a non-pointer reference to this controller, reducing our
 * memory allocation.  Instead, allocation happens in this method.
 *
 * @param assets    The (loaded) assets for this game mode
 *
 * @return true if the controller is initialized properly, false otherwise.
 */
bool GameplayController::init(const std::shared_ptr<cugl::AssetManager>& assets, int fps, cugl::Rect bounds, cugl::Size size) {
    // Initialize the scene to a locked width
    
    // time of the game set to 120 seconds
    _gameTime = 120;
    _gameTimeLeft = _gameTime;
    

    _frame=0;
    

    // we set game win and game over to false
    _gameWin = false;
    _gameOver = false;
    _gameStart = false;
    _transitionToMenu = false;
    // each image lasts for 2 frames, 25 frames per number and 4 numbers in total
    _maxCountDownFrames = 2 * 4 * 25;
    _countDownFrames = 0;
    
    // set this to zero, will be updated when game is over
    _frameCountForWin=0;

    // fps as established per App
    _fps = fps;
    
    Size dimen = Application::get()->getDisplaySize();
    _rng.seed(std::time(nullptr));
    _dirtGenSpeed = 2;
    _fixedDirtUpdateThreshold = 5 * 60;
    _maxDirtAmount = 10;
    _size = size;
    _nativeSize = size;
    
    _dirtSelected = false;
    _dirtPath = Path2();
    dimen *= SCENE_HEIGHT/dimen.height;
    if (assets == nullptr) {
        return false;
    }
        
    // Start up the input handler
    _assets = assets;
    
    _input.init();
    // _dirtThrowInput.init();
    
    // Get the constant values
    _constants = _assets->get<JsonValue>("constants");

    // get the arrow texture
    _arrowTexture = _assets->get<Texture>("arrow");

    // Initialize an empty playerVec that can hold up to 4 players
    _playerVec = { nullptr, nullptr, nullptr, nullptr };
    // Initialize an empty windowVec that can hold up to 4 window grids
    _windowVec = { nullptr, nullptr, nullptr, nullptr };
    // Initialize an empty projectileVec that can hold up to 4 projectile sets
    _projectileVec = { nullptr, nullptr, nullptr, nullptr };

    return true;
}

bool GameplayController::initLevel(int selected_level) {
    if (_ishost) {
        hostReset();
    }
    else {
        reset();
    }
    // TODO: update depending on level
    _birdActive = true;

    // Initialize the window grids
//    std::shared_ptr<cugl::JsonValue> level = _constants->get("easy board"); // TODO: make field passed in from level select through App
    switch (selected_level) {
        case 1:
            _levelJson = _assets->get<JsonValue>("level1");
            _size = _nativeSize;
            _size.height *= 2;
            break;
        case 2:
            _levelJson = _assets->get<JsonValue>("level2");
            _size = _nativeSize;
            _size.height *= 3.5;
            break;
        case 3:
            _levelJson = _assets->get<JsonValue>("level3");
            _size = _nativeSize;
            _size.height *= 3.5;
            break;
        case 4:
            _levelJson = _assets->get<JsonValue>("nightlevel");
            _size = _nativeSize;
            _size.height *= 3.5;
            break;
        case 5:
            _levelJson = _assets->get<JsonValue>("dreamylevel");
            _size = _nativeSize;
            _size.height *= 3.5;
            break;
        default:
            _levelJson = _assets->get<JsonValue>("level1");
            _size = _nativeSize;
            _size.height *= 2;
            break;
    }
    
    // texture mappings for each level (update these from the python script)
    
    std::vector<string> texture_strings_level_1 = { "day1Building", "day2Building", "day3Building", "dreamyBuilding", "nightBuilding", "level1Window1", "level1Window2", "fully_blocked_1", "fully_blocked_2", "fully_blocked_3", "fully_blocked_4", "left_blocked_1", "down_blocked_1", "planter-brown1" };
    std::vector<string> texture_strings_level_2 = { "day1Building", "day2Building", "day3Building", "dreamyBuilding", "nightBuilding", "level2Window1", "level2Window2", "down_blocked_1", "planter-brown1", "fully_blocked_1", "fully_blocked_2", "fully_blocked_3", "fully_blocked_4", "left_blocked_1" };
    std::vector<string> texture_strings_level_3 = { "level3Window1", "level3Window2", "down_blocked_1", "planter-brown1", "fully_blocked_1", "fully_blocked_2", "fully_blocked_3", "fully_blocked_4", "left_blocked_1", "day1Building", "day2Building", "day3Building", "dreamyBuilding", "nightBuilding" };
    std::vector<string> texture_strings_level_4 = { "nightWindow1", "nightWindow2", "nightWindow3", "nightWindow4", "nightWindow5", "down_blocked_1", "planter-brown1", "fully_blocked_1", "fully_blocked_2", "fully_blocked_3", "fully_blocked_4", "left_blocked_1", "day1Building", "day2Building", "day3Building", "dreamyBuilding", "nightBuilding" };
    std::vector<string> texture_strings_level_5 = { "dreamywin1", "dreamywin2", "dreamywin3", "dreamywin4", "dreamywin5", "down_blocked_1", "planter-brown1", "fully_blocked_1", "fully_blocked_2", "fully_blocked_3", "fully_blocked_4", "left_blocked_1", "day1Building", "day2Building", "day3Building", "dreamyBuilding", "nightBuilding" };
    std::vector<std::vector<string>> texture_strings_levels;
    std::vector<int> texture_ids_level_1 = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14 };
    std::vector<int> texture_ids_level_2 = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14 };
    std::vector<int> texture_ids_level_3 = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14 };
    std::vector<int> texture_ids_level_4 = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17 };
    std::vector<int> texture_ids_level_5 = { 1, 2, 3, 4, 5, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18 };
    std::vector<std::vector<int>> texture_ids_levels;
    texture_strings_levels.push_back(texture_strings_level_1);
    texture_strings_levels.push_back(texture_strings_level_2);
    texture_strings_levels.push_back(texture_strings_level_3);
    texture_strings_levels.push_back(texture_strings_level_4);
    texture_strings_levels.push_back(texture_strings_level_5);
    texture_ids_levels.push_back(texture_ids_level_1);
    texture_ids_levels.push_back(texture_ids_level_2);
    texture_ids_levels.push_back(texture_ids_level_3);
    texture_ids_levels.push_back(texture_ids_level_4);
    texture_ids_levels.push_back(texture_ids_level_5);

    std::vector<int>     dirt_counts = { 22, 50, 50, 60, 60 };
    std::vector<string>  dirt_texture_strings = { "level1dirt", "dirt", "dirt", "dirt2", "dirt"};
    _dirtTextureString = dirt_texture_strings.at(selected_level - 1);
    // select the correct mapping for this level
    _texture_strings_selected = texture_strings_levels.at(selected_level - 1);
    _texture_ids_selected = texture_ids_levels.at(selected_level - 1);
    
    _initDirtCount = dirt_counts.at(selected_level - 1);
    
    // get the win background when game is win
    _winBackground =  _assets->get<Texture>("win-background" );
    
    // get the lose background when game is lose
    _loseBackground = _assets->get<Texture>("lose-background");
    
    // get the asseets for countdown
    setCountdown1Texture(_assets->get<Texture>("C1"));
    setCountdown2Texture(_assets->get<Texture>("C2"));
    setCountdown3Texture(_assets->get<Texture>("C3"));
    setCountdownGoTexture(_assets->get<Texture>("Go"));
    setCountdownSparkleTexture(_assets->get<Texture>("Sparkle"));
    
    // Initialize random dirt generation
    // TODO: decide if we still need?
    updateDirtGenTime();

    _collisions.init(_size);

    // Get the bang sound
    _bang = _assets->get<Sound>("bang");
    _clean = _assets->get<Sound>("clean");
    
    return true;
}

/** 
 * Initializes the player, window grid, and projectile set models for all clients
 */
bool GameplayController::initClient(const std::shared_ptr<cugl::AssetManager>& assets) {
    if (assets == nullptr) {
        return false;
    }

    // Initialize window grid for self
    _windowVec[_id - 1] = make_shared<WindowGrid>();
    for (string thisWindow : _texture_strings_selected) {
        _windowVec[_id - 1]->addTexture(_assets->get<Texture>(thisWindow));
    }
    _windowVec[_id - 1]->setTextureIds(_texture_ids_selected);
    _windowVec[_id - 1]->init(_levelJson, _size); // init depends on texture
    _windowVec[_id - 1]->setInitDirtNum(_initDirtCount);
    _windowVec[_id - 1]->setDirtTexture(assets->get<Texture>(_dirtTextureString));
    _windowVec[_id - 1]->setFadedDirtTexture(assets->get<Texture>("faded-dirt"));

    // Initialize player character for self
    Vec2 startingPos = Vec2(_windowVec[_id - 1]->sideGap + (_windowVec[_id - 1]->getPaneWidth() / 2), _windowVec[_id - 1]->getPaneHeight() / 2);
    _playerVec[_id - 1] = make_shared<Player>(_id, startingPos, _windowVec[_id - 1]->getPaneHeight(), _windowVec[_id - 1]->getPaneWidth());
    _playerVec[_id - 1]->setPosition(startingPos);
    _playerVec[_id - 1]->setVelocity(Vec2::ZERO);
    // set temporary character until host sends message on character
    changeCharTexture(_playerVec[_id - 1], ""); // empty string defaults to Mushroom
    _playerVec[_id - 1]->setChar("");


    // Initialize bird textures, but do not set a location yet. that is the host's job
    if (_birdActive) {
        int height = _windowVec[_id - 1]->getNVertical();
        int width = _windowVec[_id - 1]->getNHorizontal();
        cugl::Vec2 birdTopLeftPos = cugl::Vec2(0.4, height - 0.5);
        cugl::Vec2 birdTopRightPos = cugl::Vec2(width - 0.6, height - 0.5);
        cugl::Vec2 birdBotLeftPos = cugl::Vec2(0.4, height - 3.5);
        cugl::Vec2 birdBotRightPos = cugl::Vec2(width - 0.6, height - 3.5);
        std::vector<cugl::Vec2> positions = { birdTopLeftPos, birdTopRightPos, birdBotLeftPos, birdBotRightPos };
        _bird.init(positions, 0.01, 0.04, _windowVec[_id - 1]->getPaneHeight());
        _bird.setTexture(assets->get<Texture>("bird"));
    }

    // Initialize projectiles  for self
    _projectileVec[_id - 1] = make_shared<ProjectileSet>();
    _projectileVec[_id - 1]->setDirtTexture(assets->get<Texture>(_dirtTextureString));
    _projectileVec[_id - 1]->setPoopTexture(assets->get<Texture>("poop"));
    _projectileVec[_id - 1]->setTextureScales(_windowVec[_id - 1]->getPaneHeight(), _windowVec[_id - 1]->getPaneWidth());
    _projectileVec[_id - 1]->init(_constants->get("projectiles"));
    return true;
}

/**
* Initializes the extra controllers needed for the host of the game.
*
* The constructor does not allocate any objects or memory.  This allows
* us to have a non-pointer reference to this controller, reducing our
* memory allocation.  Instead, allocation happens in this method.
* 
* Assigns player ids clockwise with host at top
* 
*          host: 1
* left: 4            right: 2
*         across: 3
*
* @param assets    The (loaded) assets for this game mode
* @param nPlayers  The number of players for this game
*
* @return true if the controller is initialized properly, false otherwise.
*/
bool GameplayController::initHost(const std::shared_ptr<cugl::AssetManager>& assets) {
    if (assets == nullptr) {
        return false;
    }

    if (_ishost) {
        _numPlayers = _network.getNumPlayers();

        for (int i = 1; i <= _numPlayers; i++) {
            // Initialize window grids
            _windowVec[i - 1] = make_shared<WindowGrid>();
            for (string thisWindow : _texture_strings_selected) {
                _windowVec[i - 1]->addTexture(_assets->get<Texture>(thisWindow));
            }
            _windowVec[i - 1]->setTextureIds(_texture_ids_selected);
            _windowVec[i - 1]->init(_levelJson, _size); // init depends on texture
            _windowVec[i - 1]->setInitDirtNum(_initDirtCount);
            _windowVec[i - 1]->setDirtTexture(assets->get<Texture>(_dirtTextureString));
            _windowVec[i - 1]->setFadedDirtTexture(assets->get<Texture>("faded-dirt"));
            _windowVec[i-1]->generateInitialBoard(_windowVec[i - 1]->getInitDirtNum());

            // Initialize player characters
            Vec2 startingPos = Vec2(_windowVec[i - 1]->sideGap + (_windowVec[i - 1]->getPaneWidth() / 2), _windowVec[i - 1]->getPaneHeight() / 2);
            _playerVec[i - 1] = make_shared<Player>(i, startingPos, _windowVec[i - 1]->getPaneHeight(), _windowVec[i - 1]->getPaneWidth());
            _playerVec[i - 1]->setPosition(startingPos);
            _playerVec[i - 1]->setVelocity(Vec2::ZERO);
            _playerVec[i - 1]->setAnimationState(Player::AnimStatus::IDLE);

            // Initialize projectiles
            _projectileVec[i - 1] = make_shared<ProjectileSet>();
            _projectileVec[i-1]->setDirtTexture(assets->get<Texture>(_dirtTextureString));
            _projectileVec[i - 1]->setPoopTexture(assets->get<Texture>("poop"));
            _projectileVec[i - 1]->setTextureScales(_windowVec[i - 1]->getPaneHeight(), _windowVec[i - 1]->getPaneWidth());
            
        }
    }

    // Initialize bird textures, but do not set a location yet. that is the host's job
    if (_birdActive) {
        int height = _windowVec[_id - 1]->getNVertical();
        int width = _windowVec[_id - 1]->getNHorizontal();
        cugl::Vec2 birdTopLeftPos = cugl::Vec2(0.4, height - 0.5);
        cugl::Vec2 birdTopRightPos = cugl::Vec2(width - 0.6, height - 0.5);
        cugl::Vec2 birdBotLeftPos = cugl::Vec2(0.4, height - 3.5);
        cugl::Vec2 birdBotRightPos = cugl::Vec2(width - 0.6, height - 3.5);
        std::vector<cugl::Vec2> positions = { birdTopLeftPos, birdTopRightPos, birdBotLeftPos, birdBotRightPos };
        _bird.init(positions, 0.01, 0.04, _windowVec[_id - 1]->getPaneHeight());
        _bird.setTexture(_assets->get<Texture>("bird"));
        // randomly place bird on a player's board
        _curBirdBoard = rand() % _numPlayers + 1;
    }

    return true;
}


#pragma mark Graphics

/**
 * Sets the texture for countdown 3.
 */
void GameplayController::setCountdown3Texture(const std::shared_ptr<cugl::Texture>& texture) {
    _countdown3Sprite = SpriteSheet::alloc(texture, 5, 5, 25);
    _countdown3Sprite->setFrame(0);
}
/**
 * Sets the texture for countdown 2.
 */
void GameplayController::setCountdown2Texture(const std::shared_ptr<cugl::Texture>& texture) {
    _countdown2Sprite = SpriteSheet::alloc(texture, 5, 5, 25);
    _countdown2Sprite->setFrame(0);
}
/**
 * Sets the texture for countdown 1.
 */
void GameplayController::setCountdown1Texture(const std::shared_ptr<cugl::Texture>& texture) {
    _countdown1Sprite = SpriteSheet::alloc(texture, 5, 5, 25);
    _countdown1Sprite->setFrame(0);
}
/**
 * Sets the texture for countdown Go.
 */
void GameplayController::setCountdownGoTexture(const std::shared_ptr<cugl::Texture>& texture) {
    _countdownGoSprite = SpriteSheet::alloc(texture, 5, 5, 25);
    _countdownGoSprite->setFrame(0);
}
/**
 * Sets the texture for countdown sparkles.
 */
void GameplayController::setCountdownSparkleTexture(const std::shared_ptr<cugl::Texture>& texture) {
    _countdownSparkleSprite = SpriteSheet::alloc(texture, 5, 5, 25);
    _countdownSparkleSprite->setFrame(0);
}

#pragma mark Gameplay Handling
void GameplayController::reset() {
    _playerVec = { nullptr, nullptr, nullptr, nullptr };
    _windowVec = { nullptr, nullptr, nullptr, nullptr };
    _projectileVec = { nullptr, nullptr, nullptr, nullptr };

    // Reset existence of enemies
    _birdLeaving = false;

    // Reset all starting current boards
    _allCurBoards = { 0, 0, 0, 0 };
    // Reset all progress trackers
    _progressVec = { 0, 0, 0, 0 };

    _dirtThrowTimer = 0;
    _projectileGenChance = 0.1;
    _projectileGenCountDown = 120;
    _currentDirtAmount = 0;

    _gameOver = false;
    _gameStart = false;
    _gameWin = false;
}

/**
 * Resets the status of the game for all players so that we can play again.
 */
void GameplayController::hostReset() {
    reset();

    _allDirtAmounts = { 0, 0, 0, 0 };
    _hasWon = { false, false, false, false };
}

/**
* HOST ONLY. Sets the character of the player given player's id.
* Possible values: "Mushroom", "Frog", "Flower", "Chameleon"
*/
void GameplayController::setCharacters(std::vector<std::string>& chars) {
    for (int i = 0; i < _numPlayers; i++) {
        auto player = _playerVec[i];
        changeCharTexture(player, chars[i]);
        player->setChar(chars[i]);
        CULog("character: %a", chars[i].c_str());
    }
}

void GameplayController::changeCharTexture(std::shared_ptr<Player>& player, std::string charChoice) {
    if (charChoice == "Frog") {
        player->setIdleTexture(_assets->get<Texture>("idle_frog"));
        player->setWipeTexture(_assets->get<Texture>("wipe_frog"));
        player->setShooTexture(_assets->get<Texture>("shoo_frog"));
        player->setThrowTexture(_assets->get<Texture>("throw_frog"));
        player->setProfileTexture(_assets->get<Texture>("profile_frog"));
    }
    else if (charChoice == "Flower") {
        player->setIdleTexture(_assets->get<Texture>("idle_flower"));
        player->setWipeTexture(_assets->get<Texture>("wipe_flower"));
        player->setShooTexture(_assets->get<Texture>("shoo_flower"));
        player->setThrowTexture(_assets->get<Texture>("throw_flower"));
        player->setProfileTexture(_assets->get<Texture>("profile_flower"));
    }
    else if (charChoice == "Chameleon") {
        player->setIdleTexture(_assets->get<Texture>("idle_chameleon"));
        player->setWipeTexture(_assets->get<Texture>("wipe_chameleon"));
        player->setShooTexture(_assets->get<Texture>("shoo_chameleon"));
        player->setThrowTexture(_assets->get<Texture>("throw_chameleon"));
        player->setProfileTexture(_assets->get<Texture>("profile_chameleon"));
    }
    else {
        player->setIdleTexture(_assets->get<Texture>("idle_mushroom"));
        player->setWipeTexture(_assets->get<Texture>("wipe_mushroom"));
        player->setShooTexture(_assets->get<Texture>("shoo_mushroom"));
        player->setThrowTexture(_assets->get<Texture>("throw_mushroom"));
        player->setProfileTexture(_assets->get<Texture>("profile_mushroom"));
    }
}

int calculateNeighborId(int myId, int dir, std::vector<std::shared_ptr<Player>> playerVec) {
    int nbrId = myId + dir;
    if (nbrId <= 0) {
        nbrId = 4;
    }
    if (nbrId > 4) {
        nbrId = 1;
    }
    while (playerVec[nbrId - 1] == nullptr) {
        nbrId += dir;
        if (nbrId <= 0) {
            nbrId = 4;
        }
        else if (nbrId > 4) {
            nbrId = 1;
        }
    }
    return nbrId;
}

/**
* Given the world positions, convert it to the board position
* based off of grid coordinates. Ex. [2, 3] or [2.3, 3] if the
* player is in the process of moving in between x = 2 and x = 3.
*/
cugl::Vec2 GameplayController::getBoardPosition(cugl::Vec2 worldPos) {
    float x_coor = (worldPos.x - _windowVec[_id-1]->sideGap) / _windowVec[_id-1]->getPaneWidth();
    float y_coor = worldPos.y / _windowVec[_id-1]->getPaneHeight();
    
    return Vec2(x_coor, y_coor);
}

/**
* Given the board positions, convert it to the world position.
*/
cugl::Vec2 GameplayController::getWorldPosition(cugl::Vec2 boardPos) {
    float x_coor = boardPos.x * _windowVec[_id - 1]->getPaneWidth() + _windowVec[_id - 1]->sideGap;
    float y_coor = boardPos.y * _windowVec[_id - 1]->getPaneHeight();
    return Vec2(x_coor, y_coor);
}

void GameplayController::advanceCountDownAnim(bool ishost) {
    if (_countDownFrames < _maxCountDownFrames) {
        // frame timer for next frame
        if (_countDownFrames % 2 == 0) {
            getCurrentCountdownSprite()->setFrame((int) _countDownFrames / 2 % 25);
            _countdownSparkleSprite->setFrame((int) _countDownFrames / 2 % 25);
        }
        //only host steps forward animation and it sends frame number to clients
        if (ishost) {
            _countDownFrames += 1;
        }
    } else {
        _gameStart = true;
    }
}

std::shared_ptr<cugl::SpriteSheet> GameplayController::getCurrentCountdownSprite() {
    int curFrame = _countDownFrames / (25 * 2);
    if (curFrame == 0) {
        return _countdown3Sprite;
    } else if (curFrame == 1) {
        return _countdown2Sprite;
    } else if (curFrame == 2) {
        return _countdown1Sprite;
    } else {
        return _countdownGoSprite;
    }
}

/**
 * Method for the return to board button listener used in GameScene
 */
void GameplayController::switchScene() {
    if (_allCurBoards[_id-1] != 0) {
        if (_ishost) {
            _allCurBoards[0] = 0;
            _allCurBoards[_id - 1] = 0;
        }
        else {
            _network.sendToHost(getJsonSceneSwitch(true));
        }
    }
}

/** 
 * Host Only. Converts game state into a JSON value for sending over the network.
 * Only called by the host, as only the host transmits board states over the network.
 * 
 * @param id    the id of the player of the board state to get
 * @returns JSON value representing game board state
 */
std::shared_ptr<cugl::JsonValue> GameplayController::getJsonBoard(int id, bool isPartial) {
    std::shared_ptr<Player> player = _playerVec[id-1];
    std::shared_ptr<WindowGrid> windows = _windowVec[id - 1];
    std::shared_ptr<ProjectileSet> projectiles = _projectileVec[id-1];


    const std::shared_ptr<JsonValue> json = std::make_shared<JsonValue>();
    json->init(JsonValue::Type::ObjectType);
    json->appendValue("player_id", std::to_string(id));
    json->appendValue("player_char", player->getChar());
    if (!_gameStart) {
        json->appendValue("countdown_frame", std::to_string(_countDownFrames));
    }
    std::string win_str = _hasWon[id-1] ? "true" : "false";
    json->appendValue("has_won", win_str);
    json->appendValue("num_dirt", std::to_string(_allDirtAmounts[id - 1]));
    json->appendValue("curr_board", std::to_string(_allCurBoards[id - 1]));
    json->appendValue("progress", std::to_string(_progressVec[id-1]));

    cugl::Vec2 playerBoardPos = getBoardPosition(player->getPosition());
    if(!isPartial) json->appendValue("player_x", std::to_string(playerBoardPos.x));
    json->appendValue("player_y", std::to_string(playerBoardPos.y));

    json->appendValue("anim_state", std::to_string(player->statusToInt[player->getAnimationState()]));

    json->appendValue("timer", std::to_string(_gameTimeLeft));
    
    if (!isPartial) {
        
        if (_curBirdBoard == id) {
            const std::shared_ptr<JsonValue> birdPos = std::make_shared<JsonValue>();
            birdPos->init(JsonValue::Type::ArrayType);
            birdPos->appendValue(std::to_string(_bird.birdPosition.x));
            birdPos->appendValue(std::to_string(_bird.birdPosition.y));
            json->appendChild("bird_pos", birdPos);
            json->appendValue("bird_facing_right", (_bird.isFacingRight()));
        }

        const std::shared_ptr<JsonValue> dirtArray = std::make_shared<JsonValue>();
        dirtArray->init(JsonValue::Type::ArrayType);
        for (int col = 0; col < windows->getNHorizontal(); ++col) {
            for (int row = 0; row < windows->getNVertical(); ++row) {
                bool hasDirt = windows->getWindowState(row, col);
                if (hasDirt) {
                    const std::shared_ptr<JsonValue> dirtPos = std::make_shared<JsonValue>();
                    dirtPos->init(JsonValue::Type::ArrayType);
                    dirtPos->appendValue(std::to_string(row));
                    dirtPos->appendValue(std::to_string(col));
                    dirtArray->appendChild(dirtPos);
                }
            }
        }
        json->appendChild("dirts", dirtArray);

        const std::shared_ptr<JsonValue> projArray = std::make_shared<JsonValue>();
        projArray->init(JsonValue::Type::ArrayType);

        for (shared_ptr<ProjectileSet::Projectile> proj : projectiles->current) {
            const std::shared_ptr<JsonValue> projJson = std::make_shared<JsonValue>();
            projJson->init(JsonValue::Type::ObjectType);

            cugl::Vec2 projBoardPos = getBoardPosition(proj->position);
            const std::shared_ptr<JsonValue> projPos = std::make_shared<JsonValue>();
            projPos->init(JsonValue::Type::ArrayType);
            projPos->appendValue(std::to_string(projBoardPos.x));
            projPos->appendValue(std::to_string(projBoardPos.y));
            projJson->appendChild("pos", projPos);

            const std::shared_ptr<JsonValue> projVel = std::make_shared<JsonValue>();
            projVel->init(JsonValue::Type::ArrayType);
            projVel->appendValue(std::to_string(proj->velocity.x));
            projVel->appendValue(std::to_string(proj->velocity.y));
            projJson->appendChild("vel", projVel);

            cugl::Vec2 projDestBoardPos = getBoardPosition(proj->destination);
            const std::shared_ptr<JsonValue> projDest = std::make_shared<JsonValue>();
            projDest->init(JsonValue::Type::ArrayType);
            projDest->appendValue(std::to_string(projDestBoardPos.x));
            projDest->appendValue(std::to_string(projDestBoardPos.y));
            projJson->appendChild("dest", projDest);

            std::string projTypeStr = "POOP";
            if (proj->type == ProjectileSet::Projectile::ProjectileType::DIRT) {
                projTypeStr = "DIRT";
            }
            projJson->appendValue("type", projTypeStr);

            projArray->appendChild(projJson);
        }
        json->appendChild("projectiles", projArray);
    }

    return json;
}

/**
* Updates a neighboring or own board given the JSON value representing its game state.
* Called by CLIENT ONLY.
* 
* * Example board state (full message):
 * {
    "player_id":  "1",
    "player_char": "Frog",
    "has_won": "false",
    "num_dirt": "1",
    "curr_board": "0",
    "player_x": "3.0",
    "player_y": "4.0",
    "anim_state": "1",
    "timer": "145",
    "bird_pos": ["2.4", "6.0"],
    "bird_facing_right": true,
    "dirts": [ ["0", "1"], ["2", "2"], ["0", "2"] ],
    "projectiles": [
            { 
                "pos": ["3.0", "1.45"],
                "vel": ["2", "3"],
                "dest": ["12.23", "23.5"],
                "type: "DIRT"
            },
            {
                "pos": ["5.0", "0.2"],
                "vel": [],
                "dest": ["12.23", "23.5"],
                "type": "POOP"
            }
        ]
 * }
 * 
 * Example board state (partial message):
 * {
    "player_id":  "1",
    "countdown_frame": "20",
    "player_char": "Frog",
    "has_won": "false",
    "num_dirt": "1",
    "curr_board": "0",
    "player_y": "4.0",
    "timer": "145",
    "progress": "0.7"
 * }
*
* @params data     The data to update
*/
void GameplayController::updateBoard(std::shared_ptr<JsonValue> data) {
    int playerId = std::stod(data->getString("player_id", "0"));
    std::string playerChar = data->getString("player_char");
    std::string playerHasWon = data->getString("has_won", "false");
    if (playerHasWon == "true" && !_gameOver) {
        _gameOver = true;
        setWin(playerId == _id);
        return;
    }
    _progressVec[playerId - 1] = std::stod(data->getString("progress", "0"));
    // set to some random default high value indicating count down is over
    if (!_gameStart) {
        _countDownFrames = std::stod(data->getString("countdown_frame", "5000"));
        advanceCountDownAnim(false);
    }
    
    // get x, y positions of player
    Vec2 playerBoardPos(std::stod(data->getString("player_x", "0")), std::stod(data->getString("player_y", "0")));

    if (playerId == _id && playerChar != _playerVec[_id - 1]->getChar()) {
        // first time this client is receiving message about their chosen character
        _playerVec[_id - 1]->setChar(playerChar);
        changeCharTexture(_playerVec[_id - 1], playerChar);
    }

    if (_playerVec[playerId - 1] == nullptr) {
        // first time this client is receiving message on another player

        // instantiate this player's window grid in this client's game instance
        _windowVec[playerId - 1] = make_shared<WindowGrid>();
        for (string thisWindow : _texture_strings_selected) {
            _windowVec[playerId - 1]->addTexture(_assets->get<Texture>(thisWindow));
        }
        _windowVec[playerId - 1]->setTextureIds(_texture_ids_selected);
        _windowVec[playerId - 1]->init(_levelJson, _size); // init depends on texture
        _windowVec[playerId - 1]->setInitDirtNum(_initDirtCount);
        _windowVec[playerId - 1]->setDirtTexture(_assets->get<Texture>(_dirtTextureString));
        _windowVec[playerId - 1]->setFadedDirtTexture(_assets->get<Texture>("faded-dirt"));

        // instantiate this player in this client's game instance
        _playerVec[playerId - 1] = std::make_shared<Player>(playerId, getWorldPosition(playerBoardPos), _windowVec[_id - 1]->getPaneWidth(), _windowVec[_id - 1]->getPaneHeight());
        // set the player's character textures
        _playerVec[playerId - 1]->setChar(playerChar);
        changeCharTexture(_playerVec[playerId - 1], playerChar);

        // instantiate this player's projectile set in this client's game instance
        _projectileVec[playerId - 1] = make_shared<ProjectileSet>();
        _projectileVec[playerId - 1]->setDirtTexture(_assets->get<Texture>(_dirtTextureString));
        _projectileVec[playerId - 1]->setPoopTexture(_assets->get<Texture>("poop"));
        _projectileVec[playerId - 1]->setTextureScales(_windowVec[_id - 1]->getPaneHeight(), _windowVec[_id - 1]->getPaneWidth());
    }

    auto player = _playerVec[playerId - 1];
    player->setPosition(getWorldPosition(playerBoardPos));

    player->setAnimationState(player->animStatusNames[std::stod(data->getString("anim_state"))]);
    
    auto windows = _windowVec[playerId - 1];
    auto projectiles = _projectileVec[playerId-1];
    // CULog("playerId: %d", playerId);

    if (playerId == _id) {
        // update own board info
        _gameTimeLeft = std::stoi(data->getString("timer"));
        _currentDirtAmount = std::stod(data->getString("num_dirt", "0"));
    }
    _allCurBoards[playerId - 1] = std::stod(data->getString("curr_board", "0"));


    if (data->has("bird_pos")) {
        _curBirdBoard = playerId;
        // update bird position, no matter which board the bird is on
        const std::vector<std::shared_ptr<JsonValue>>& birdPos = data->get("bird_pos")->children();
        Vec2 birdBoardPos(std::stod(birdPos[0]->asString()), std::stod(birdPos[1]->asString()));
        _curBirdPos = getWorldPosition(birdBoardPos);

        _bird.setFacingRight(data->getBool("bird_facing_right"));
        if (playerId == _id) _birdLeaving = false;
    }
    else if (playerId == _id && _curBirdBoard == _id) {
        _birdLeaving = true;
        _curBirdBoard = 0; // set to arbitrary value to represent bird not on board
    }


    if (data->has("dirts") && data->has("projectiles")) {
        // populate player's board with dirt
        windows->clearBoard();
        for (const std::shared_ptr< JsonValue>& jsonDirt : data->get("dirts")->children()) {
            std::vector<std::string> dirtPos = jsonDirt->asStringArray();
            windows->addDirt(std::stod(dirtPos[0]), std::stod(dirtPos[1]));
        }

        // populate player's projectile setZ
        projectiles->clearCurrentSet(); // clear current set to rewrite
        for (const std::shared_ptr<JsonValue>& projNode : data->get("projectiles")->children()) {
            // get projectile position
            const std::vector<std::shared_ptr<JsonValue>>& projPos = projNode->get("pos")->children();
            Vec2 pos(std::stod(projPos[0]->asString()), std::stod(projPos[1]->asString()));

            // get projectile velocity
            const std::vector<std::shared_ptr<JsonValue>>& projVel = projNode->get("vel")->children();
            Vec2 vel(std::stod(projVel[0]->asString()), std::stod(projVel[1]->asString()));

            // get projectile destination
            const std::vector<std::shared_ptr<JsonValue>>& projDest = projNode->get("dest")->children();
            Vec2 dest(std::stod(projDest[0]->asString()), std::stod(projDest[1]->asString()));

            // get projectile type
            string typeStr = projNode->get("type")->asString();
            auto type = ProjectileSet::Projectile::ProjectileType::POOP;
            if (typeStr == "DIRT") {
                type = ProjectileSet::Projectile::ProjectileType::DIRT;
            }

            // add the projectile to neighbor's projectile set
            projectiles->spawnProjectileClient(getWorldPosition(pos), vel, getWorldPosition(dest), type);
        }
    }
}

/**
 * Converts a movement vector into a JSON value for sending over the network.
 *
 * @param move    the movement vector
 * @returns JSON value representing a movement
 */
std::shared_ptr<cugl::JsonValue> GameplayController::getJsonMove(const cugl::Vec2 move) {
    const std::shared_ptr<JsonValue> json = std::make_shared<JsonValue>();
    json->init(JsonValue::Type::ObjectType);
    json->appendValue("player_id", std::to_string(_id));

    const std::shared_ptr<JsonValue> vel = std::make_shared<JsonValue>();
    vel->init(JsonValue::Type::ArrayType);
    vel->appendValue(std::to_string(move.x));
    vel->appendValue(std::to_string(move.y));
    json->appendChild("vel", vel);
    

    return json;
}

/**
* Called by the host only. Updates a client player's board for player at player_id
* based on the movement or other action data stored in the JSON value.
* 
* Player ids assigned clockwise with host at top
* 
*          host: 1
* left: 4            right: 2
*         across: 3
* 
 * Example movement message:
 * {
 *    "player_id":  1,
 *    "vel": [0.42, 0.66]
 * }
*
* @params data     The data to update
*/
void GameplayController::processMovementRequest(std::shared_ptr<cugl::JsonValue> data) {
//    int playerId = data->getInt("player_id", 0);
    int playerId = std::stod(data->getString("player_id", "0"));
    const std::vector<std::shared_ptr<JsonValue>>& vel = data->get("vel")->children();
    Vec2 moveVec(std::stod(vel[0]->asString()), std::stod(vel[1]->asString()));
    std::shared_ptr<Player> player = _playerVec[playerId-1];
    std::shared_ptr<WindowGrid> windows = _windowVec[playerId-1];


    // Check if player is stunned for this frame

    if (player->getAnimationState() == Player::IDLE) {
        // Move the player, ignoring collisions
        int moveResult = player->move(moveVec, getSize(), windows);
        if (moveResult == -1 || moveResult == 1) {
            // Request to switch to neighbor's board
            _allCurBoards[playerId - 1] = moveResult;
        }
    }
}



/**
* Called by the client only. Returns a JSON value representing a return to board request
* for sending over the network.
* 
* pre-condition: if not returning, guarantee that the player is on an edge
* 
* Player ids assigned clockwise with host at top
*
*          host: 1
* left: 4            right: 2
*         across: 3
*
* Example Scene Switch Request Message:
* {
*    "player_id":  1,
*    "switch_destination": 1
* }
*
* @param returning  whether the player is returning to their board
* @returns JSON value representing a scene switch
*/
std::shared_ptr<cugl::JsonValue> GameplayController::getJsonSceneSwitch(bool returning) {
    const std::shared_ptr<JsonValue> json = std::make_shared<JsonValue>();
    json->init(JsonValue::Type::ObjectType);
    json->appendValue("player_id", std::to_string(_id));

    if (returning) {
        json->appendValue("switch_destination", std::to_string(0));
    }
    else {
        // pre-condition: if not returning, guarantee that the player is on an edge
        int edge = _playerVec[_id-1]->getEdge(_windowVec[_id - 1]->sideGap, getSize());
        json->appendValue("switch_destination", std::to_string(edge));
    }
    return json;
}

/**
* Called by host only to process return to board requests. Updates a client player's
* currently viewed board for the player at player_id based on the current board
* value stored in the JSON value.
*
* @params data     The data to update
*/
void GameplayController::processSceneSwitchRequest(std::shared_ptr<cugl::JsonValue> data) {

    int playerId = std::stod(data->getString("player_id", "0"));
    int switchDestination = std::stod(data->getString("switch_destination", "0"));

    // update the board of the player to their switch destination
    if (switchDestination == 0) {
        _allCurBoards[playerId - 1] = switchDestination;
    }
}

/**
* Called by client only. Represents a dirt throw action as a JSON value for sending over the network.
* 
* Example Dirt Throw Message
* {
    "player_id_source":  1,
    "player_id_target":  2,
    "dirt_pos": [0, 14.76],
    "dirt_vel": [0.0, 5.0],
    "dirt_dest": [30.2, 122.4],
    "dirt_amount": 10
* }
*
* @param target The id of the player whose board the current player is sending dirt to
* @param pos    The starting position of the dirt projectile
* @param vel    The velocity vector of the dirt projectile
* @param dest   The destination coordinates of the dirt projectile
*
* @returns JSON value representing a dirt throw action
*/
std::shared_ptr<cugl::JsonValue> GameplayController::getJsonDirtThrow(const int target, const cugl::Vec2 pos, const cugl::Vec2 vel, const cugl::Vec2 dest, const int amt) {
    const std::shared_ptr<JsonValue> json = std::make_shared<JsonValue>();
    json->init(JsonValue::Type::ObjectType);
    json->appendValue("player_id_source", std::to_string(_id));
    json->appendValue("player_id_target", std::to_string(target));
    
    cugl::Vec2 boardPos = getBoardPosition(pos);
    const std::shared_ptr<JsonValue> dirtPos = std::make_shared<JsonValue>();
    dirtPos->init(JsonValue::Type::ArrayType);
    dirtPos->appendValue(std::to_string(boardPos.x));
    dirtPos->appendValue(std::to_string(boardPos.y));
    json->appendChild("dirt_pos", dirtPos);

    const std::shared_ptr<JsonValue> dirtVel = std::make_shared<JsonValue>();
    dirtVel->init(JsonValue::Type::ArrayType);
    dirtVel->appendValue(std::to_string(vel.x));
    dirtVel->appendValue(std::to_string(vel.y));
    json->appendChild("dirt_vel", dirtVel);

    cugl::Vec2 boardDest = getBoardPosition(dest);
    const std::shared_ptr<JsonValue> dirtDest = std::make_shared<JsonValue>();
    dirtDest->init(JsonValue::Type::ArrayType);
    dirtDest->appendValue(std::to_string(boardDest.x));
    dirtDest->appendValue(std::to_string(boardDest.y));
    json->appendChild("dirt_dest", dirtDest);

    json->appendValue("dirt_amount", std::to_string(amt));

    // CULog("dirt throw from player %d to %d", _id, target);

    return json;
}

/**
* Called by host only. Updates the boards of both the dirt thrower and the player
* receiving the dirt projectile given the information stored in the JSON value.
*
* @params data     The data to update
*/
void GameplayController::processDirtThrowRequest(std::shared_ptr<cugl::JsonValue> data) {
    int source_id = std::stod(data->getString("player_id_source", "0"));
    int target_id = std::stod(data->getString("player_id_target", "0"));

    _playerVec[source_id - 1]->setAnimationState(Player::THROWING);

    const std::vector<std::shared_ptr<JsonValue>>& pos = data->get("dirt_pos")->children();
    Vec2 dirt_pos(std::stod(pos[0]->asString()), std::stod(pos[1]->asString()));

    const std::vector<std::shared_ptr<JsonValue>>& vel = data->get("dirt_vel")->children();
    Vec2 dirt_vel(std::stod(vel[0]->asString()), std::stod(vel[1]->asString()));

    const std::vector<std::shared_ptr<JsonValue>>& dest = data->get("dirt_dest")->children();
    Vec2 dirt_dest(std::stod(dest[0]->asString()), std::stod(dest[1]->asString()));

    const int amount = std::stoi(data->getString("dirt_amount", "1"));

    _allDirtAmounts[source_id - 1] = max(0, _allDirtAmounts[source_id - 1] - amount);
    _currentDirtAmount = _allDirtAmounts[0];

    _projectileVec[target_id-1]->spawnProjectile(getWorldPosition(dirt_pos), dirt_vel, getWorldPosition(dirt_dest), ProjectileSet::Projectile::ProjectileType::DIRT, amount);
}

/**
 * The method called to update the game mode.
 *
 * This method contains any gameplay code that is not an OpenGL call.
 * 
 * We need to update this method to constantly talk to the server.
 *
 * @param timestep  The amount of time (in seconds) since the last frame
 * @param worldPos  The position of the user's touch in world positions, used for dirt throwing
 * @param dirtCon   The dirt throw input controller used by the game scene
 * @param dirtThrowButton   The dirt throw button from the game scene
 */
void GameplayController::update(float timestep, Vec2 worldPos, DirtThrowInputController& dirtCon, std::shared_ptr<cugl::scene2::Button> dirtThrowButton, std::shared_ptr<cugl::scene2::SceneNode> dirtThrowArc) {
    
    _input.update();


    // get or transmit board states over network
    if (_network.getConnection()) {
        _network.getConnection()->receive([this](const std::string source,
            const std::vector<std::byte>& data) {
                if (!_ishost) {
                    std::shared_ptr<JsonValue> incomingMsg = _network.processMessage(source, data);
                    if (incomingMsg->has("curr_board")) {
                        // CULog("got board state message");
                        updateBoard(incomingMsg);
                    } 
                }
                else { // is host
                    // process action data - movement or dirt throw
                    std::shared_ptr<JsonValue> incomingMsg = _network.processMessage(source, data);
                    if (incomingMsg->has("vel")) {
                        // CULog("got movement message");
                        processMovementRequest(incomingMsg);
                    } else if (incomingMsg->has("switch_destination")) {
                        // CULog("got switch scene request message");
                        processSceneSwitchRequest(incomingMsg);
                    }
                    else if (incomingMsg->has("player_id_target")) {
                        // CULog("got dirt throw message");
                        processDirtThrowRequest(incomingMsg);
                    }
                }
            });
        _network.checkConnection();
    }
    
    // host steps all boards forward
    if (_ishost) {
        // update time
        if (_gameTimeLeft >= 1 && _gameStart) {
            _frame = _frame + 1;
        } if (_frame == _fps) {
            _gameTimeLeft = max(0, _gameTimeLeft - 1);
            _projectileGenChance = 0.95 / (1 + exp(-0.05 * (100 - _gameTimeLeft / 2)));
            _frame = 0;
        }

        // check if game is over when timer hits 0
        if (_gameTimeLeft == 0 && !_gameOver) {
            _gameOver = true;
            vector<int> v;
            for (auto window : _windowVec) {
                if (window == nullptr) continue;
                v.push_back(window->getTotalDirt());
            }
            auto min_idx = std::distance(v.begin(), std::min_element(v.begin(), v.end()));
            _hasWon[min_idx] = true;
        }

        // update bird if active
        if (_birdActive && _gameStart) {
            _bird.move();
            std::shared_ptr<WindowGrid> windows = _windowVec[_curBirdBoard - 1];
            std::shared_ptr<ProjectileSet> projectiles = _projectileVec[_curBirdBoard - 1];

            if (!_birdLeaving && _bird.atColCenter(windows->getNHorizontal(), windows->getPaneWidth(), windows->sideGap) >= 0) {
                std::bernoulli_distribution dist(_projectileGenChance);
                if (dist(_rng)) {
                    // random chance to generate bird poo at column center
                    generatePoo(projectiles);
                }
            }
        }
        
        if (!_gameStart) {
            advanceCountDownAnim();
        } else {
            for (int i = 0; i < _numPlayers; i++) {
                stepForward(_playerVec[i], _windowVec[i], _projectileVec[i]);
            }
        }
        
        std::string host_uuid = _network.getConnection()->getUUID();
        for (auto peer_uuid : _network.getConnection()->getPlayers()) {
            if (peer_uuid == host_uuid) continue; // can skip transmitting information to self
            int id = _UUIDmap[peer_uuid];
            int curBoardId = id; // the id of the board this player is currently viewing
            // calculate which board this current player is on, then transmit necessary information
            if (_allCurBoards[id - 1] != 0) {
                curBoardId = calculateNeighborId(id, _allCurBoards[id - 1], _playerVec);
            }
            // CULog("transmitting board state for player %d", i);
            for (int boardToTransmit = 1; boardToTransmit <= _numPlayers; boardToTransmit++) {
                if (curBoardId == boardToTransmit) {
                    // transmit full board state
                    _network.transmitMessage(peer_uuid, getJsonBoard(boardToTransmit, false));
                }
                else {
                    // transmit partial board state
                    _network.transmitMessage(peer_uuid, getJsonBoard(boardToTransmit, true));
                }
            }
        }
        // _input.update();
        if (_input.didPressReset()) {
            // host resets game for all players
            hostReset();
        }
        // update the game state for self (host). Updates for the rest of the players are done in processMovementRequest(),
        // called whenever the host recieves a movement or other action message.
        _currentDirtAmount = _allDirtAmounts[0];
        _gameWin = _hasWon[0];
        _curBirdPos = getWorldPosition(_bird.birdPosition);

    }
    else {
        // not host - advance all players' animations on local instance
        for (auto player : _playerVec) {
            if (player == nullptr) continue;
            player->advanceAnimation();
        }
    }

    // When the player is on other's board and are able to throw dirt
    int myCurBoard = _allCurBoards[_id - 1];
    if (myCurBoard != 0) {
        bool ifSwitch = false;
        float button_x = myCurBoard == -1 ? getSize().width - _windowVec[_id-1]->sideGap + 150 : _windowVec[_id - 1]->sideGap - 150;
        float arc_start = myCurBoard == -1 ? 270 : 90;
        float arc_rotate_angle = myCurBoard == -1 ? 0 : M_PI;
        cugl::Vec2 buttonPos(button_x, SCENE_HEIGHT / 2);
        dirtThrowButton->setPosition(buttonPos);
        dirtThrowArc->setPosition(buttonPos);
        dirtThrowArc->setAngle(arc_rotate_angle);
        if ((myCurBoard == -1 && _input.getDir().x == 1) || (myCurBoard == 1 && _input.getDir().x == -1)) {
            ifSwitch = true;
        }
        if (_currentDirtAmount > 0) {
            // _dirtThrowInput.update();
            float player_x = myCurBoard == -1 ? getSize().width - _windowVec[_id - 1]->sideGap : _windowVec[_id - 1]->sideGap;
            cugl::Vec2 playerPos(player_x, _playerVec[_id-1]->getPosition().y);
            if (!_dirtSelected) {
                if (dirtCon.didPress() && dirtThrowButton->isDown()) {
                    _dirtSelected = true;
                    _prevInputPos = worldPos;
                }
            } else {
                ifSwitch = false;
                if (dirtCon.didRelease()) {
                    _dirtSelected = false;
                    Vec2 diff = worldPos - _prevInputPos;
                    if ((myCurBoard == 1 && diff.x > 0) || (myCurBoard == -1 && diff.x < 0)) {
                        diff.x = 0;
                    }
                    if (diff.length() > dirtThrowArc->getWidth() / 2) {
                        diff = diff.getNormalization() * dirtThrowArc->getWidth() / 2;
                    }
                    Vec2 destination = playerPos - diff * 7;
                    Vec2 snapped_dest = getBoardPosition(destination);
                    snapped_dest.x = clamp(round(snapped_dest.x), 0.0f, (float)_windowVec[_id - 1]->getNHorizontal()) + 0.5;
                    snapped_dest.y = clamp(round(snapped_dest.y), 0.0f, (float)_windowVec[_id - 1]->getNVertical()) + 0.5;
                    snapped_dest = getWorldPosition(snapped_dest);
                    Vec2 velocity = (snapped_dest - playerPos).getNormalization() * 8;
                    int targetId = calculateNeighborId(_id, myCurBoard, _playerVec);

                    if (_ishost) {
                        processDirtThrowRequest(getJsonDirtThrow(targetId, playerPos, velocity, snapped_dest, _currentDirtAmount));
                    }
                    else {
                        _network.sendToHost(getJsonDirtThrow(targetId, playerPos, velocity, snapped_dest, _currentDirtAmount));
                    }
                    dirtThrowButton->setPosition(buttonPos);
                } else if (dirtCon.isDown()) {
                    // cugl::Vec2 buttonPos(button_x, dirtThrowButton->getPositionY());
                    std::vector<Vec2> vertices = { playerPos };
                    Vec2 diff = worldPos - _prevInputPos;
                    if ((myCurBoard == 1 && diff.x > 0) || (myCurBoard == -1 && diff.x < 0)) {
                        diff.x = 0;
                    }
                    if (diff.length() > dirtThrowArc->getWidth() / 2) {
                        diff = diff.getNormalization() * dirtThrowArc->getWidth() / 2;
                    }
                    Vec2 destination = playerPos - diff * 7;
                    dirtThrowButton->setPosition(buttonPos + diff);
                    Vec2 snapped_dest = getBoardPosition(destination);
                    snapped_dest.x = clamp(round(snapped_dest.x), 0.0f, (float)_windowVec[_id - 1]->getNHorizontal()) + 0.5;
                    snapped_dest.y = clamp(round(snapped_dest.y), 0.0f, (float)_windowVec[_id - 1]->getNVertical()) + 0.5;
                    vertices.push_back(getWorldPosition(snapped_dest));
                    SimpleExtruder se;
                    se.set(Path2(vertices));
                    se.calculate(10);
                    _dirtPath = se.getPolygon();
                }
            }
        }
        if (ifSwitch) {
            switchScene();
        }
    }
    // When a player is on their own board
    else if (myCurBoard == 0 && _gameStart) {
        if (!_ishost) {
            // Read the keyboard for each controller.
//            _input.update();
            // pass movement over network for host to process
            if (_network.getConnection()) {
                _network.checkConnection();
                if (_input.getDir().length() > 0 && !_ishost) {
                    // CULog("transmitting movement message over network for player %d", _id);
                    std::shared_ptr<JsonValue> m = getJsonMove(_input.getDir());
                    std::string s = m->toString();
                    _network.sendToHost(m);
                }
                // send over scene switch requests are handled by button listener
            }
        }
        if (_ishost) {
            // Check if player is stunned for this frame
            if (_playerVec[_id-1]->getAnimationState() == Player::IDLE) {
                // Move the player, ignoring collisions
                int moveResult = _playerVec[_id - 1]->move(_input.getDir(), getSize(), _windowVec[_id - 1]);
                if (_numPlayers > 1 && (moveResult == -1 || moveResult == 1)) {
                    _allCurBoards[0] = moveResult;
                } 
            }
        }
    }

    // advance bird flying frame
    _bird.advanceBirdFrame();

    // update frame count for win / lose screen
    // if a number of frames have passed,
    // we will call setRequestForMenu
    // to let app know that we should
    // switch to the main menu
    if (_gameOver) {
        _frameCountForWin = _frameCountForWin +1;
    }
    
    if (_frameCountForWin>4*_fps && _gameOver) {
        setRequestForMenu(true);
    };  
}

/**
 * This helper method calculates all the grid coordinates in which dirt should land in
 * given a center (where the player has aimed) and the total amount of dirt to spawn.
 *
 * This takes into account the size of the window grid and attempts to spawn the dirt close
 * to a circle. It does not spawn any dirt out of bounds. For example, if the center is
 * close to the edge of the grid, all the extra dirt that would have landed out of bounds
 * is pushed inside.
 */
std::vector<cugl::Vec2> calculateLandedDirtPositions(const int width, const int height, Vec2 centerCoords, int amount) {
    // CULog("center: %f, %f", centerCoords.x, centerCoords.y);
    std::vector<cugl::Vec2> dirtPositions;

    if (amount <= 0) {
        // No dirt to spawn, return empty vector
        return dirtPositions;
    }

    dirtPositions.emplace_back(centerCoords); // Start with one dirt at the center
    amount--; // Decrement the amount of dirt left to spawn

    int layer = 1;

    while (amount > 0) {
        // start at top corner of diamond
        int currX = centerCoords.x;
        int currY = centerCoords.y + layer;

        // move down and right until at right corner of diamond
        while (amount > 0 && currY != centerCoords.y) {
            currX += 1;
            currY -= 1;
            if (currX >= 0 && currX < height && currY >= 0 && currY < width) {
                dirtPositions.emplace_back(Vec2(currX, currY));
                // CULog("dirt spawned at %d, %d", currX, currY);
                amount--;
            }
        }
        // move down and left until at bottom corner of diamond
        while (amount > 0 && currX != centerCoords.x) {
            currX -= 1;
            currY -= 1;
            if (currX >= 0 && currX < height && currY >= 0 && currY < width) {
                dirtPositions.emplace_back(Vec2(currX, currY));
                // CULog("dirt spawned at %d, %d", currX, currY);
                amount--;
            }
        }
        // move up and left until at right corner of diamond
        while (amount > 0 && currY != centerCoords.y) {
            currX -= 1;
            currY += 1;
            if (currX >= 0 && currX < height && currY >= 0 && currY < width) {
                dirtPositions.emplace_back(Vec2(currX, currY));
                // CULog("dirt spawned at %d, %d", currX, currY);
                amount--;
            }
        }
        // move up and right until back at the top corner of diamond
        while (amount > 0 && currX != centerCoords.x) {
            currX += 1;
            currY += 1;
            if (currX >= 0 && currX < height && currY >= 0 && currY < width) {
                dirtPositions.emplace_back(Vec2(currX, currY));
                // CULog("dirt spawned at %d, %d", currX, currY);
                amount--;
            }
        }
        layer++;
    }

    return dirtPositions;
}

/**
* FOR HOST ONLY. This method does all the heavy lifting work for update.
* The host steps forward each player's game state, given references to the player, board, and projectile set.
*/
void GameplayController::stepForward(std::shared_ptr<Player>& player, std::shared_ptr<WindowGrid>& windows, std::shared_ptr<ProjectileSet>& projectiles) {
    int player_id = player->getId();
    
    if (windows->getTotalDirt() == 0 && !_gameOver) {
        _gameOver = true;
        _hasWon[player_id - 1] = true;
        _curBirdBoard = rand() % _numPlayers + 1;
    }

    // calculate progress
    float numWindowPanes = windows->getNHorizontal() * windows->getNVertical();
    auto progress = (numWindowPanes - windows->getTotalDirt()) / numWindowPanes;
    _progressVec[player_id - 1] = progress;

    std::vector<std::pair<cugl::Vec2, int>> landedDirts;

    if (_allCurBoards[player_id - 1] == 0) {
        // only check if player is stunned, has removed dirt, or collided with projectile
        // if they are on their own board.

        // note: the frame updates only update to the host's local state.
        // the host only sends over movement/positioning updates
        // if (player->getStunFrames() > 0) {
        //     player->decreaseStunFrames();
        // } 
        // else {
        //     player->move();
        // }
        
        player->move();
        
        // remove any dirt the player collides with
        Vec2 grid_coors = player->getCoorsFromPos(windows->getPaneHeight(), windows->getPaneWidth(), windows->sideGap);
        player->setCoors(grid_coors);

        int clamped_y = std::clamp(static_cast<int>(grid_coors.y), 0, windows->getNVertical() - 1);
        int clamped_x = std::clamp(static_cast<int>(grid_coors.x), 0, windows->getNHorizontal() - 1);
        bool dirtRemoved = windows->removeDirt(clamped_y, clamped_x);
        if (dirtRemoved) {
            // filling up dirty bucket
            // set amount of frames plaer is frozen for for cleaning dirt
            player->setAnimationState(Player::AnimStatus::WIPING);
            if (player_id == _id) {
                AudioEngine::get()->play("clean", _clean, false, _clean->getVolume(), true);
            };
            _allDirtAmounts[player_id - 1] = min(_maxDirtAmount, _allDirtAmounts[player_id - 1] + 1);
        }

        // Check for collisions and play sound
        auto collision_result = _collisions.resolveCollision(player, projectiles);
        if (collision_result.first) { // if collision occurred
            if (player_id == _id) {
                AudioEngine::get()->play("bang", _bang, false, _bang->getVolume(), true);
            };
            player->setAnimationState(Player::STUNNED);
            if (collision_result.second.has_value()) {
                landedDirts.push_back(collision_result.second.value());
            }
        }
        player->advanceAnimation();
        
        if (!_birdLeaving && _curBirdBoard == player_id && _collisions.resolveBirdCollision(player, _bird, getWorldPosition(_bird.birdPosition), 0.5)) {
            // set amount of frames plaer is frozen for for shooing bird
            if (player->getAnimationState() == Player::AnimStatus::IDLE) {
                player->setAnimationState(Player::AnimStatus::SHOOING);
                _bird.resetBirdPathToExit(windows->getNHorizontal());
                _birdLeaving = true;
            }
        }
        
        //if (_birdLeaving && player->getAnimationState() != Player::SHOOING) {
          //  // send bird away after shooing
          //  _bird.resetBirdPathToExit(windows->getNHorizontal());
       // }
        
        if (_birdLeaving && _bird.birdReachesExit()) {
            _curBirdBoard = std::distance(_progressVec.begin(), std::max_element(_progressVec.begin(), _progressVec.end())) + 1;
            
            std::uniform_int_distribution<> distr(0, windows->getNVertical() - 1);
            int spawnRow = distr(_rng);
//            CULog("bird spawn at row %d", spawnRow);
            _bird.resetBirdPath(windows->getNVertical(), windows->getNHorizontal(), spawnRow);
            _birdLeaving = false;
        }
    }

    // Move the projectiles, get the center destination and amount of landed dirts
    auto landedProjs = projectiles->update(getSize());;
    landedDirts.insert(landedDirts.end(), landedProjs.begin(), landedProjs.end());
    // Add any landed dirts
    for (auto landedDirt : landedDirts) {
        cugl::Vec2 center = landedDirt.first;
        int x_coor = (int)((center.x - windows->sideGap) / windows->getPaneWidth());
        int y_coor = (int)(center.y / windows->getPaneHeight());
        x_coor = std::clamp(x_coor, 0, windows->getNHorizontal() - 1);
        y_coor = std::clamp(y_coor, 0, windows->getNVertical() - 1);

        int amount = landedDirt.second;
        std::vector<cugl::Vec2> landedCoords = calculateLandedDirtPositions(windows->getNVertical(), windows->getNHorizontal(), Vec2(x_coor, y_coor), amount);
        for (cugl::Vec2 dirtPos : landedCoords) {
            windows->addDirt(dirtPos.y, dirtPos.x);
        }
        
    }
}



/** update when dirt is generated */
void GameplayController::updateDirtGenTime() {
    _dirtGenTimes.clear();
    std::uniform_int_distribution<> distr(0, _fixedDirtUpdateThreshold);
    for(int n=0; n<_dirtGenSpeed; ++n) {
        _dirtGenTimes.insert(distr(_rng));
    }
}

/** handles dirt generation */
void GameplayController::generateDirt() {
    std::uniform_int_distribution<int> rowDist(0, _windowVec[_id-1]->getNVertical() - 1);
    std::uniform_int_distribution<int> colDist(0, _windowVec[_id - 1]->getNHorizontal() - 1);
    int rand_row = rowDist(_rng);
    int rand_col = colDist(_rng);
//    CULog("player at: (%f, %f)", _player->getCoors().y, _player->getCoors().x);
//    CULog("generate at: (%d, %d)", (int)rand_row, (int) rand_col);

    // if add dirt already exists at location or player at location and board is not full, repeat
    while (Vec2((int)_playerVec[_id - 1]->getCoors().y, (int)_playerVec[_id - 1]->getCoors().x) == Vec2((int)rand_row, (int)rand_col) || !_windowVec[_id - 1]->addDirt(rand_row, rand_col)) {
        rand_row = rowDist(_rng);
        rand_col = colDist(_rng);
    }
}

/** handles poo generation */
void GameplayController::generatePoo(std::shared_ptr<ProjectileSet> projectiles) {
//    CULog("player at: (%f, %f)", _player->getCoors().y, _player->getCoors().x);
//    CULog("generate at: %d", (int)rand_row);

// if add dirt already exists at location or player at location and board is not full, repeat
    cugl::Vec2 birdWorldPos = getWorldPosition(_bird.birdPosition);
    // randomize the destination of bird poo, any window pane below the current bird position
    std::uniform_int_distribution<int> rowDist(0, floor(_bird.birdPosition.y));
    int rand_row_center = rowDist(_rng);
    cugl::Vec2 birdPooDest = getWorldPosition(Vec2(_bird.birdPosition.x, rand_row_center));
    projectiles->spawnProjectile(Vec2(birdWorldPos.x, birdWorldPos.y - _windowVec[_id - 1]->getPaneHeight()/2), Vec2(0, min(-2.4f,-2-_projectileGenChance)), birdPooDest, ProjectileSet::Projectile::ProjectileType::POOP);
}

/** Checks whether board is full except player current location*/
const bool GameplayController::checkBoardFull() {
    for (int x = 0; x < _windowVec[_id - 1]->getNHorizontal(); x++) {
        for (int y = 0; y < _windowVec[_id - 1]->getNVertical(); y++) {
                if (_windowVec[_id - 1]->getWindowState(y, x) == 0) {
                    if (Vec2((int)_playerVec[_id - 1]->getCoors().y, (int)_playerVec[_id - 1]->getCoors().x) == Vec2(y, x)) {
                        // consider current place occupied
                        continue;
                    }
                    return false;
                }
        }
    }
    return true; // No 0s found, all dirty spots
}

/**
 * Draws all this scene to the given SpriteBatch.
 *
 * The default implementation of this method simply draws the scene graph
 * to the sprite batch.  By overriding it, you can do custom drawing
 * in its place.
 *
 * @param batch     The SpriteBatch to draw with.
 */
void GameplayController::draw(const std::shared_ptr<cugl::SpriteBatch>& batch) {    
    auto player = _playerVec[_id - 1];
    int leftId = calculateNeighborId(_id, -1, _playerVec);
    int rightId = calculateNeighborId(_id, 1, _playerVec);
    auto playerLeft = _playerVec[leftId - 1];
    auto playerRight = _playerVec[rightId - 1];
    if (_allCurBoards[_id-1] == 0) {
        _windowVec[_id-1]->draw(batch, getSize());
        player->draw(batch, getSize());

        if (leftId != _id && rightId != _id) {
            Affine2 leftTrans = Affine2();
            leftTrans.translate(playerLeft->getProfileTexture()->getSize() * -0.5);
            leftTrans.scale(0.4);
            leftTrans.translate(_windowVec[_id - 1]->sideGap - 50, player->getPosition().y);
            batch->draw(playerLeft->getProfileTexture(), Vec2(), leftTrans);
            Affine2 leftTransArrow = Affine2();
            leftTransArrow.scale(0.75);
            leftTransArrow.translate(_windowVec[_id - 1]->sideGap - 130, player->getPosition().y - (_arrowTexture->getHeight() / 2));
            batch->draw(_arrowTexture, Vec2(), leftTransArrow);

            Affine2 rightTrans = Affine2();
            rightTrans.translate(playerRight->getProfileTexture()->getSize() * -0.5);
            rightTrans.scale(0.4);
            rightTrans.translate(getSize().width - _windowVec[_id - 1]->sideGap + 50, player->getPosition().y);
            batch->draw(playerRight->getProfileTexture(), Vec2(), rightTrans);
            Affine2 rightTransArrow = Affine2();
            rightTransArrow.scale(Vec2(-0.75, 0.75));
            rightTransArrow.translate(getSize().width - _windowVec[_id - 1]->sideGap + 130, player->getPosition().y - (_arrowTexture->getHeight() / 2));
            batch->draw(_arrowTexture, Vec2(), rightTransArrow);
        }

        if (_allCurBoards[leftId-1] == 1) {
            // left neighbor is on this player's board
            playerLeft->drawPeeking(batch, getSize(), _allCurBoards[leftId - 1], _windowVec[_id-1]->sideGap);
            // TODO: draw danger/warning
        }
        if (_allCurBoards[rightId-1] == -1) {
            // right neighbor is on this player's board
            playerRight->drawPeeking(batch, getSize(), _allCurBoards[rightId - 1], _windowVec[_id - 1]->sideGap);
            // TODO: draw danger/warning
        }
        _projectileVec[_id-1]->draw(batch, getSize(), _windowVec[_id - 1]->getPaneWidth(), _windowVec[_id - 1]->getPaneHeight());
        if (_curBirdBoard == _id) {
            _bird.draw(batch, getSize(), _curBirdPos);
        }
    }
    else if (_allCurBoards[_id-1] == -1 && leftId != _id) {
        _windowVec[leftId - 1]->draw(batch, getSize());
        if (_allCurBoards[leftId - 1] == 0) {
            playerLeft->draw(batch, getSize());
        }
        player->drawPeeking(batch, getSize(), _allCurBoards[_id - 1], _windowVec[_id - 1]->sideGap);
        _projectileVec[leftId-1]->draw(batch, getSize(), _windowVec[leftId - 1]->getPaneWidth(), _windowVec[leftId - 1]->getPaneHeight());

        vector<Vec2> potentialDirts;
        if (_dirtSelected && _dirtPath.size() != 0) {
            batch->setColor(Color4::BLACK);
            batch->fill(_dirtPath);
            Vec2 dirtDest = _dirtPath.getVertices().back() - Vec2(0.5, 0.5);
            Vec2 landedDirtCoords = getBoardPosition(dirtDest);
            landedDirtCoords.y = std::clamp(static_cast<int>(landedDirtCoords.y), 0, _windowVec[leftId - 1]->getNVertical() - 1);
            landedDirtCoords.x = std::clamp(static_cast<int>(landedDirtCoords.x), 0, _windowVec[leftId - 1]->getNHorizontal() - 1);
            potentialDirts = calculateLandedDirtPositions(_windowVec[leftId - 1]->getNVertical(), _windowVec[leftId - 1]->getNHorizontal(), landedDirtCoords, _currentDirtAmount);
        }
        if (potentialDirts.size() > 0) {
            _windowVec[leftId - 1]->drawPotentialDirt(batch, getSize(), potentialDirts);
        }

        if (_curBirdBoard == leftId) {
            _bird.draw(batch, getSize(), _curBirdPos);
        }
    }
    else if (_allCurBoards[_id - 1] == 1 && rightId != _id) {
        _windowVec[rightId - 1]->draw(batch, getSize());
        if (_allCurBoards[rightId - 1] == 0) {
            playerRight->draw(batch, getSize());
        }
        player->drawPeeking(batch, getSize(), _allCurBoards[_id - 1], _windowVec[_id - 1]->sideGap);
        _projectileVec[rightId-1]->draw(batch, getSize(), _windowVec[rightId - 1]->getPaneWidth(), _windowVec[rightId - 1]->getPaneHeight());

        vector<Vec2> potentialDirts;
        if (_dirtSelected && _dirtPath.size() != 0) {
            batch->setColor(Color4::BLACK);
            batch->fill(_dirtPath);
            Vec2 dirtDest = _dirtPath.getVertices().back();
            Vec2 landedDirtCoords = getBoardPosition(dirtDest);
            landedDirtCoords.y = std::clamp(static_cast<int>(landedDirtCoords.y), 0, _windowVec[rightId - 1]->getNVertical() - 1);
            landedDirtCoords.x = std::clamp(static_cast<int>(landedDirtCoords.x), 0, _windowVec[rightId - 1]->getNHorizontal() - 1);
            potentialDirts = calculateLandedDirtPositions(_windowVec[rightId - 1]->getNVertical(), _windowVec[rightId - 1]->getNHorizontal(), landedDirtCoords, _currentDirtAmount);
        }
        if (potentialDirts.size() > 0) {
            _windowVec[rightId - 1]->drawPotentialDirt(batch, getSize(), potentialDirts);
        }
        if (_curBirdBoard == rightId) {
            _bird.draw(batch, getSize(), _curBirdPos);
        }
    }
}


void GameplayController::setActive(bool f) {
    // yes this code is bad and needs to be reworked
    if (!f) {
        _isActive=false;
        setRequestForMenu(false);
        setGameOver(false);
        setGameStart(false);
        setWin(false);
    } else {
        _isActive = true;
        _audioController->playGameplayMusic();
        setRequestForMenu(false);
        setGameOver(false);
        setGameStart(false);
        setWin(false);
        _frameCountForWin = 0;
        _countDownFrames = 0;
    };
    _gameTimeLeft = _gameTime;
}

void GameplayController::drawCountdown(const std::shared_ptr<cugl::SpriteBatch>& batch, cugl::Vec3 campos, cugl::Size s) {
    if (!_gameStart) {
        Affine2 countdownTrans;
        Affine2 sparkleTrans;
        double countdownScale;
        double sparkleHScale;
        double sparkleWScale;
        std::shared_ptr<cugl::SpriteSheet> currentCountdownSprite = getCurrentCountdownSprite();
        currentCountdownSprite->setOrigin(Vec2(currentCountdownSprite->getFrameSize().width/2, currentCountdownSprite->getFrameSize().height/2));
        _countdownSparkleSprite->setOrigin(Vec2(_countdownSparkleSprite->getFrameSize().width/2, _countdownSparkleSprite->getFrameSize().height/2));
        countdownScale = (float)getSize().getIHeight() / currentCountdownSprite->getFrameSize().height / 2;
        sparkleHScale = (float)getSize().getIHeight() / _countdownSparkleSprite->getFrameSize().height / 2;
        sparkleWScale = (float)getSize().getIWidth() / _countdownSparkleSprite->getFrameSize().width * 1.4 / 2;
        countdownTrans.scale(countdownScale);
        sparkleTrans.scale(Vec2(sparkleWScale, sparkleHScale));
        countdownTrans.translate(campos);
        sparkleTrans.translate(campos);
        _countdownSparkleSprite->draw(batch, sparkleTrans);
        currentCountdownSprite->draw(batch, countdownTrans);
    }
}
