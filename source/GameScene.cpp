//  GameScene.cpp
//
//  This is the primary class file for running the game.
//
//  Author: High Rise Games
//
#include <cugl/cugl.h>
#include <iostream>
#include <sstream>
#include <random>

#include "GameScene.h"
//#include "GLCollisionController.h"

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
bool GameScene::init(const std::shared_ptr<cugl::AssetManager>& assets, int fps) {
    // Initialize the scene to a locked width
    
    // time of the game set to 200 seconds
    _gameTime = 200;
    
    _frame=0;
    
    // fps as established per App
    _fps = fps;
    
    Size dimen = Application::get()->getDisplaySize();
    _rng.seed(std::time(nullptr));
    _projectileGenChance = 0.1;
    _projectileGenCountDown = 120;
    _dirtGenSpeed = 2;
    _dirtThrowTimer = 0;
    _fixedDirtUpdateThreshold = 5 * 60;
    _maxDirtAmount = 1;
    _currentDirtAmount = 0;
    _curBoard = 0;
    _dirtSelected = false;
    _dirtPath = Path2();
    _onAdjacentBoard = false;
    dimen *= SCENE_HEIGHT/dimen.height;
    if (assets == nullptr) {
        return false;
    } else if (!Scene2::init(dimen)) {
        return false;
    }
    
    _quit = false;
    
    // Start up the input handler
    _assets = assets;
    _input.init(getBounds());
    _dirtThrowInput.init();
    
    // Get the background image and constant values
    _background = _assets->get<Texture>("background");
    _constants = _assets->get<JsonValue>("constants");
    auto layer = _assets->get<scene2::SceneNode>("switch");
    layer->setContentSize(dimen);
    layer->doLayout(); // This rearranges the children to fit the screen

    // Initialize the window grids
    _windows.setTexture(assets->get<Texture>("window")); // MUST SET TEXTURE FIRST
    _windows.init(_constants->get("easy board"), getSize()); // init depends on texture
    _windows.setDirtTexture(assets->get<Texture>("dirt"));

    _windowsLeft.setTexture(assets->get<Texture>("window")); // MUST SET TEXTURE FIRST
    _windowsLeft.init(_constants->get("easy board"), getSize()); // init depends on texture
    _windowsLeft.setDirtTexture(assets->get<Texture>("dirt"));

    _windowsRight.setTexture(assets->get<Texture>("window")); // MUST SET TEXTURE FIRST
    _windowsRight.init(_constants->get("easy board"), getSize()); // init depends on texture
    _windowsRight.setDirtTexture(assets->get<Texture>("dirt"));

    // Initialize projectiles
    _projectiles.setDirtTexture(assets->get<Texture>("dirt"));
    _projectiles.setPoopTexture(assets->get<Texture>("poop"));
    _projectiles.setTextureScales(_windows.getPaneHeight(), _windows.getPaneWidth());
//    _projectiles.init(_constants->get("projectiles"));

    _projectilesLeft.setDirtTexture(assets->get<Texture>("dirt"));
    _projectilesLeft.setPoopTexture(assets->get<Texture>("poop"));
    _projectilesLeft.setTextureScales(_windows.getPaneHeight(), _windows.getPaneWidth());

    _projectilesRight.setDirtTexture(assets->get<Texture>("dirt"));
    _projectilesRight.setPoopTexture(assets->get<Texture>("poop"));
    _projectilesRight.setTextureScales(_windows.getPaneHeight(), _windows.getPaneWidth());

    // Make a ship and set its texture
    // starting position is most bottom left window
    Vec2 startingPos = Vec2(_windows.sideGap + (_windows.getPaneWidth() / 2), _windows.getPaneHeight());

    // no ids given yet - to be assigned in initPlayers()
    _player = std::make_shared<Player>(-1, startingPos, _constants->get("ship"), _windows.getPaneHeight(), _windows.getPaneWidth());
    _player->setTexture(assets->get<Texture>("ship"));
    
    _playerLeft = std::make_shared<Player>(-1, startingPos, _constants->get("ship"), _windows.getPaneHeight(), _windows.getPaneWidth());
    _playerLeft->setTexture(assets->get<Texture>("ship"));

    _playerRight = std::make_shared <Player>(-1, startingPos, _constants->get("ship"), _windows.getPaneHeight(), _windows.getPaneWidth());
    _playerRight->setTexture(assets->get<Texture>("ship"));

    // Initialize random dirt generation
    updateDirtGenTime();
    
    // Initialize dirt bucket
    setEmptyBucket(assets->get<Texture>("bucketempty"));
    setFullBucket(assets->get<Texture>("bucketfull"));
    
//    // Initialize switch scene icon
//    setSwitchSceneButton(assets->get<Texture>("switchSceneButton"));
//    // Initialize return scene icon
//    setReturnSceneButton(assets->get<Texture>("returnSceneButton"));

    // Create and layout the dirt amount
    std::string dirt_msg = strtool::format("%d", _currentDirtAmount);
    _dirtText = TextLayout::allocWithText(dirt_msg, assets->get<Font>("pixel32"));
    _dirtText->layout();

    _collisions.init(getSize());

    // Get the bang sound
    _bang = assets->get<Sound>("bang");

    // Create and layout the health meter
    std::string health_msg = strtool::format("Health %d", _player->getHealth());
    std::string time_msg = strtool::format("Time %d", _player->getHealth());
    _text = TextLayout::allocWithText(time_msg, assets->get<Font>("pixel32"));
    _text->layout();
    _healthText = TextLayout::allocWithText(health_msg, assets->get<Font>("pixel32"));
    _healthText->layout();
    
    reset();
    
    _tn_button = std::dynamic_pointer_cast<scene2::Button>(_assets->get<scene2::SceneNode>("switch_play"));
    _tn_button->addListener([=](const std::string& name, bool down) {
        if (down && _player->getEdge(_windows.sideGap, getSize())) {
            if (_onAdjacentBoard) {
                CULog("leaving other player's board");
            } else {
                CULog("entering other player's board");
            }
            _onAdjacentBoard = !_onAdjacentBoard;
        }
    });
    addChild(layer);
    return true;
}

