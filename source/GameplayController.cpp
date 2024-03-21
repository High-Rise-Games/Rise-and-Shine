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
#include "GameAudioController.h"

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
    
    _frame=0;
    
    // Initialize a game audio controller
    _audioController.init(assets);
    
    

    // fps as established per App
    _fps = fps;
    
    Size dimen = Application::get()->getDisplaySize();
    _rng.seed(std::time(nullptr));
    _projectileGenChance = 0.4;
    _projectileGenCountDown = 120;
    _dirtGenSpeed = 2;
    _dirtThrowTimer = 0;
    _fixedDirtUpdateThreshold = 5 * 60;
    _maxDirtAmount = 1;
    _currentDirtAmount = 0;
    _size = size;
    
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

    // Initialize all starting current boards
    _curBoard = 0;
    _curBoardRight = 0;
    _curBoardLeft = 0;
    
    // Initialize the window grids
    _windows.setTexture(assets->get<Texture>("window")); // MUST SET TEXTURE FIRST
    _windows.init(_constants->get("easy board"), size); // init depends on texture
    _windows.setDirtTexture(assets->get<Texture>("dirt"));

    _windowsLeft.setTexture(assets->get<Texture>("window")); // MUST SET TEXTURE FIRST
    _windowsLeft.init(_constants->get("easy board"), size); // init depends on texture
    _windowsLeft.setDirtTexture(assets->get<Texture>("dirt"));

    _windowsRight.setTexture(assets->get<Texture>("window")); // MUST SET TEXTURE FIRST
    _windowsRight.init(_constants->get("easy board"), size); // init depends on texture
    _windowsRight.setDirtTexture(assets->get<Texture>("dirt"));

    // Initialize projectiles
    _projectiles.setDirtTexture(assets->get<Texture>("dirt"));
    _projectiles.setPoopTexture(assets->get<Texture>("poop"));
    _projectiles.setTextureScales(_windows.getPaneHeight(), _windows.getPaneWidth());
//    _projectiles.init(_constants->get("projectiles"));

    _projectilesLeft.setDirtTexture(assets->get<Texture>("dirt"));
    _projectilesLeft.setPoopTexture(assets->get<Texture>("poop"));
    _projectilesLeft.setTextureScales(_windowsLeft.getPaneHeight(), _windowsLeft.getPaneWidth());

    _projectilesRight.setDirtTexture(assets->get<Texture>("dirt"));
    _projectilesRight.setPoopTexture(assets->get<Texture>("poop"));
    _projectilesRight.setTextureScales(_windowsRight.getPaneHeight(), _windowsRight.getPaneWidth());

    // Initialize bird textures, but do not set a location yet. that is the host's job
    if (_birdActive) {
        _bird.setTexture(assets->get<Texture>("bird"));
        _bird.init(cugl::Vec2(_windows.sideGap, (_windows.getNVertical() - 1) * _windows.getPaneHeight()),
            cugl::Vec2(size.getIWidth() - _windows.sideGap, (_windows.getNVertical() - 1) * _windows.getPaneHeight()), 2, 0.05);
        _curBirdBoard = 2;
    }
    
    // Make a ship and set its texture
    // starting position is most bottom left window
    Vec2 startingPos = Vec2(_windows.sideGap + (_windows.getPaneWidth() / 2), _windows.getPaneHeight());

    // no ids given yet - to be assigned in initPlayers()
    _player = std::make_shared<Player>(-1, startingPos, _constants->get("ship"), _windows.getPaneHeight(), _windows.getPaneWidth());
    _player->setTexture(assets->get<Texture>("player_1"));
    
    _playerLeft = std::make_shared<Player>(-1, startingPos, _constants->get("ship"), _windows.getPaneHeight(), _windows.getPaneWidth());
    _playerLeft->setTexture(assets->get<Texture>("player_1"));

    _playerRight = std::make_shared <Player>(-1, startingPos, _constants->get("ship"), _windows.getPaneHeight(), _windows.getPaneWidth());
    _playerRight->setTexture(assets->get<Texture>("player_1"));

    // Initialize random dirt generation
    updateDirtGenTime();

    _collisions.init(size);

    // Get the bang sound
    _bang = assets->get<Sound>("bang");
    
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

    _windowsAcross.setTexture(assets->get<Texture>("window")); // MUST SET TEXTURE FIRST
    _windowsAcross.init(_constants->get("easy board"), getSize()); // init depends on texture
    _windowsAcross.setDirtTexture(assets->get<Texture>("dirt"));

    _projectilesAcross.setDirtTexture(assets->get<Texture>("dirt"));
    _projectilesAcross.setPoopTexture(assets->get<Texture>("poop"));
    _projectilesAcross.setTextureScales(_windows.getPaneHeight(), _windows.getPaneWidth());

    // starting position is most bottom left window
    Vec2 startingPos = Vec2(_windows.sideGap + (_windows.getPaneWidth() / 2), _windows.getPaneHeight());

    // player ids for self, right, and left already assigned from earlier initPlayers call
    _playerAcross = std::make_shared<Player>(3, startingPos, _constants->get("ship"), _windows.getPaneHeight(), _windows.getPaneWidth());
    _playerAcross->setTexture(assets->get<Texture>("player_1"));

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
    _projectileGenChance = 0.4;
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
 * Method for the scene switch listener used in GameScene
 */
