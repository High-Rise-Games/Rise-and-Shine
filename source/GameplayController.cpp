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
    
    // time of the game set to 200 seconds
    _gameTime = 200;
    _gameTimeLeft = _gameTime;
    

    _frame=0;
    
    // Initialize a game audio controller
    _audioController.init(assets);
    
    // we set game win and game over to false
    _gameWin = false;
    _gameOver = false;
    _transitionToMenu = false;
    
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

    // Initialize existence of enemies
    _birdActive = true;
    _birdLeaving = false;

    // Initialize all starting current boards
    _curBoard = 0;
    _curBoardRight = 0;
    _curBoardLeft = 0;
    
    
    return true;
}

bool GameplayController::initLevel(int selected_level) {
    // Initialize the window grids
//    std::shared_ptr<cugl::JsonValue> level = _constants->get("easy board"); // TODO: make field passed in from level select through App
    std::shared_ptr<cugl::JsonValue> level = std::make_shared<JsonValue>();
    switch (selected_level) {
        case 1:
            // CULog("garage selecting 1");
            level = _assets->get<JsonValue>("templatelevel");
            _size = _nativeSize;
            _size.height *= 1.5;
            break;
        case 2:
            // CULog("garage selecting 2");
            level = _assets->get<JsonValue>("templatelevel2");
            _size = _nativeSize;
            _size.height *= 2;
            break;
        case 3:
            // CULog("garage selecting 3");
            level = _assets->get<JsonValue>("templatelevel3");
            _size = _nativeSize;
            _size.height *= 3;
            break;
        default:
            // CULog("garage selecting default");
            level = _assets->get<JsonValue>("templatelevel");
            _size = _nativeSize;
            _size.height *= 1.5;
            break;
    }

    string window_strings[13] = { "window_1", "window_2", "window_3", "window_4", "window_5", "window_6", "window_7", "window_8", "window_9", "window_10", "window_11", "window_12", "window_13", };
    
    _windows.setBuildingTexture(_assets->get<Texture>("building_1"));
    for (string thisWindow: window_strings) {
        _windows.addTexture(_assets->get<Texture>(thisWindow));
    }
    //_windows.addTexture(assets->get<Texture>("window_1"));
    //_windows.addTexture(assets->get<Texture>("window_2"));
    //_windows.addTexture(assets->get<Texture>("window_3"));
    _windows.init(level, _size); // init depends on texture
    _windows.setInitDirtNum(selected_level * 5);
    _windows.setDirtTexture(_assets->get<Texture>("dirt"));
    _windows.setFadedDirtTexture(_assets->get<Texture>("faded-dirt"));
    
    // get the win background when game is win
    _winBackground = _assets->get<Texture>("win-background");
    
    // get the lose background when game is lose
    _loseBackground = _assets->get<Texture>("lose-background");

    _windowsLeft.setBuildingTexture(_assets->get<Texture>("building_1"));
    for (string thisWindow: window_strings) {
        _windowsLeft.addTexture(_assets->get<Texture>(thisWindow));
    }
    //_windowsLeft.addTexture(assets->get<Texture>("window_1"));
    //_windowsLeft.addTexture(assets->get<Texture>("window_2"));
    //_windowsLeft.addTexture(assets->get<Texture>("window_3"));
    _windowsLeft.init(level, _size); // init depends on texture
    _windowsLeft.setDirtTexture(_assets->get<Texture>("dirt"));
    _windowsLeft.setFadedDirtTexture(_assets->get<Texture>("faded-dirt"));

    _windowsRight.setBuildingTexture(_assets->get<Texture>("building_1"));
    for (string thisWindow: window_strings) {
        _windowsRight.addTexture(_assets->get<Texture>(thisWindow));
    }
    //_windowsRight.addTexture(assets->get<Texture>("window_1"));
    //_windowsRight.addTexture(assets->get<Texture>("window_2"));
    //_windowsRight.addTexture(assets->get<Texture>("window_3"));
    _windowsRight.init(level, _size); // init depends on texture
    _windowsRight.setDirtTexture(_assets->get<Texture>("dirt"));
    _windowsRight.setFadedDirtTexture(_assets->get<Texture>("faded-dirt"));

    _windowsAcross.setBuildingTexture(_assets->get<Texture>("building_1"));
    for (string thisWindow : window_strings) {
        _windowsAcross.addTexture(_assets->get<Texture>(thisWindow));
    }
    //_windowsAcross.addTexture(assets->get<Texture>("window_1"));
    //_windowsAcross.addTexture(assets->get<Texture>("window_2"));
    //_windowsAcross.addTexture(assets->get<Texture>("window_3"));
    _windowsAcross.init(level, getSize()); // init depends on texture
    _windowsAcross.setDirtTexture(_assets->get<Texture>("dirt"));
    _windowsAcross.setFadedDirtTexture(_assets->get<Texture>("faded-dirt"));

    // Initialize projectiles
    _projectiles.setDirtTexture(_assets->get<Texture>("dirt"));
    _projectiles.setPoopTexture(_assets->get<Texture>("poop"));
    _projectiles.setTextureScales(_windows.getPaneHeight(), _windows.getPaneWidth());
//    _projectiles.init(_constants->get("projectiles"));

    _projectilesLeft.setDirtTexture(_assets->get<Texture>("dirt"));
    _projectilesLeft.setPoopTexture(_assets->get<Texture>("poop"));
    _projectilesLeft.setTextureScales(_windowsLeft.getPaneHeight(), _windowsLeft.getPaneWidth());

    _projectilesRight.setDirtTexture(_assets->get<Texture>("dirt"));
    _projectilesRight.setPoopTexture(_assets->get<Texture>("poop"));
    _projectilesRight.setTextureScales(_windowsRight.getPaneHeight(), _windowsRight.getPaneWidth());

    _projectilesAcross.setDirtTexture(_assets->get<Texture>("dirt"));
    _projectilesAcross.setPoopTexture(_assets->get<Texture>("poop"));
    _projectilesAcross.setTextureScales(_windows.getPaneHeight(), _windows.getPaneWidth());

    // Initialize bird textures, but do not set a location yet. that is the host's job
    if (_birdActive) {
        cugl::Vec2 birdTopLeftPos = cugl::Vec2(0.4, _windows.getNVertical() - 0.5);
        cugl::Vec2 birdTopRightPos = cugl::Vec2(_windows.getNHorizontal() - 0.6, _windows.getNVertical() - 0.5);
        cugl::Vec2 birdBotLeftPos = cugl::Vec2(0.4, _windows.getNVertical() - 3.5);
        cugl::Vec2 birdBotRightPos = cugl::Vec2(_windows.getNHorizontal() - 0.6, _windows.getNVertical() - 3.5);
        std::vector<cugl::Vec2> positions = {birdTopLeftPos, birdTopRightPos, birdBotLeftPos, birdBotRightPos};
        _bird.init(positions, 0.01, 0.04, _windows.getPaneHeight());
        _curBirdBoard = 2;
        _bird.setTexture(_assets->get<Texture>("bird"));
    }
    
    // Make a ship and set its texture
    // starting position is most bottom left window
    Vec2 startingPos = Vec2(_windows.sideGap + (_windows.getPaneWidth() / 2), _windows.getPaneHeight());

    // no ids given yet - to be assigned in initPlayers()
    _player = std::make_shared<Player>(-1, startingPos, _constants->get("ship"), _windows.getPaneHeight(), _windows.getPaneWidth());
    changeCharTexture(_player, ""); // sets to default mushroom
    
    _playerLeft = std::make_shared<Player>(-1, startingPos, _constants->get("ship"), _windows.getPaneHeight(), _windows.getPaneWidth());
    changeCharTexture(_playerLeft, "");

    _playerRight = std::make_shared <Player>(-1, startingPos, _constants->get("ship"), _windows.getPaneHeight(), _windows.getPaneWidth());
    changeCharTexture(_playerRight, "");

    _playerAcross = std::make_shared<Player>(-1, startingPos, _constants->get("ship"), _windows.getPaneHeight(), _windows.getPaneWidth());
    changeCharTexture(_playerAcross, "");

    // Initialize random dirt generation
    updateDirtGenTime();

    _collisions.init(_size);

    // Get the bang sound
    _bang = _assets->get<Sound>("bang");
    _clean = _assets->get<Sound>("clean");
    
    reset();
    return true;
}