/** 
 * Initializes the player models for all players, whether host or client. 
 * Sets IDs (corresponds to side of building - TODO: change?), textures (TODO)
 */
bool GameScene::initPlayers(const std::shared_ptr<cugl::AssetManager>& assets) {
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
bool GameScene::initHost(const std::shared_ptr<cugl::AssetManager>& assets) {
    if (assets == nullptr) {
        return false;
    }

    _numPlayers = _network.getNumPlayers();
    initPlayers(assets);

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
    _playerAcross->setTexture(assets->get<Texture>("ship"));

    hostReset();

    return true;
}

/**
 * Disposes of all (non-static) resources allocated to this mode.
 */
void GameScene::dispose() {
    if (_active) {
        removeAllChildren();
        _active = false;
        _tn_button = nullptr;
    }
}


#pragma mark -
#pragma mark Gameplay Handling
void GameScene::reset() {
    Vec2 startingPos = Vec2(_windows.sideGap + (_windows.getPaneWidth() / 2), _windows.getPaneHeight() / 2);
    _player->setPosition(startingPos);
    _player->setAngle(0);
    _player->setVelocity(Vec2::ZERO);
    _player->setHealth(_constants->get("ship")->getInt("health", 0));

    _playerLeft->setPosition(startingPos);
    _playerLeft->setAngle(0);
    _playerLeft->setVelocity(Vec2::ZERO);
    _playerLeft->setHealth(_constants->get("ship")->getInt("health", 0));

    _playerRight->setPosition(startingPos);
    _playerRight->setAngle(0);
    _playerRight->setVelocity(Vec2::ZERO);
    _playerRight->setHealth(_constants->get("ship")->getInt("health", 0));

    _windows.clearBoard();
    _windows.generateInitialBoard(_windows.getInitDirtNum());
    _windowsLeft.clearBoard();
    _windowsLeft.generateInitialBoard(_windows.getInitDirtNum());
    _windowsRight.clearBoard();
    _windowsRight.generateInitialBoard(_windows.getInitDirtNum());

    _projectiles.current.clear();
    _projectiles.init(_constants->get("projectiles"));
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
void GameScene::hostReset() {
    reset();
    // starting position is most bottom left window
    Vec2 startingPos = Vec2(_windows.sideGap+(_windows.getPaneWidth()/2), _windows.getPaneHeight()/2);

    _playerAcross->setPosition(startingPos);
    _playerAcross->setAngle(0);
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
 * Converts game state into a JSON value for sending over the network.
 * Only called by the host, as only the host transmits board states over the network.
 * 
 * @param id    the id of the player of the board state to get
 * @returns JSON value representing game board state
 */
std::shared_ptr<cugl::JsonValue> GameScene::getJsonBoard(int id) {
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
    json->appendValue("player_x", player->getPosition().x);
    json->appendValue("player_y", player->getPosition().y);

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

        const std::shared_ptr<JsonValue> projPos = std::make_shared<JsonValue>();
        projPos->init(JsonValue::Type::ArrayType);
        projPos->appendValue(proj->position.x);
        projPos->appendValue(proj->position.y);
        projJson->appendChild("pos", projPos);

        const std::shared_ptr<JsonValue> projVel = std::make_shared<JsonValue>();
        projVel->init(JsonValue::Type::ArrayType);
        projVel->appendValue(proj->velocity.x);
        projVel->appendValue(proj->velocity.y);
        projJson->appendChild("vel", projVel);

        if (proj->type == ProjectileSet::Projectile::ProjectileType::DIRT) {
            projJson->appendValue("type", "DIRT");
        }
        else if (proj->type == ProjectileSet::Projectile::ProjectileType::POOP) {
            projJson->appendValue("type", "POOP");
        }
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
    "player_x": 30.2,
    "player_y": 124.2,
    "dirts": [ [0, 1], [2, 2], [0, 2] ],
    "projectiles": [ 
            { 
                "pos": [0.5, 1.676],
                "vel": [2, 3],
                "type: "DIRT"
            },
            {
                "pos": [1.5, 3.281],
                "vel": [0, -2], 
                "type": "POOP"
            }
        ]
 * }
*
* @params data     The data to update
*/
void GameScene::updateBoard(std::shared_ptr<JsonValue> data) {
    int playerId = data->getInt("player_id", 0);

    std::shared_ptr<Player> player;
    WindowGrid* windows;
    ProjectileSet* projectiles;
    CULog("playerId: %d", playerId);
    
    if (playerId == _id) {
        // update own board info
        player = _player;
        windows = &_windows;
        projectiles = &_projectiles;
        _currentDirtAmount = data->getInt("num_dirt", 0);
        _curBoard = data->getInt("curr_board", 0);
    }
    else if (playerId == _id + 1 || (_id == 4 && playerId == 1)) {
        // if assigning ids clockwise, this is the left neighbor
        player = _playerLeft;
        windows = &_windowsLeft;
        projectiles = &_projectilesLeft;
    }
    else if (playerId == _id - 1 || (_id == 1 && playerId == 4)) {
        // if assigning ids clockwise, this is the right neighbor
        player = _playerRight;
        windows = &_windowsRight;
        projectiles = &_projectilesRight;
    }
    else {
        // otherwise, player is on the opposite board and we do not need to track their board state.
        return;
    }

    // update neighbor's game states

    // get x, y positions of neighbor
    player->setPosition(Vec2(data->getFloat("player_x", 0), data->getFloat("player_y", 0)));

    // populate neighbor's board with dirt
    windows->clearBoard();
    for (const std::shared_ptr< JsonValue>& jsonDirt : data->get("dirts")->children()) {
        std::vector<int> dirtPos = jsonDirt->asIntArray();
        windows->addDirt(dirtPos[0], dirtPos[1]);
    }
        
    // populate neighbor's projectile set
    projectiles->clearPending(); // clear pending set to rewrite
    for (const std::shared_ptr<JsonValue>& projNode : data->get("projectiles")->children()) {
        // get projectile position
        const std::vector<std::shared_ptr<JsonValue>>& projPos = projNode->get("pos")->children();
        Vec2 pos(projPos[0]->asFloat(), projPos[1]->asFloat());

        // get projectile velocity
        const std::vector<std::shared_ptr<JsonValue>>& projVel = projNode->get("vel")->children();
        Vec2 vel(projVel[0]->asInt(), projVel[1]->asInt());
        
        // get projectile velocity
        const std::vector<std::shared_ptr<JsonValue>>& projDest = projNode->get("dest")->children();
        Vec2 dest(projDest[0]->asInt(), projDest[1]->asInt());

        // get projectile type
        string typeStr = projNode->get("type")->asString();
        auto type = ProjectileSet::Projectile::ProjectileType::POOP;
        if (typeStr == "DIRT") {
            type = ProjectileSet::Projectile::ProjectileType::DIRT;
        }

        // add the projectile to neighbor's projectile set
        projectiles->spawnProjectile(pos, vel, dest, type);
    }
}

/**
 * Converts a movement vector into a JSON value for sending over the network.
 *
 * @param move    the movement vector
 * @returns JSON value representing a movement
 */
std::shared_ptr<cugl::JsonValue> GameScene::getJsonMove(const cugl::Vec2 move) {
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
 *    "switch_destination": 1
 * }
*
* @params data     The data to update
*/
void GameScene::updateFromAction(std::shared_ptr<cugl::JsonValue> data) {
    // TODO: add conditional check here for movement action or other action
    
    // check if action is about scene switching
    if (!(data->has("switch_destination"))) {
        
        int playerId = data->getInt("player_id", 0);
        const std::vector<std::shared_ptr<JsonValue>>& vel = data->get("vel")->children();
        Vec2 moveVec(vel[0]->asFloat(), vel[1]->asFloat());
        
        // playerId can't be 1, since host cannot send action message over network to itself
        if (playerId == 2) {
            stepForward(_playerRight, moveVec, _windowsRight, _projectilesRight);
        }
        else if (playerId == 3) {
            stepForward(_playerAcross, moveVec, _windowsAcross, _projectilesAcross);
        }
        else if (playerId == 4) {
            stepForward(_playerLeft, moveVec, _windowsLeft, _projectilesLeft);
        }
    } // logic for switch scene requests
    else {
        ProcessSceneSwitchRequest(data);
    }
}


/**
* Called by the client only. Client calls this function to transmit message
* to request from the host to switch scenes.
*
* Player ids assigned clockwise with host at top
*
*          host: 1
* left: 4            right: 2
*         across: 3
*
 * Example Scene Request Message:
 * {
 *    "player_id":  1,
 *    "vel": [0.234, 1.153]
 * }
*
*/
void GameScene::SceneSwitchRequest() {
    
    
    // first check that we are on an edge
    // to make a valid scene switch request
    
    if (_player->getEdge(_windows.sideGap, getSize()) == -1 ||
        _player->getEdge(_windows.sideGap, getSize()) == 0) {
        
        const std::shared_ptr<JsonValue> json = std::make_shared<JsonValue>();
        json->init(JsonValue::Type::ObjectType);
        json->appendValue("player_id", static_cast<double>(_id));
        
        // the left or right board we want to switch into
        // initialized to zero or xcode will bother me
        // about it.
        int switch_destination = 0;
        
        if (_player->getEdge(_windows.sideGap, getSize()) == -1) {
            switch_destination = -1;
        }
        
        if (_player->getEdge(_windows.sideGap, getSize()) == 0) {
            switch_destination = 1;
        }
        
        json->appendValue("switch_destination", static_cast<double>(switch_destination));
        
        _network.transmitMessage(json);
        
    }
}


/**
* HOST ONLY. Logic to process switch scene requests.
*/
void GameScene::ProcessSceneSwitchRequest(std::shared_ptr<cugl::JsonValue> data) {
    
    int playerId = data->getInt("player_id", 0);
    int switchDestination = data->getInt("switch_destination", 1);
    
    // update the board of the player to their switch destination
    _allCurBoards[playerId-1] = switchDestination;
    
}


/**
 * The method called to update the game mode.
 *
 * This method contains any gameplay code that is not an OpenGL call.
 * 
 * We need to update this method to constantly talk to the server.
 *
 * @param timestep  The amount of time (in seconds) since the last frame
 */
void GameScene::update(float timestep) {

    // get or transmit board states over network
    if (_network.getConnection()) {
        _network.getConnection()->receive([this](const std::string source,
            const std::vector<std::byte>& data) {
                if (!_ishost) {
                    std::shared_ptr<JsonValue> incomingMsg = _network.processMessage(source, data);
                    if (incomingMsg->has("dirts")) {
                        CULog("got board state message");
                        updateBoard(incomingMsg);
                    }
                }
                else { // is host
                    // process action data - movement or dirt throw
                    std::shared_ptr<JsonValue> incomingMsg = _network.processMessage(source, data);
                    if (incomingMsg->has("vel")) {
                        CULog("got movement message");
                        updateFromAction(incomingMsg);
                    } else if (incomingMsg->has("switch_destination")) {
                        CULog("got switch scene request message");
                        updateFromAction(incomingMsg);
                    }
                }
            });
        _network.checkConnection();

        if (_ishost) {
            for (int i = 1; i <= _numPlayers; i++) {
                _network.transmitMessage(getJsonBoard(i));
                CULog("transmitting board state for player %d", i);
            }
        }
    }

    // When the player is on other's board
    // TODO: now the player is draged as dirt, need to switch to dirt later
    if (_curBoard != 0) {
        _dirtThrowInput.update();
        if (_curBoard == -1) {
            _projectilesLeft.update(getSize());
        }
        else if (_curBoard == 1) {
            _projectilesRight.update(getSize());
        }
        if (!_dirtSelected) {
            if (_dirtThrowInput.didPress()) {
                Vec2 screenPos = _dirtThrowInput.getPosition();
                Vec3 convertedWorldPos = screenToWorldCoords(screenPos);
                Vec2 worldPos = Vec2(convertedWorldPos.x, convertedWorldPos.y);
                float distance = (worldPos - _player->getPosition()).length();
                if (distance <= _player->getRadius()) {
                    _dirtSelected = true;
                    _prevDirtPos = _player->getPosition();
                }
            }
        } else {
            if (_dirtThrowInput.didRelease()) {
                _dirtSelected = false;
                Vec2 currentScreenPos = _dirtThrowInput.getPosition();
                Vec3 currentWorldPos = screenToWorldCoords(currentScreenPos);
                Vec2 currentPos = Vec2(currentWorldPos.x, currentWorldPos.y);
                Vec2 diff = currentPos - _prevDirtPos;
                Vec2 velocity = diff.getNormalization() * -5;
                Vec2 destination = currentPos - diff * 5;
                // logic for throwing the dirt = converting to projectile
                if (_curBoard == -1) {
                    _projectilesLeft.spawnProjectile(currentPos, velocity, destination, ProjectileSet::Projectile::ProjectileType::DIRT);
                }
                else if (_curBoard == 1) {
                    _projectilesRight.spawnProjectile(currentPos, velocity, destination, ProjectileSet::Projectile::ProjectileType::DIRT);
                }
            } else if (_dirtThrowInput.isDown()) {
                Vec2 currentScreenPos = _dirtThrowInput.getPosition();
                Vec3 currentWorldPos = screenToWorldCoords(currentScreenPos);
                Vec2 currentPos = Vec2(currentWorldPos.x, currentWorldPos.y);
                _player->setPosition(currentPos);
                std::vector<Vec2> vertices = {currentPos};
                Vec2 diff = currentPos - _prevDirtPos;
                Vec2 destination = currentPos - diff * 5;
                vertices.push_back(destination);
                _dirtPath = Path2(vertices);
            }
        }
    }
    // When a player is on their own board
    else { 
        if (!_ishost) {
            // Read the keyboard for each controller.
            _input.update();
            // pass movement over network for host to process
            if (_network.getConnection()) {
                _network.checkConnection();
                if (_input.getDir().length() > 0 && !_ishost) {
                    CULog("transmitting movement message over network for player %d", _id);
                    std::shared_ptr<JsonValue> m = getJsonMove(_input.getDir());
                    std::string s = m->toString();
                    _network.transmitMessage(m);
                }
                // send over scene switch requests if necessary
                SceneSwitchRequest();
            }
        }
        if (_ishost) {
            _input.update();
            if (_input.didPressReset()) {
                // host resets game for all players
                hostReset();
            }
            // update the game state for self (host). Updates for the rest of the players are done in updateFromAction(),
            // called whenever the host recieves a movement or other action message.
            stepForward(_player, _input.getDir(), _windows, _projectiles);
            _currentDirtAmount = _allDirtAmounts[0];
            _curBoard = _allCurBoards[0];
        }

        // each player manages their own UI elements/text boxes for displaying resource information
        // Update the health meter
        _healthText->setText(strtool::format("Health %d", _player->getHealth()));

        // update time
        if ((_gameTime>=1)) {
            _frame = _frame+1;
        } if (_frame==_fps && (_gameTime>=1)) {
            _gameTime=_gameTime-1;
           _projectileGenChance = max(0.8, _projectileGenChance + (200 - _gameTime) * 0.002);
            _frame = 0;
        }
        
        
        _text->setText(strtool::format("Time %d", _gameTime));
        
        
        _healthText->layout();
        _text->layout();
        
        // Update the dirt display
        _dirtText->setText(strtool::format("%d", _currentDirtAmount));
        _dirtText->layout();
        
        _tn_button->setVisible(true);
        _tn_button->activate();
    }
    

    
}

/**
* FOR HOST ONLY. This method does all the heavy lifting work for update.
* The host steps forward each player's game state, given references to the player, board, and projectile set.
*/
void GameScene::stepForward(std::shared_ptr<Player>& player, const Vec2 moveVec, WindowGrid& windows, ProjectileSet& projectiles) {
    //Checks and returns true if board is full besides current player position
        //if (windows.checkBoardFull()) {
            // TODO: implement lose screen here?
            // TODO: make checkBoardFull a method of WindowGrid, not GameScene
        //}

    bool movedOverEdge = false;
    // Check if player is stunned for this frame
    if (player->getStunFrames() > 0) {
        player->decreaseStunFrames();
    }
    else {
        // Move the player, ignoring collisions
        bool validMove = player->move(moveVec, getSize(), windows.sideGap);
        // Player tried to move over the edge if the above call to move was not "valid", i.e. player tried
        // to move off of the board (which should constitute a toss dirt action).
        movedOverEdge = !validMove;
    }

    // remove any dirt the player collides with
    Vec2 grid_coors = player->getCoorsFromPos(windows.getPaneHeight(), windows.getPaneWidth(), windows.sideGap);
    player->setCoors(grid_coors);
    //    if (grid_coors == NULL) {
    //        CULog("player coors: NULL");
    //        CULog("player coors: (%f, %f)", grid_coors.y, grid_coors.x);
    //    }

    int player_id = player->getId();
    bool dirtRemoved = windows.removeDirt(grid_coors.y, grid_coors.x);
    if (dirtRemoved) {
        // filling up dirty bucket
        _allDirtAmounts[player_id -1] = min(_maxDirtAmount, _allDirtAmounts[player_id - 1] + 1);
    }

    if (player->getEdge(windows.sideGap, getSize()) && !movedOverEdge) {
        _allDirtAmounts[player_id -1] = max(0, _allDirtAmounts[player_id - 1] - 1);
        // TODO: currently moving to either edge sends user to the right, only if a user exists to the right.
        if (player_id + 1 <= _numPlayers || player_id == 4) {
            _allCurBoards[player_id - 1] = 1;
        }
        
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

    // dynamic dirt generation logic (based on generation rate)
    
    // Generate falling hazard based on chance
    if (_projectileGenCountDown == 0) {
        std::bernoulli_distribution dist(_projectileGenChance);
        if (dist(_rng)) {
            // random dirt generation logic
            CULog("generating random poo");
            generatePoo();
         }
        _projectileGenCountDown = 120;
     }
     else {
         _projectileGenCountDown -= 1;
     }
        
    // Move the projectiles
    projectiles.update(getSize());

    // Check for collisions and play sound
    if (player->getStunFrames() <= 0 && _collisions.resolveCollision(player, projectiles)) {
        AudioEngine::get()->play("bang", _bang, false, _bang->getVolume(), true);
    }
}

/** update when dirt is generated */
void GameScene::updateDirtGenTime() {
    _dirtGenTimes.clear();
    std::uniform_int_distribution<> distr(0, _fixedDirtUpdateThreshold);
    for(int n=0; n<_dirtGenSpeed; ++n) {
        _dirtGenTimes.insert(distr(_rng));
    }
}

/** handles dirt generation */
void GameScene::generateDirt() {
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
void GameScene::generatePoo() {
    std::uniform_real_distribution<float> rowDist(_windows.sideGap + 40, (float)getSize().getIWidth() - _windows.sideGap - 60);
    float rand_row = rowDist(_rng);
//    CULog("player at: (%f, %f)", _player->getCoors().y, _player->getCoors().x);
//    CULog("generate at: %d", (int)rand_row);
    // if add dirt already exists at location or player at location and board is not full, repeat
    _projectiles.spawnProjectile(Vec2(rand_row, getSize().height - 50), Vec2(0, min(-2.4f,-2-_projectileGenChance)), ProjectileSet::Projectile::ProjectileType::POOP);
}

/** Checks whether board is full except player current location*/
const bool GameScene::checkBoardFull() {
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
const bool GameScene::checkBoardEmpty() {
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
void GameScene::render(const std::shared_ptr<cugl::SpriteBatch>& batch) {
    // For now we render 3152-style
    // DO NOT DO THIS IN YOUR FINAL GAME
    CULog("current board: %d", _curBoard);
    batch->begin(getCamera()->getCombined());
    
    batch->draw(_background,Rect(Vec2::ZERO,getSize()));
    //_asteroids.draw(batch,getSize());
    if (_curBoard == 0) {
        _windows.draw(batch, getSize());
        _player->draw(batch, getSize());
        _projectiles.draw(batch, getSize(), _windows.getPaneWidth(), _windows.getPaneHeight());
    }
    else if (_curBoard == -1) {
        _windowsLeft.draw(batch, getSize());
        _playerLeft->draw(batch, getSize());
        _player->draw(batch, getSize());
        _projectilesLeft.draw(batch, getSize(), _windowsLeft.getPaneWidth(), _windowsLeft.getPaneHeight());
        if (_dirtSelected && _dirtPath.size() != 0) {
            batch->setColor(Color4::BLACK);
            batch->outline(_dirtPath);
        }
    }
    else if (_curBoard == 1) {
        _windowsRight.draw(batch, getSize());
        _playerRight->draw(batch, getSize());
        _player->draw(batch, getSize());
        _projectilesRight.draw(batch, getSize(), _windowsRight.getPaneWidth(), _windowsRight.getPaneHeight());
        if (_dirtSelected && _dirtPath.size() != 0) {
            batch->setColor(Color4::BLACK);
            batch->outline(_dirtPath);
        }
    }
    batch->setColor(Color4::BLACK);
    batch->drawText(_text, Vec2(getSize().width - 10 - _text->getBounds().size.width, getSize().height - _text->getBounds().size.height));
    batch->drawText(_healthText, Vec2(10, getSize().height - _healthText->getBounds().size.height));
    
    //set bucket texture location
    Affine2 bucketTrans = Affine2();
    Vec2 bOrigin(_fullBucket->getWidth()/2,_fullBucket->getHeight()/2);
    float bucketScaleFactor = std::min(((float)getSize().getIWidth() / (float)_fullBucket->getWidth()) /2, ((float)getSize().getIHeight() / (float)_fullBucket->getHeight() /2));
    bucketTrans.scale(bucketScaleFactor);
    
    Vec2 bucketLocation(getSize().width - ((float)_fullBucket->getWidth() * bucketScaleFactor/2),
                        getSize().height - (float)_fullBucket->getHeight() * bucketScaleFactor/2);
    bucketTrans.translate(bucketLocation);
    
//    //set switch/return scene button texture location
//    Affine2 sceneButtonTrans = Affine2();
//    Vec2 sbOrigin(_switchSceneButton->getWidth()/2,_switchSceneButton->getHeight()/2);
//    float sceneButtonScaleFactor = std::min(((float)getSize().getIWidth() / (float)_switchSceneButton->getWidth()) /2, ((float)getSize().getIHeight() / (float)_switchSceneButton->getHeight() /2));
//    sceneButtonTrans.scale(sceneButtonScaleFactor);
//    
//    Vec2 sceneButtonLocation(getSize().width - ((float)_switchSceneButton->getWidth() * sceneButtonScaleFactor/2),
//                        (float)_switchSceneButton->getHeight() * sceneButtonScaleFactor/2);
//    sceneButtonTrans.translate(sceneButtonLocation);
    
    // draw different bucket based on dirt amount
    if (_currentDirtAmount == 0) {
        batch->draw(_emptyBucket, bOrigin, bucketTrans);
    } else {
        batch->draw(_fullBucket, bOrigin, bucketTrans);
    }
    
//    // draw scene button
//    if (_onAdjacentBoard) {
//        batch->draw(_returnSceneButton, sbOrigin, sceneButtonTrans);
//    } else {
//        batch->draw(_switchSceneButton, sbOrigin, sceneButtonTrans);
//    }
    
    // draw dirt amount text, centered at bucket
    Affine2 dirtTextTrans = Affine2();
    dirtTextTrans.scale(1.2);
    dirtTextTrans.translate(bucketLocation.x - _dirtText->getBounds().size.width/2,
                            bucketLocation.y);
    batch->setColor(Color4::BLACK);
    batch->drawText(_dirtText, dirtTextTrans);
    batch->setColor(Color4::WHITE);
    
    
    batch->end();
}