void GameplayController::switchScene() {
    int edge = _player->getEdge(_windows.sideGap, getSize());
    if (_curBoard != 0) {
        CULog("leaving other player's board");
        if (_ishost) {
            _allCurBoards[0] = 0;
            _curBoard = 0;
        }
        else {
            _network.transmitMessage(getJsonSceneSwitch(true));
        }
    }
    else if (edge != 0) {
        CULog("entering other player's board: %d", edge);
        if (_ishost) {
            // if move is valid
            int destinationId = _id + edge;
            if (destinationId == 0) {
                destinationId = 4;
            }
            else if (destinationId == 5) {
                destinationId == 1;
            }
            if (destinationId <= _numPlayers) {
                _allCurBoards[0] = edge;
                _curBoard = edge;
            }
        }
        else {
            // first check that we are on an edge
            // to make a valid scene switch request
            if (_player->getEdge(_windows.sideGap, getSize()) != 0) {
                _network.transmitMessage(getJsonSceneSwitch(false));
            }
        }
    }
}

/** 
 * Converts game state into a JSON value for sending over the network.
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
    json->appendValue("player_id", static_cast<double>(id));
    json->appendValue("num_dirt", static_cast<double>(_allDirtAmounts[id - 1]));
    json->appendValue("curr_board", static_cast<double>(_allCurBoards[id - 1]));

    cugl::Vec2 playerBoardPos = getBoardPosition(player->getPosition());
    json->appendValue("player_x", playerBoardPos.x);
    json->appendValue("player_y", playerBoardPos.y);

    json->appendValue("health", static_cast<double>(player->getHealth()));
    json->appendValue("stun_frames", static_cast<double>(player->getStunFrames()));
    json->appendValue("timer", static_cast<double>(_gameTime));
    json->appendValue("has_bird", id == _boardWithBird);

    cugl::Vec2 birdBoardPos = getBoardPosition(_bird.birdPosition);
    const std::shared_ptr<JsonValue> birdPos = std::make_shared<JsonValue>();
    birdPos->init(JsonValue::Type::ArrayType);
    birdPos->appendValue(static_cast<double>(birdBoardPos.x));
    birdPos->appendValue(static_cast<double>(birdBoardPos.y));
    json->appendChild("bird_pos", birdPos);

    const std::shared_ptr<JsonValue> dirtArray = std::make_shared<JsonValue>();
    dirtArray->init(JsonValue::Type::ArrayType);
    for (int col = 0; col < windows->getNHorizontal(); ++col) {
        for (int row = 0; row < windows->getNVertical(); ++row) {
            bool hasDirt = windows->getWindowState(row, col);
            if (hasDirt) {
                const std::shared_ptr<JsonValue> dirtPos = std::make_shared<JsonValue>();
                dirtPos->init(JsonValue::Type::ArrayType);
                dirtPos->appendValue(static_cast<double>(row));
                dirtPos->appendValue(static_cast<double>(col));
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
        projPos->appendValue(projBoardPos.x);
        projPos->appendValue(projBoardPos.y);
        projJson->appendChild("pos", projPos);

        const std::shared_ptr<JsonValue> projVel = std::make_shared<JsonValue>();
        projVel->init(JsonValue::Type::ArrayType);
        projVel->appendValue(proj->velocity.x);
        projVel->appendValue(proj->velocity.y);
        projJson->appendChild("vel", projVel);

        cugl::Vec2 projDestBoardPos = getBoardPosition(proj->destination);
        const std::shared_ptr<JsonValue> projDest = std::make_shared<JsonValue>();
        projDest->init(JsonValue::Type::ArrayType);
        projDest->appendValue(projDestBoardPos.x);
        projDest->appendValue(projDestBoardPos.y);
        projJson->appendChild("dest", projDest);

        if (proj->type == ProjectileSet::Projectile::ProjectileType::DIRT) {
            projJson->appendValue("type", "DIRT");
        }
        else if (proj->type == ProjectileSet::Projectile::ProjectileType::POOP) {
            projJson->appendValue("type", "POOP");
        }
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
    "player_id":  1,
    "num_dirt": 1,
    "curr_board": 0,
    "player_x": 3.0,
    "player_y": 4.0,
    "health": 3,
    "stun_frames": 0,
    "timer": 145,
    "has_bird": True,
    "bird_pos": [2.4, 6.0],
    "dirts": [ [0, 1], [2, 2], [0, 2] ],
    "projectiles": [ 
            { 
                "pos": [3.0, 1.45],
                "vel": [2, 3],
                "dest": [12.23, 23.5],
                "type: "DIRT"
            },
            {
                "pos": [5.0, 0.2],
                "vel": [0, -2], 
                "dest": [12.23, 23.5],
                "type": "POOP"
            }
        ]
 * }
*
* @params data     The data to update
*/
void GameplayController::updateBoard(std::shared_ptr<JsonValue> data) {
    int playerId = data->getInt("player_id", 0);

    std::shared_ptr<Player> player;
    WindowGrid* windows;
    ProjectileSet* projectiles;
    // CULog("playerId: %d", playerId);
    
    if (playerId == _id) {
        // update own board info
        player = _player;
        windows = &_windows;
        projectiles = &_projectiles;
        _currentDirtAmount = data->getInt("num_dirt", 0);
        _curBoard = data->getInt("curr_board", 0);
        if (data->getBool("has_bird", false)) {
            _curBirdBoard = 0;
        }
    }
    else if (playerId == _id + 1 || (_id == 4 && playerId == 1)) {
        // if assigning ids clockwise, this is the right neighbor
        player = _playerRight;
        windows = &_windowsRight;
        projectiles = &_projectilesRight;
        _curBoardRight = data->getInt("curr_board", 0);
        if (data->getBool("has_bird", false)) {
            _curBirdBoard = 1;
        }
    }
    else if (playerId == _id - 1 || (_id == 1 && playerId == 4)) {
        // if assigning ids clockwise, this is the left neighbor
        player = _playerLeft;
        windows = &_windowsLeft;
        projectiles = &_projectilesLeft;
        _curBoardLeft = data->getInt("curr_board", 0);
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
    Vec2 birdBoardPos(birdPos[0]->asFloat(), birdPos[1]->asFloat());
    _curBirdPos = getWorldPosition(birdBoardPos);

    // get x, y positions of player
    Vec2 playerBoardPos(data->getFloat("player_x", 0), data->getFloat("player_y", 0));
    player->setPosition(getWorldPosition(playerBoardPos));
    player->setHealth(data->getInt("health", 3));

    // populate player's board with dirt
    windows->clearBoard();
    for (const std::shared_ptr< JsonValue>& jsonDirt : data->get("dirts")->children()) {
        std::vector<int> dirtPos = jsonDirt->asIntArray();
        windows->addDirt(dirtPos[0], dirtPos[1]);
    }

    player->setStunFrames(data->getInt("stun_frames"));
    _gameTime = data->getInt("timer");
        
    // populate player's projectile set
    projectiles->clearCurrentSet(); // clear current set to rewrite
    for (const std::shared_ptr<JsonValue>& projNode : data->get("projectiles")->children()) {
        // get projectile position
        const std::vector<std::shared_ptr<JsonValue>>& projPos = projNode->get("pos")->children();
        Vec2 pos(projPos[0]->asFloat(), projPos[1]->asFloat());

        // get projectile velocity
        const std::vector<std::shared_ptr<JsonValue>>& projVel = projNode->get("vel")->children();
        Vec2 vel(projVel[0]->asInt(), projVel[1]->asInt());
        
        // get projectile destination
        const std::vector<std::shared_ptr<JsonValue>>& projDest = projNode->get("dest")->children();
        Vec2 dest(projDest[0]->asInt(), projDest[1]->asInt());

        // get projectile type
        string typeStr = projNode->get("type")->asString();
        auto type = ProjectileSet::Projectile::ProjectileType::POOP;
        if (typeStr == "DIRT") {
            type = ProjectileSet::Projectile::ProjectileType::DIRT;
            CULog("is dirt");
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
    json->appendValue("player_id", static_cast<double>(_id));

    const std::shared_ptr<JsonValue> vel = std::make_shared<JsonValue>();
    vel->init(JsonValue::Type::ArrayType);
    vel->appendValue(move.x);
    vel->appendValue(move.y);
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
    int playerId = data->getInt("player_id", 0);
    const std::vector<std::shared_ptr<JsonValue>>& vel = data->get("vel")->children();
    Vec2 moveVec(vel[0]->asFloat(), vel[1]->asFloat());
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
    if (player->getStunFrames() == 0) {
        // Move the player, ignoring collisions
        int moveResult = player->move(moveVec, getSize(), windows->sideGap);
        if (moveResult == -1 || moveResult == 1) {
            // Request to switch to neighbor's board
            int destinationId = playerId + moveResult;
            if (destinationId == 0) {
                destinationId = 4;
            }
            else if (destinationId == 5) {
                destinationId == 1;
            }

            if (destinationId <= _numPlayers) {
                _allCurBoards[playerId - 1] = moveResult;
            }
        }
    }
}



/**
* Called by the client only. Returns a JSON value representing a scene switch request
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
    json->appendValue("player_id", static_cast<double>(_id));

    if (returning) {
        json->appendValue("switch_destination", static_cast<double>(0));
    }
    else {
        // pre-condition: if not returning, guarantee that the player is on an edge
        int edge = _player->getEdge(_windows.sideGap, getSize());
        json->appendValue("switch_destination", static_cast<double>(edge));
    }
    return json;
}

/**
* Called by host only to process switch scene requests. Updates a client player's
* currently viewed board for the player at player_id based on the current board
* value stored in the JSON value.
*
* @params data     The data to update
*/
void GameplayController::processSceneSwitchRequest(std::shared_ptr<cugl::JsonValue> data) {

    int playerId = data->getInt("player_id", 0);
    int switchDestination = data->getInt("switch_destination", 0);
    CULog("player %d switching to %d", playerId, switchDestination);

    // update the board of the player to their switch destination
    if (switchDestination == 0) {
        _allCurBoards[playerId - 1] = switchDestination;
    }
    else {
        int destinationId = playerId + switchDestination;
        if (destinationId == 0) {
            destinationId = 4;
        }
        else if (destinationId == 5) {
            destinationId == 1;
        }

        if (destinationId <= _numPlayers) {
            _allCurBoards[playerId - 1] = switchDestination;
        }
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
    "dirt_dest": [30.2, 122.4]
* }
*
* @param target The id of the player whose board the current player is sending dirt to
* @param pos    The starting position of the dirt projectile
* @param vel    The velocity vector of the dirt projectile
* @param dest   The destination coordinates of the dirt projectile
*
* @returns JSON value representing a dirt throw action
*/
std::shared_ptr<cugl::JsonValue> GameplayController::getJsonDirtThrow(const int target, const cugl::Vec2 pos, const cugl::Vec2 vel, const cugl::Vec2 dest) {
    const std::shared_ptr<JsonValue> json = std::make_shared<JsonValue>();
    json->init(JsonValue::Type::ObjectType);
    json->appendValue("player_id_source", static_cast<double>(_id));
    json->appendValue("player_id_target", static_cast<double>(target));
    
    cugl::Vec2 boardPos = getBoardPosition(pos);
    const std::shared_ptr<JsonValue> dirtPos = std::make_shared<JsonValue>();
    dirtPos->init(JsonValue::Type::ArrayType);
    dirtPos->appendValue(static_cast<double>(boardPos.x));
    dirtPos->appendValue(static_cast<double>(boardPos.y));
    json->appendChild("dirt_pos", dirtPos);

    const std::shared_ptr<JsonValue> dirtVel = std::make_shared<JsonValue>();
    dirtVel->init(JsonValue::Type::ArrayType);
    dirtVel->appendValue(static_cast<double>(vel.x));
    dirtVel->appendValue(static_cast<double>(vel.y));
    json->appendChild("dirt_vel", dirtVel);

    cugl::Vec2 boardDest = getBoardPosition(dest);
    const std::shared_ptr<JsonValue> dirtDest = std::make_shared<JsonValue>();
    dirtDest->init(JsonValue::Type::ArrayType);
    dirtDest->appendValue(static_cast<double>(boardDest.x));
    dirtDest->appendValue(static_cast<double>(boardDest.y));
    json->appendChild("dirt_dest", dirtDest);

    CULog("dirt throw from player %d to %d", _id, target);

    return json;
}

/**
* Called by host only. Updates the boards of both the dirt thrower and the player
* receiving the dirt projectile given the information stored in the JSON value.
*
* @params data     The data to update
*/
void GameplayController::processDirtThrowRequest(std::shared_ptr<cugl::JsonValue> data) {
    int source_id = data->getInt("player_id_source", 0);
    int target_id = data->getInt("player_id_target", 0);

    const std::vector<std::shared_ptr<JsonValue>>& pos = data->get("dirt_pos")->children();
    Vec2 dirt_pos(pos[0]->asFloat(), pos[1]->asFloat());

    const std::vector<std::shared_ptr<JsonValue>>& vel = data->get("dirt_vel")->children();
    Vec2 dirt_vel(vel[0]->asFloat(), vel[1]->asFloat());

    const std::vector<std::shared_ptr<JsonValue>>& dest = data->get("dirt_dest")->children();
    Vec2 dirt_dest(dest[0]->asFloat(), dest[1]->asFloat());

    _allDirtAmounts[source_id - 1] = max(0, _allDirtAmounts[source_id - 1] - 1);
    _currentDirtAmount = _allDirtAmounts[0];

    if (target_id == 1) {
        _projectiles.spawnProjectile(getWorldPosition(dirt_pos), dirt_vel, getWorldPosition(dirt_dest), ProjectileSet::Projectile::ProjectileType::DIRT);
    }
    if (target_id == 2) {
        _projectilesRight.spawnProjectile(getWorldPosition(dirt_pos), dirt_vel, getWorldPosition(dirt_dest), ProjectileSet::Projectile::ProjectileType::DIRT);
    }
    if (target_id == 3) {
        _projectilesAcross.spawnProjectile(getWorldPosition(dirt_pos), dirt_vel, getWorldPosition(dirt_dest), ProjectileSet::Projectile::ProjectileType::DIRT);
    }
    if (target_id == 4) {
        _projectilesLeft.spawnProjectile(getWorldPosition(dirt_pos), dirt_vel, getWorldPosition(dirt_dest), ProjectileSet::Projectile::ProjectileType::DIRT);
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
 */
void GameplayController::update(float timestep, Vec2 worldPos, DirtThrowInputController& dirtCon) {
    
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
                        CULog("got switch scene request message");
                        processSceneSwitchRequest(incomingMsg);
                    }
                    else if (incomingMsg->has("player_id_target")) {
                        CULog("got dirt throw message");
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

            if (_bird.atColCenter(windows->getNHorizontal(), windows->getPaneWidth(), windows->sideGap) >= 0) {
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
        _curBirdBoard = _boardWithBird == 4 ? -1 : _boardWithBird - 1;
        _curBirdPos = _bird.birdPosition;
    }

    // When the player is on other's board and are able to throw dirt
    if (_curBoard != 0 && _currentDirtAmount > 0) {
        // _dirtThrowInput.update();
        if (!_dirtSelected) {
            if (dirtCon.didPress()) {
                float distance = (worldPos - _player->getPosition()).length();
                if (distance <= _player->getRadius()) {
                    _dirtSelected = true;
                    _prevDirtPos = _player->getPosition();
                }
            }
        } else {
            if (dirtCon.didRelease()) {
                _dirtSelected = false;
                Vec2 diff = worldPos - _prevDirtPos;
                Vec2 velocity = diff.getNormalization() * -5;
                Vec2 destination = worldPos - diff * 5;

                int targetId = _id + _curBoard;
                if (targetId == 0) {
                    targetId = 4;
                }
                if (targetId == 5) {
                    targetId = 1;
                }
                if (_ishost) {
                    processDirtThrowRequest(getJsonDirtThrow(targetId, _prevDirtPos, velocity, destination));
                }
                else {
                    _network.transmitMessage(getJsonDirtThrow(targetId, _prevDirtPos, velocity, destination));
                }
                _player->setPosition(_prevDirtPos);

            } else if (dirtCon.isDown()) {
                _player->setPosition(worldPos);
                std::vector<Vec2> vertices = { worldPos };
                Vec2 diff = worldPos - _prevDirtPos;
                Vec2 destination = worldPos - diff * 5;
                vertices.push_back(destination);
                _dirtPath = Path2(vertices);
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
            if (_player->getStunFrames() == 0) {
                // Move the player, ignoring collisions
                int moveResult = _player->move(_input.getDir(), getSize(), _windows.sideGap);
                if (moveResult == -1 && _numPlayers == 4) {
                    _allCurBoards[0] = -1;
                } else if (moveResult == 1 && _numPlayers >= 2) {
                    _allCurBoards[0] = 1;
                }
            }
        }
    }

    // update time
    if ((_gameTime>=1)) {
        _frame = _frame+1;
    } if (_frame==_fps && (_gameTime>=1)) {
        _gameTime=_gameTime-1;
        _projectileGenChance = 0.8 / (1 + exp(-0.05 * (_gameTime - 50)));
        _frame = 0;
    }
}

/**
* FOR HOST ONLY. This method does all the heavy lifting work for update.
* The host steps forward each player's game state, given references to the player, board, and projectile set.
*/
void GameplayController::stepForward(std::shared_ptr<Player>& player, WindowGrid& windows, ProjectileSet& projectiles) {
    int player_id = player->getId();

    if (_allCurBoards[player_id - 1] == 0) {
        // only check if player is stunned, has removed dirt, or collided with projectile
        // if they are on their own board.
        if (player->getStunFrames() > 0) {
            player->decreaseStunFrames();
        }
        else {
            player->move();
        }
        // remove any dirt the player collides with
        Vec2 grid_coors = player->getCoorsFromPos(windows.getPaneHeight(), windows.getPaneWidth(), windows.sideGap);
        player->setCoors(grid_coors);

        int clamped_y = std::clamp(static_cast<int>(grid_coors.y), 0, windows.getNVertical() - 1);
        int clamped_x = std::clamp(static_cast<int>(grid_coors.x), 0, windows.getNHorizontal() - 1);
        bool dirtRemoved = windows.removeDirt(clamped_y, clamped_x);
        if (dirtRemoved) {
            // filling up dirty bucket
            _allDirtAmounts[player_id - 1] = min(_maxDirtAmount, _allDirtAmounts[player_id - 1] + 1);
        }

        // Check for collisions and play sound
        if (player->getStunFrames() <= 0 && _collisions.resolveCollision(player, projectiles)) {
            AudioEngine::get()->play("bang", _bang, false, _bang->getVolume(), true);
        }
        
        if (_boardWithBird == player_id && _collisions.resolveBirdCollision(player, _bird, 0.01)) {
            CULog("collided with bird");
            _boardWithBird = _bird.isFacingRight() ? _boardWithBird + 1 : _boardWithBird - 1;
            if (_boardWithBird == 0) {
                _boardWithBird = _numPlayers;
            }
            if (_boardWithBird > _numPlayers) {
                _boardWithBird = 1;
            }
        }
    }
    
    // Generate falling hazard based on chance
//    if (_projectileGenCountDown == 0) {
//        std::bernoulli_distribution dist(_projectileGenChance);
//        if (dist(_rng)) {
//            // random projectile generation logic
//            // CULog("generating random poo");
//            generatePoo(projectiles);
//        }
//        _projectileGenCountDown = 120;
//    }
//    else {
//        _projectileGenCountDown -= 1;
//    }

    // Move the projectiles
    std::vector<cugl::Vec2> landedDirts = projectiles.update(getSize());
    // Add any landed dirts
    for (auto dirtPos : landedDirts) {
        int x_coor = ((int)(dirtPos.x - windows.sideGap) / windows.getPaneWidth());
        int y_coor = (int)(dirtPos.y / windows.getPaneHeight());
        x_coor = std::clamp(x_coor, 0, windows.getNHorizontal()-1);
        y_coor = std::clamp(y_coor, 0, windows.getNVertical()-1);
        windows.addDirt(y_coor, x_coor);
    }

    // fixed dirt generation logic (every 5 seconds)
    // if (!checkBoardEmpty() && !checkBoardFull()) {
    //     if (_dirtThrowTimer <= _fixedDirtUpdateThreshold) {
    //         auto search = _dirtGenTimes.find(_dirtThrowTimer);
    //         if (search != _dirtGenTimes.end()) {
    //             // random dirt generation logic
//                CULog("generating random dirt");
    //             generateDirt();
    //         }
    //         _dirtThrowTimer++;
    //     }
    //     else {
    //         _dirtThrowTimer = 0;
    //         updateDirtGenTime();
            //            CULog("generating fixed dirt");
    //         generateDirt();
    //     }
    // }
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

///** handles random poo generation */
//void GameplayController::generateRandomPoo(ProjectileSet& projectiles) {
//    std::uniform_real_distribution<float> rowDist(_windows.sideGap + 40, (float)getSize().getIWidth() - _windows.sideGap - 60);
//    float rand_row = rowDist(_rng);
////    CULog("player at: (%f, %f)", _player->getCoors().y, _player->getCoors().x);
////    CULog("generate at: %d", (int)rand_row);
//    // if add dirt already exists at location or player at location and board is not full, repeat
//    projectiles.spawnProjectile(Vec2(rand_row, getSize().height - 50), Vec2(0, min(-2.4f,-2-_projectileGenChance)), Vec2(rand_row, 0), ProjectileSet::Projectile::ProjectileType::POOP);
//}

/** handles poo generation */
void GameplayController::generatePoo(ProjectileSet* projectiles) {
//    CULog("player at: (%f, %f)", _player->getCoors().y, _player->getCoors().x);
//    CULog("generate at: %d", (int)rand_row);
    // if add dirt already exists at location or player at location and board is not full, repeat
    projectiles->spawnProjectile(_bird.birdPosition, Vec2(0, min(-2.4f,-2-_projectileGenChance)), Vec2(_bird.birdPosition.x, 0), ProjectileSet::Projectile::ProjectileType::POOP);
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
const bool GameplayController::checkBoardEmpty() {
    for (int x = 0; x < _windows.getNHorizontal(); x++) {
        for (int y = 0; y < _windows.getNVertical(); y++) {
                if (_windows.getWindowState(y, x) == 1) {
                    return false;
                }
        }
    }
    return true; // No 1s found, board is clear
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
        _player->draw(batch, getSize(), _windows);
        _projectiles.draw(batch, getSize(), _windows.getPaneWidth(), _windows.getPaneHeight());
        if (_curBirdBoard == 0) {
            _bird.draw(batch, getSize(), _curBirdPos);
        }
    }
    else if (_curBoard == -1) {
        _windowsLeft.draw(batch, getSize());
        _playerLeft->draw(batch, getSize(), _windows);
        _player->draw(batch, getSize(), _windows);
        _projectilesLeft.draw(batch, getSize(), _windowsLeft.getPaneWidth(), _windowsLeft.getPaneHeight());
        if (_dirtSelected && _dirtPath.size() != 0) {
            batch->setColor(Color4::BLACK);
            batch->outline(_dirtPath);
        }
        if (_curBirdBoard == -1) {
            _bird.draw(batch, getSize(), _curBirdPos);
        }
    }
    else if (_curBoard == 1) {
        _windowsRight.draw(batch, getSize());
        _playerRight->draw(batch, getSize(), _windows);
        _player->draw(batch, getSize(), _windows);
        _projectilesRight.draw(batch, getSize(), _windowsRight.getPaneWidth(), _windowsRight.getPaneHeight());
        if (_dirtSelected && _dirtPath.size() != 0) {
            batch->setColor(Color4::BLACK);
            batch->outline(_dirtPath);
        }
        if (_curBirdBoard == 1) {
            _bird.draw(batch, getSize(), _curBirdPos);
        }
    }
}


void GameplayController::setActive(bool f) {
    if (!f) {
        _audioController.update(false);
        _isActive=false;
    } else {
        _isActive = true;
    };
}