/** 
 * Initializes the player models for all players, whether host or client. 
 * Sets IDs (corresponds to side of building - TODO: change?), textures (TODO)
 */
bool GameplayController::initPlayers(const std::shared_ptr<cugl::AssetManager>& assets) {
    if (assets == nullptr) {
        return false;
    }

    // id of self is set while connecting to the lobby. all other players have ids set, even if they don't exist.
    _player->setId(_id);
    int leftId = _id == 1 ? 4 : _id - 1;
    _playerLeft->setId(leftId);
    int rightId = _id == 4 ? 1 : _id + 1;
    _playerRight->setId(rightId);
    int acrossId = (_id + 2) % 4;
    if (acrossId == 0) acrossId = 4;
    _playerAcross->setId(acrossId);

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

    _numPlayers = _network.getNumPlayers();
    initPlayers(assets);

    if (_birdActive) {
        // randomly place bird on a player's board
        _boardWithBird = rand() % _numPlayers + 1;
    }

    // starting position is most bottom left window
    Vec2 startingPos = Vec2(_windows.sideGap + (_windows.getPaneWidth() / 2), _windows.getPaneHeight());

    hostReset();

    return true;
}


#pragma mark -
#pragma mark Gameplay Handling
void GameplayController::reset() {
    Vec2 startingPos = Vec2(_windows.sideGap + (_windows.getPaneWidth() / 2), _windows.getPaneHeight() / 2);
    _player->setPosition(startingPos);
    //_player->setAngle(0);
    _player->setVelocity(Vec2::ZERO);
    _player->setHealth(_constants->get("ship")->getInt("health", 0));

    _playerLeft->setPosition(startingPos);
    //_playerLeft->setAngle(0);
    _playerLeft->setVelocity(Vec2::ZERO);
    _playerLeft->setHealth(_constants->get("ship")->getInt("health", 0));

    _playerRight->setPosition(startingPos);
    //_playerRight->setAngle(0);
    _playerRight->setVelocity(Vec2::ZERO);
    _playerRight->setHealth(_constants->get("ship")->getInt("health", 0));

    _windows.clearBoard();
    _windows.generateInitialBoard(_windows.getInitDirtNum());
    _windowsLeft.clearBoard();
    _windowsLeft.generateInitialBoard(_windows.getInitDirtNum());
    _windowsRight.clearBoard();
    _windowsRight.generateInitialBoard(_windows.getInitDirtNum());

    _projectiles.current.clear();
//    _projectiles.init(_constants->get("projectiles"));
    _projectilesLeft.current.clear();
    _projectilesLeft.init(_constants->get("projectiles"));
    _projectilesRight.current.clear();
    _projectilesRight.init(_constants->get("projectiles"));

    _dirtThrowTimer = 0;
    _projectileGenChance = 0.1;
    _projectileGenCountDown = 120;
    _currentDirtAmount = 0;
    _curBoard = 0;
}

/**
 * Resets the status of the game for all players so that we can play again.
 */
void GameplayController::hostReset() {
    reset();
    // starting position is most bottom left window
    Vec2 startingPos = Vec2(_windows.sideGap+(_windows.getPaneWidth()/2), _windows.getPaneHeight()/2);

    _playerAcross->setPosition(startingPos);
    //_playerAcross->setAngle(0);
    _playerAcross->setVelocity(Vec2::ZERO);
    _playerAcross->setHealth(_constants->get("ship")->getInt("health", 0));
    
    _windowsAcross.clearBoard();
    _windowsAcross.generateInitialBoard(_windows.getInitDirtNum());

    _projectilesAcross.current.clear();
    _projectilesAcross.init(_constants->get("projectiles"));

    _allDirtAmounts = { 0, 0, 0, 0 };
    _allCurBoards = { 0, 0, 0, 0 };
}

/**
* HOST ONLY. Sets the character of the player given player's id.
* Possible values: "Mushroom", "Frog", "Flower", "Chameleon"
*/
void GameplayController::setCharacters(std::vector<std::string>& chars) {
    for (int id = 0; id < _numPlayers; id++) {
        switch (id+1) {
        case 1:
            changeCharTexture(_player, chars[id]);
            _player->setChar(chars[id]);
            break;
        case 2:
            changeCharTexture(_playerRight, chars[id]);
            _playerRight->setChar(chars[id]);
            break;
        case 3:
            changeCharTexture(_playerAcross, chars[id]);
            _playerAcross->setChar(chars[id]);
            break;
        default:
            changeCharTexture(_playerLeft, chars[id]);
            _playerLeft->setChar(chars[id]);
            break;
        }
    }
}

void GameplayController::changeCharTexture(std::shared_ptr<Player>& player, std::string charChoice) {
    if (charChoice == "Frog") {
        player->setIdleTexture(_assets->get<Texture>("idle_frog"));
        player->setWipeTexture(_assets->get<Texture>("wipe_frog"));
        player->setShooTexture(_assets->get<Texture>("shoo_frog"));
        player->setThrowTexture(_assets->get<Texture>("throw_frog"));
    }
    else if (charChoice == "Flower") {
        player->setIdleTexture(_assets->get<Texture>("idle_flower"));
        player->setWipeTexture(_assets->get<Texture>("wipe_flower"));
        player->setShooTexture(_assets->get<Texture>("shoo_flower"));
        player->setThrowTexture(_assets->get<Texture>("throw_flower"));
    }
    else if (charChoice == "Chameleon") {
        player->setIdleTexture(_assets->get<Texture>("idle_chameleon"));
        player->setWipeTexture(_assets->get<Texture>("wipe_chameleon"));
        player->setShooTexture(_assets->get<Texture>("shoo_chameleon"));
        player->setThrowTexture(_assets->get<Texture>("throw_chameleon"));
    }
    else {
        player->setIdleTexture(_assets->get<Texture>("idle_mushroom"));
        player->setWipeTexture(_assets->get<Texture>("wipe_mushroom"));
        player->setShooTexture(_assets->get<Texture>("shoo_mushroom"));
        player->setThrowTexture(_assets->get<Texture>("throw_mushroom"));
    }
}

/**
* Given the world positions, convert it to the board position
* based off of grid coordinates. Ex. [2, 3] or [2.3, 3] if the
* player is in the process of moving in between x = 2 and x = 3.
*/
cugl::Vec2 GameplayController::getBoardPosition(cugl::Vec2 worldPos) {
    float x_coor = (worldPos.x - _windows.sideGap) / _windows.getPaneWidth();
    float y_coor = worldPos.y / _windows.getPaneHeight();
    
    return Vec2(x_coor, y_coor);
}

/**
* Given the board positions, convert it to the world position.
*/
cugl::Vec2 GameplayController::getWorldPosition(cugl::Vec2 boardPos) {
    float x_coor = boardPos.x * _windows.getPaneWidth() + _windows.sideGap;
    float y_coor = boardPos.y * _windows.getPaneHeight();
    return Vec2(x_coor, y_coor);
}

/**
 * Method for the return to board button listener used in GameScene
 */
void GameplayController::switchScene() {
    if (_curBoard != 0) {
        if (_ishost) {
            _allCurBoards[0] = 0;
            _curBoard = 0;
        }
        else {
            _network.transmitMessage(getJsonSceneSwitch(true));
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
std::shared_ptr<cugl::JsonValue> GameplayController::getJsonBoard(int id) {
    std::shared_ptr<Player> player;
    WindowGrid* windows;
    ProjectileSet* projectiles;

    if (id == 1) {
        player = _player;
        windows = &_windows;
        projectiles = &_projectiles;
    }
    else if (id == 2) {
        player = _playerRight;
        windows = &_windowsRight;
        projectiles = &_projectilesRight;
    }
    else if (id == 3) {
        player = _playerAcross;
        windows = &_windowsAcross;
        projectiles = &_projectilesAcross;
    }
    else {
        player = _playerLeft;
        windows = &_windowsLeft;
        projectiles = &_projectilesLeft;
    }


    const std::shared_ptr<JsonValue> json = std::make_shared<JsonValue>();
    json->init(JsonValue::Type::ObjectType);
    json->appendValue("player_id", std::to_string(id));
    json->appendValue("player_char", player->getChar());
    std::string win_str = checkBoardEmpty(*windows) ? "true" : "false";
    json->appendValue("has_won", win_str);
    json->appendValue("num_dirt", std::to_string(_allDirtAmounts[id - 1]));
    json->appendValue("curr_board", std::to_string(_allCurBoards[id - 1]));

    cugl::Vec2 playerBoardPos = getBoardPosition(player->getPosition());
    json->appendValue("player_x", std::to_string(playerBoardPos.x));
    json->appendValue("player_y", std::to_string(playerBoardPos.y));

    json->appendValue("health", std::to_string((player->getHealth())));
    json->appendValue("stun_frames", std::to_string(player->getStunFrames()));
    json->appendValue("wipe_frames", std::to_string(player->getWipeFrames()));
    json->appendValue("shoo_frames", std::to_string(player->getShooFrames()));
    json->appendValue("timer", std::to_string(_gameTimeLeft));
    json->appendValue("has_bird", (id == _boardWithBird));
    
    const std::shared_ptr<JsonValue> birdPos = std::make_shared<JsonValue>();
    birdPos->init(JsonValue::Type::ArrayType);
    birdPos->appendValue(std::to_string(_bird.birdPosition.x));
    birdPos->appendValue(std::to_string(_bird.birdPosition.y));
    json->appendChild("bird_pos", birdPos);

    json->appendValue("bird_facing_right", (_bird.isFacingRight()));

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
            projTypeStr = "DIRT" ;
        }
        projJson->appendValue("type", projTypeStr);
        
        projArray->appendChild(projJson);
    }
    json->appendChild("projectiles", projArray);

    return json;
}

/**
* Updates a neighboring or own board given the JSON value representing its game state
* 
* * Example board state:
 * {
    "player_id":  "1",
    "player_char": "Frog",
    "has_won": "false",
    "num_dirt": "1",
    "curr_board": "0",
    "player_x": "3.0",
    "player_y": "4.0",
    "health": "3",
    "stun_frames": "0",
    "wipe_frames": "0",
    "shoo_frames": "0",
    "timer": "145",
    "has_bird": True,
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
                "vel": ["0", "-2"],
                "dest": ["12.23", "23.5"],
                "type": "POOP"
            }
        ]
 * }
*
* @params data     The data to update
*/
void GameplayController::updateBoard(std::shared_ptr<JsonValue> data) {
    int playerId = std::stod(data->getString("player_id", "0"));
    std::string playerChar = data->getString("player_char");
    std::string playerHasWon = data->getString("has_won", "false");
    if (playerHasWon == "true" && !isGameOver()) {
        setGameOver(true);
        setWin(playerId == _id);
        return;
    }

    std::shared_ptr<Player> player;
    WindowGrid* windows;
    ProjectileSet* projectiles;
    // CULog("playerId: %d", playerId);
    
    if (playerId == _id) {
        // update own board info
        player = _player;
        windows = &_windows;
        projectiles = &_projectiles;
        _currentDirtAmount = std::stod(data->getString("num_dirt", "0"));
        _curBoard = std::stod(data->getString("curr_board", "0"));
        if (data->getBool("has_bird", false)) {
            _curBirdBoard = 0;
        }
    }
    else if (playerId == _id + 1 || (_id == 4 && playerId == 1)) {
        // if assigning ids clockwise, this is the right neighbor
        player = _playerRight;
        windows = &_windowsRight;
        projectiles = &_projectilesRight;
        _curBoardRight = std::stod(data->getString("curr_board", "0"));
        if (data->getBool("has_bird", false)) {
            _curBirdBoard = 1;
        }
    }
    else if (playerId == _id - 1 || (_id == 1 && playerId == 4)) {
        // if assigning ids clockwise, this is the left neighbor
        player = _playerLeft;
        windows = &_windowsLeft;
        projectiles = &_projectilesLeft;
        _curBoardLeft = std::stod(data->getString("curr_board", "0"));
        if (data->getBool("has_bird", false)) {
            _curBirdBoard = -1;
        }
    }
    else {
        // update the bird state to signify it is not on self or neighbor's boards
        if (data->getBool("has_bird", false)) {
            _curBirdBoard = 2;
        }
        // otherwise, this is the opposite side board and we do not need to track their board state.
        return;
    }

    // update game states
    // update bird position, no matter which board the bird is on
    const std::vector<std::shared_ptr<JsonValue>>& birdPos = data->get("bird_pos")->children();
    Vec2 birdBoardPos(std::stod(birdPos[0]->asString()), std::stod(birdPos[1]->asString()));
    _curBirdPos = getWorldPosition(birdBoardPos);

    _bird.setFacingRight(data->getBool("bird_facing_right"));

    // update the player's character textures if they are not already set
    if (player->getChar() != playerChar) {
        player->setChar(playerChar);
        changeCharTexture(player, playerChar);
    }

    // get x, y positions of player
    Vec2 playerBoardPos(std::stod(data->getString("player_x", "0")), std::stod(data->getString("player_y", "0")));
    player->setPosition(getWorldPosition(playerBoardPos));
    player->setHealth(std::stod(data->getString("health", "3")));

    // populate player's board with dirt
    windows->clearBoard();
    for (const std::shared_ptr< JsonValue>& jsonDirt : data->get("dirts")->children()) {
        std::vector<std::string> dirtPos = jsonDirt->asStringArray();
        windows->addDirt(std::stod(dirtPos[0]), std::stod(dirtPos[1]));
    }

    player->setStunFrames(std::stoi(data->getString("stun_frames")));
    player->setWipeFrames(std::stoi(data->getString("wipe_frames")));
    player->setShooFrames(std::stoi(data->getString("shoo_frames")));
//    _gameTime = data->getInt("timer");
        
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
    std::shared_ptr<Player> player = _player;
    WindowGrid* windows;
        
    // playerId can't be 1, since host does not send action message over network to itself
    if (playerId == 2) {
        player = _playerRight;
        windows = &_windowsRight;
    }
    else if (playerId == 3) {
        player = _playerAcross;
        windows = &_windowsAcross;
    }
    else if (playerId == 4) {
        player = _playerLeft;
        windows = &_windowsLeft;
    }

    // Check if player is stunned for this frame

    if (player->getStunFrames() == 0 && 
        player->getWipeFrames() == player->getMaxWipeFrames() &&
        player->getShooFrames() == player->getMaxShooFrames()) {
        // Move the player, ignoring collisions
        int moveResult = player->move(moveVec, getSize(), windows);
        if (moveResult == -1 || moveResult == 1) {
            // Request to switch to neighbor's board
            int destinationId = playerId + moveResult;
            if (destinationId == 0) {
                destinationId = 4;
            }
            else if (destinationId > 4) {
                destinationId = 1;
            }

            if (destinationId <= _numPlayers) {
                _allCurBoards[playerId - 1] = moveResult;
            }
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
        int edge = _player->getEdge(_windows.sideGap, getSize());
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

    const std::vector<std::shared_ptr<JsonValue>>& pos = data->get("dirt_pos")->children();
    Vec2 dirt_pos(std::stod(pos[0]->asString()), std::stod(pos[1]->asString()));

    const std::vector<std::shared_ptr<JsonValue>>& vel = data->get("dirt_vel")->children();
    Vec2 dirt_vel(std::stod(vel[0]->asString()), std::stod(vel[1]->asString()));

    const std::vector<std::shared_ptr<JsonValue>>& dest = data->get("dirt_dest")->children();
    Vec2 dirt_dest(std::stod(dest[0]->asString()), std::stod(dest[1]->asString()));

    const int amount = std::stoi(data->getString("dirt_amount", "1"));

    _allDirtAmounts[source_id - 1] = max(0, _allDirtAmounts[source_id - 1] - amount);
    _currentDirtAmount = _allDirtAmounts[0];

    if (target_id == 1) {
        _projectiles.spawnProjectile(getWorldPosition(dirt_pos), dirt_vel, getWorldPosition(dirt_dest), ProjectileSet::Projectile::ProjectileType::DIRT, amount);
    }
    if (target_id == 2) {
        _projectilesRight.spawnProjectile(getWorldPosition(dirt_pos), dirt_vel, getWorldPosition(dirt_dest), ProjectileSet::Projectile::ProjectileType::DIRT, amount);
    }
    if (target_id == 3) {
        _projectilesAcross.spawnProjectile(getWorldPosition(dirt_pos), dirt_vel, getWorldPosition(dirt_dest), ProjectileSet::Projectile::ProjectileType::DIRT, amount);
    }
    if (target_id == 4) {
        _projectilesLeft.spawnProjectile(getWorldPosition(dirt_pos), dirt_vel, getWorldPosition(dirt_dest), ProjectileSet::Projectile::ProjectileType::DIRT, amount);
    }
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
void GameplayController::update(float timestep, Vec2 worldPos, DirtThrowInputController& dirtCon, std::shared_ptr<cugl::scene2::Button> dirtThrowButton) {
    
    // update the audio controller
    _audioController.update(isActive());
    
    // get or transmit board states over network
    if (_network.getConnection()) {
        _network.getConnection()->receive([this](const std::string source,
            const std::vector<std::byte>& data) {
                if (!_ishost) {
                    std::shared_ptr<JsonValue> incomingMsg = _network.processMessage(source, data);
                    if (incomingMsg->has("dirts")) {
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
        // update bird if active
        if (_birdActive) {
            _bird.move();
            WindowGrid* windows;
            ProjectileSet* projectiles;
            if (_boardWithBird == 1) {
                windows = &_windows;
                projectiles = &_projectiles;
            }
            else if (_boardWithBird == 2) {
                windows = &_windowsRight;
                projectiles = &_projectilesRight;
            }
            else if (_boardWithBird == 3) {
                windows = &_windowsAcross;
                projectiles = &_projectilesAcross;
            }
            else {
                windows = &_windowsLeft;
                projectiles = &_projectilesLeft;
            }

            if (!_birdLeaving && _bird.atColCenter(windows->getNHorizontal(), windows->getPaneWidth(), windows->sideGap) >= 0) {
                std::bernoulli_distribution dist(_projectileGenChance);
                if (dist(_rng)) {
                    // random chance to generate bird poo at column center
                    generatePoo(projectiles);
                }
            }
            
        }

        stepForward(_player, _windows, _projectiles);
        if (_numPlayers > 1) {
            stepForward(_playerRight, _windowsRight, _projectilesRight);
        }
        if (_numPlayers > 2) {
            stepForward(_playerAcross, _windowsAcross, _projectilesAcross);
        }
        if (_numPlayers > 3) {
            stepForward(_playerLeft, _windowsLeft, _projectilesLeft);
        }
        for (int i = 1; i <= _numPlayers; i++) {
            _network.transmitMessage(getJsonBoard(i));
            // CULog("transmitting board state for player %d", i);
        }
        
        _input.update();
        if (_input.didPressReset()) {
            // host resets game for all players
            hostReset();
        }
        // update the game state for self (host). Updates for the rest of the players are done in processMovementRequest(),
        // called whenever the host recieves a movement or other action message.
        _currentDirtAmount = _allDirtAmounts[0];
        _curBoard = _allCurBoards[0];
        _curBoardLeft = _allCurBoards[_numPlayers - 1];
        _curBoardRight = _allCurBoards[1];
        _curBirdBoard = _boardWithBird == 4 ? -1 : _boardWithBird - 1;
        _curBirdPos = getWorldPosition(_bird.birdPosition);

    }
    else {
        // not host - advance all players idle or wipe or shoo frames
        if (_player->getWipeFrames() < _player->getMaxWipeFrames()) {
            _player->advanceWipeFrame();
        }
        if (_player->getShooFrames() < _player->getMaxShooFrames()) {
            _player->advanceShooFrame();
        }
        _player->advanceIdleFrame();

        if (_playerLeft->getWipeFrames() < _playerLeft->getMaxWipeFrames()) {
            _playerLeft->advanceWipeFrame();
        }
        if (_playerLeft->getShooFrames() < _playerLeft->getMaxShooFrames()) {
            _playerLeft->advanceShooFrame();
        }
        _playerLeft->advanceIdleFrame();

        if (_playerRight->getWipeFrames() < _playerRight->getMaxWipeFrames()) {
            _playerRight->advanceWipeFrame();
        }
        if (_playerRight->getShooFrames() < _playerRight->getMaxShooFrames()) {
            _playerRight->advanceShooFrame();
        }
        _playerRight->advanceIdleFrame();

        // uncomment later if we are adding the ability to see opposite player's board
        // if (_playerAcross->getWipeFrames() < _playerAcross->getMaxWipeFrames()) {
        //     _playerAcross->advanceWipeFrame();
        // }
        // _playerAcross->advanceIdleFrame();
    }

    // When the player is on other's board and are able to throw dirt
    if (_curBoard != 0 && _currentDirtAmount > 0) {
        // _dirtThrowInput.update();
        float player_x = _curBoard == -1 ? getSize().width - _windows.sideGap : _windows.sideGap;
        float button_x = _curBoard == -1 ? getSize().width - _windows.sideGap + 150 : _windows.sideGap - 150;
        cugl::Vec2 playerPos(player_x, _player->getPosition().y);
        cugl::Vec2 buttonPos(button_x, Application::get()->getDisplaySize().height / 2);
        dirtThrowButton->setPosition(buttonPos);
        if (!_dirtSelected) {
            if (dirtCon.didPress() && dirtThrowButton->isDown()) {
                _dirtSelected = true;
                _prevInputPos = worldPos;
            }
        } else {
            if (dirtCon.didRelease()) {
                _dirtSelected = false;
                Vec2 diff = worldPos - _prevInputPos;
                Vec2 destination = playerPos - diff * 5;
                Vec2 snapped_dest = getBoardPosition(destination);
                snapped_dest.x = clamp(round(snapped_dest.x), 0.0f, (float)_windows.getNHorizontal()) + 0.5;
                snapped_dest.y = clamp(round(snapped_dest.y), 0.0f, (float)_windows.getNVertical()) + 0.5;
                snapped_dest = getWorldPosition(snapped_dest);
                Vec2 velocity = (snapped_dest - playerPos).getNormalization() * 5;

                int targetId = _id + _curBoard;
                if (targetId == 0) {
                    targetId = 4;
                }
                if (targetId == 5) {
                    targetId = 1;
                }
                if (_ishost) {
                    processDirtThrowRequest(getJsonDirtThrow(targetId, playerPos, velocity, snapped_dest, _currentDirtAmount));
                }
                else {
                    _network.transmitMessage(getJsonDirtThrow(targetId, playerPos, velocity, snapped_dest, _currentDirtAmount));
                }
                dirtThrowButton->setPosition(buttonPos);

            } else if (dirtCon.isDown()) {
                std::vector<Vec2> vertices = { playerPos };
                Vec2 diff = worldPos - _prevInputPos;
                Vec2 destination = playerPos - diff * 5;
                dirtThrowButton->setPosition(buttonPos + diff);
                Vec2 snapped_dest = getBoardPosition(destination);
                snapped_dest.x = clamp(round(snapped_dest.x), 0.0f, (float)_windows.getNHorizontal()) + 0.5;
                snapped_dest.y = clamp(round(snapped_dest.y), 0.0f, (float)_windows.getNVertical()) + 0.5;
                vertices.push_back(getWorldPosition(snapped_dest));
                SimpleExtruder se;
                se.set(Path2(vertices));
                se.calculate(10);
                _dirtPath = se.getPolygon();
            }
        }
    }
    // When a player is on their own board
    else if (_curBoard == 0) {
        if (!_ishost) {
            // Read the keyboard for each controller.
            _input.update();
            // pass movement over network for host to process
            if (_network.getConnection()) {
                _network.checkConnection();
                if (_input.getDir().length() > 0 && !_ishost) {
                    // CULog("transmitting movement message over network for player %d", _id);
                    std::shared_ptr<JsonValue> m = getJsonMove(_input.getDir());
                    std::string s = m->toString();
                    _network.transmitMessage(m);
                }
                // send over scene switch requests are handled by button listener
            }
        }
        if (_ishost) {
            // Check if player is stunned for this frame
            if (_player->getStunFrames() == 0 && 
                _player->getWipeFrames() == _player->getMaxWipeFrames() &&
                _player->getShooFrames() == _player->getMaxShooFrames()) {
                // Move the player, ignoring collisions
                int moveResult = _player->move(_input.getDir(), getSize(), &_windows);
                if (moveResult == -1 && _numPlayers == 4) {
                    _allCurBoards[0] = -1;
                } else if (moveResult == 1 && _numPlayers >= 2) {
                    _allCurBoards[0] = 1;
                }
            }
        }
    }

    // advance bird flying frame
    _bird.advanceBirdFrame();

    // update time
    if ((_gameTimeLeft>=1)) {
        _frame = _frame+1;
    } if (_frame==_fps) {
        _gameTimeLeft=max(0, _gameTimeLeft-1);
        _projectileGenChance = 0.95 / (1 + exp(-0.05 * (100 - _gameTimeLeft/2)));
        _frame = 0;
    }
    
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
void GameplayController::stepForward(std::shared_ptr<Player>& player, WindowGrid& windows, ProjectileSet& projectiles) {
    int player_id = player->getId();

    if (checkBoardEmpty(windows) && !isGameOver()) {
        setGameOver(true);
        setWin(player_id == _id); // sets the host's local _gameWin property
    }

    std::vector<std::pair<cugl::Vec2, int>> landedDirts;

    if (_allCurBoards[player_id - 1] == 0) {
        // only check if player is stunned, has removed dirt, or collided with projectile
        // if they are on their own board.
        if (player->getStunFrames() > 0) {
            player->decreaseStunFrames();
        } 
        else {
            player->move();
        }
        
        if (player->getWipeFrames() < player->getMaxWipeFrames()) {
            player->advanceWipeFrame();
        }
        if (player->getShooFrames() < player->getMaxShooFrames()) {
            player->advanceShooFrame();
        }
        else {
            player->move();
        }
        
        player->advanceIdleFrame();
        
        
        // remove any dirt the player collides with
        Vec2 grid_coors = player->getCoorsFromPos(windows.getPaneHeight(), windows.getPaneWidth(), windows.sideGap);
        player->setCoors(grid_coors);

        int clamped_y = std::clamp(static_cast<int>(grid_coors.y), 0, windows.getNVertical() - 1);
        int clamped_x = std::clamp(static_cast<int>(grid_coors.x), 0, windows.getNHorizontal() - 1);
        bool dirtRemoved = windows.removeDirt(clamped_y, clamped_x);
        if (dirtRemoved) {
            // filling up dirty bucket
            // set amount of frames plaer is frozen for for cleaning dirt
            player->resetWipeFrames();
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
            if (collision_result.second.has_value()) {
                landedDirts.push_back(collision_result.second.value());
            }
        }
        
        if (!_birdLeaving && _boardWithBird == player_id && _collisions.resolveBirdCollision(player, _bird, getWorldPosition(_bird.birdPosition), 4)) {
            // set amount of frames plaer is frozen for for shooing bird
            player->resetShooFrames();
            _birdLeaving = true;
        }
        
        if (_birdLeaving && player->getShooFrames() == player->getMaxShooFrames()) {
            // send bird away after shooing
            _bird.resetBirdPathToExit(_windows.getNHorizontal());
        }
        
        if (_birdLeaving && _bird.birdReachesExit()) {
            _boardWithBird = _bird.isFacingRight() ? _boardWithBird + 1 : _boardWithBird - 1;
            if (_boardWithBird == 0) {
                _boardWithBird = _numPlayers;
            }
            if (_boardWithBird > _numPlayers) {
                _boardWithBird = 1;
            }
            
            std::uniform_int_distribution<> distr(0, _windows.getNVertical() - 1);
            int spawnRow = distr(_rng);
//            CULog("bird spawn at row %d", spawnRow);
            _bird.resetBirdPath(_windows.getNVertical(), _windows.getNHorizontal(), spawnRow);
            _birdLeaving = false;
        }
    }

    // Move the projectiles, get the center destination and amount of landed dirts
    auto landedProjs = projectiles.update(getSize());;
    landedDirts.insert(landedDirts.end(), landedProjs.begin(), landedProjs.end());
    // Add any landed dirts
    for (auto landedDirt : landedDirts) {
        cugl::Vec2 center = landedDirt.first;
        int x_coor = (int)((center.x - windows.sideGap) / windows.getPaneWidth());
        int y_coor = (int)(center.y / windows.getPaneHeight());
        x_coor = std::clamp(x_coor, 0, windows.getNHorizontal() - 1);
        y_coor = std::clamp(y_coor, 0, windows.getNVertical() - 1);

        int amount = landedDirt.second;
        std::vector<cugl::Vec2> landedCoords = calculateLandedDirtPositions(windows.getNVertical(), windows.getNHorizontal(), Vec2(x_coor, y_coor), amount);
        for (cugl::Vec2 dirtPos : landedCoords) {
            windows.addDirt(dirtPos.y, dirtPos.x);
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
    std::uniform_int_distribution<int> rowDist(0, _windows.getNVertical() - 1);
    std::uniform_int_distribution<int> colDist(0, _windows.getNHorizontal() - 1);
    int rand_row = rowDist(_rng);
    int rand_col = colDist(_rng);
//    CULog("player at: (%f, %f)", _player->getCoors().y, _player->getCoors().x);
//    CULog("generate at: (%d, %d)", (int)rand_row, (int) rand_col);

    // if add dirt already exists at location or player at location and board is not full, repeat
    while (Vec2((int)_player->getCoors().y, (int)_player->getCoors().x) == Vec2((int)rand_row, (int)rand_col) || !_windows.addDirt(rand_row, rand_col)) {
        rand_row = rowDist(_rng);
        rand_col = colDist(_rng);
    }
}

/** handles poo generation */
void GameplayController::generatePoo(ProjectileSet* projectiles) {
//    CULog("player at: (%f, %f)", _player->getCoors().y, _player->getCoors().x);
//    CULog("generate at: %d", (int)rand_row);

// if add dirt already exists at location or player at location and board is not full, repeat
    cugl::Vec2 birdWorldPos = getWorldPosition(_bird.birdPosition);
    // randomize the destination of bird poo, any window pane below the current bird position
    std::uniform_int_distribution<int> rowDist(0, floor(_bird.birdPosition.y));
    int rand_row_center = rowDist(_rng);
    cugl::Vec2 birdPooDest = getWorldPosition(Vec2(_bird.birdPosition.x, rand_row_center));
    projectiles->spawnProjectile(Vec2(birdWorldPos.x, birdWorldPos.y - _windows.getPaneHeight()/2), Vec2(0, min(-2.4f,-2-_projectileGenChance)), birdPooDest, ProjectileSet::Projectile::ProjectileType::POOP);
}

/** Checks whether board is full except player current location*/
const bool GameplayController::checkBoardFull() {
    for (int x = 0; x < _windows.getNHorizontal(); x++) {
        for (int y = 0; y < _windows.getNVertical(); y++) {
                if (_windows.getWindowState(y, x) == 0) {
                    if (Vec2((int)_player->getCoors().y, (int)_player->getCoors().x) == Vec2(y, x)) {
                        // consider current place occupied
                        continue;
                    }
                    return false;
                }
        }
    }
    return true; // No 0s found, all dirty spots
}

/** Checks whether board is empty except player current location*/
const bool GameplayController::checkBoardEmpty(WindowGrid playerWindowGrid) {
    for (int x = 0; x < playerWindowGrid.getNHorizontal(); x++) {
        for (int y = 0; y < playerWindowGrid.getNVertical(); y++) {
                if (playerWindowGrid.getWindowState(y, x)) {
                    return false;
                }
        }
    }
    return true; // No dirt found, board is clear
}

/** Returns number of dirts on the player's board **/
float GameplayController::returnNumBoardDirts(WindowGrid playerWindowGrid) {
    float count=0;
    for (int x = 0; x < playerWindowGrid.getNHorizontal(); x++) {
        for (int y = 0; y < playerWindowGrid.getNVertical(); y++) {
                if (playerWindowGrid.getWindowState(y, x) == 1) {
                    count=count+1;
                }
        }
    }
    return count; // No 1s found, board is clear
}

/** Returns number of max amount of dirt player's board could hold **/
float GameplayController::returnBoardMaxDirts(WindowGrid playerWindowGrid) {
    return playerWindowGrid.getNVertical()*playerWindowGrid.getNHorizontal();
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
    if (_curBoard == 0) {
        _windows.draw(batch, getSize());
        _player->draw(batch, getSize());
        if (_curBoardLeft == 1) {
            // left neighbor is on this player's board
            _playerLeft->drawPeeking(batch, getSize(), _curBoardLeft, _windows.sideGap);
        }
        if (_curBoardRight == -1) {
            // right neighbor is on this player's board
            _playerRight->drawPeeking(batch, getSize(), _curBoardRight, _windows.sideGap);
        }
        _projectiles.draw(batch, getSize(), _windows.getPaneWidth(), _windows.getPaneHeight());
        if (_curBirdBoard == 0) {
            _bird.draw(batch, getSize(), _curBirdPos);
        }
    }
    else if (_curBoard == -1) {
        _windowsLeft.draw(batch, getSize());
        if (_curBoardLeft == 0) {
            _playerLeft->draw(batch, getSize());
        }
        _player->drawPeeking(batch, getSize(), _curBoard, _windows.sideGap);
        _projectilesLeft.draw(batch, getSize(), _windowsLeft.getPaneWidth(), _windowsLeft.getPaneHeight());

        vector<Vec2> potentialDirts;
        if (_dirtSelected && _dirtPath.size() != 0) {
            batch->setColor(Color4::BLACK);
            batch->fill(_dirtPath);
            Vec2 dirtDest = _dirtPath.getVertices().back() - Vec2(0.5, 0.5);
            Vec2 landedDirtCoords = getBoardPosition(dirtDest);
            landedDirtCoords.y = std::clamp(static_cast<int>(landedDirtCoords.y), 0, _windowsRight.getNVertical() - 1);
            landedDirtCoords.x = std::clamp(static_cast<int>(landedDirtCoords.x), 0, _windowsRight.getNHorizontal() - 1);
            potentialDirts = calculateLandedDirtPositions(_windowsLeft.getNVertical(), _windowsLeft.getNHorizontal(), landedDirtCoords, _currentDirtAmount);
        }
        if (potentialDirts.size() > 0) {
            _windowsLeft.drawPotentialDirt(batch, getSize(), potentialDirts);
        }

        if (_curBirdBoard == -1) {
            _bird.draw(batch, getSize(), _curBirdPos);
        }
    }
    else if (_curBoard == 1) {
        _windowsRight.draw(batch, getSize());
        if (_curBoardRight == 0) {
            _playerRight->draw(batch, getSize());
        }
        _player->drawPeeking(batch, getSize(), _curBoard, _windows.sideGap);
        _projectilesRight.draw(batch, getSize(), _windowsRight.getPaneWidth(), _windowsRight.getPaneHeight());

        vector<Vec2> potentialDirts;
        if (_dirtSelected && _dirtPath.size() != 0) {
            batch->setColor(Color4::BLACK);
            batch->fill(_dirtPath);
            Vec2 dirtDest = _dirtPath.getVertices().back();
            Vec2 landedDirtCoords = getBoardPosition(dirtDest);
            landedDirtCoords.y = std::clamp(static_cast<int>(landedDirtCoords.y), 0, _windowsRight.getNVertical() - 1);
            landedDirtCoords.x = std::clamp(static_cast<int>(landedDirtCoords.x), 0, _windowsRight.getNHorizontal() - 1);
            potentialDirts = calculateLandedDirtPositions(_windowsRight.getNVertical(), _windowsRight.getNHorizontal(), landedDirtCoords, _currentDirtAmount);
        }
        if (potentialDirts.size() > 0) {
            _windowsRight.drawPotentialDirt(batch, getSize(), potentialDirts);
        }
        if (_curBirdBoard == 1) {
            _bird.draw(batch, getSize(), _curBirdPos);
        }
    }
    

}


void GameplayController::setActive(bool f) {
    // yes this code is bad and needs to be reworked
    if (!f) {
        _audioController.update(false);
        _isActive=false;
        setRequestForMenu(false);
        setGameOver(false);
        setWin(false);
    } else {
        _isActive = true;
        setRequestForMenu(false);
        setGameOver(false);
        setWin(false);
        _frameCountForWin = 0;
    };
    _gameTimeLeft = _gameTime;
}
