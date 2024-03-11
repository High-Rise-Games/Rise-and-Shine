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
    
    // fps as established per App
    _fps = fps;
    
    Size dimen = Application::get()->getDisplaySize();
    _rng.seed(std::time(nullptr));
    _dirtGenSpeed = 2;
    _dirtThrowTimer = 0;
    _fixedDirtUpdateThreshold = 5 * 60;
    _maxDirtAmount = 1;
    _currentDirtAmount = 0;
    _curBoard = 0;
    _dirtSelected = false;
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

    // Initialize the window grid
    _windows.setTexture(assets->get<Texture>("window")); // MUST SET TEXTURE FIRST
    _windows.init(_constants->get("easy board"), getSize()); // init depends on texture
    _windows.setDirtTexture(assets->get<Texture>("dirt"));

    // Initialize projectiles
    _projectiles.setDirtTexture(assets->get<Texture>("dirt"));
    _projectiles.setPoopTexture(assets->get<Texture>("poop"));
    _projectiles.setTextureScales(_windows.getPaneHeight(), _windows.getPaneWidth());
    _projectiles.init(_constants->get("projectiles"));

    _projectilesLeft.setDirtTexture(assets->get<Texture>("dirt"));
    _projectilesLeft.setPoopTexture(assets->get<Texture>("poop"));
    _projectilesLeft.setTextureScales(_windows.getPaneHeight(), _windows.getPaneWidth());

    _projectilesRight.setDirtTexture(assets->get<Texture>("dirt"));
    _projectilesRight.setPoopTexture(assets->get<Texture>("poop"));
    _projectilesRight.setTextureScales(_windows.getPaneHeight(), _windows.getPaneWidth());

    
    // Make a ship and set its texture
    // starting position is most bottom left window
    Vec2 startingPos = Vec2(_windows.sideGap+(_windows.getPaneWidth()/2), _windows.getPaneHeight());
    
    // TODO: host hands out ids, but can't do that until host is in charge of all board states.
    _player = std::make_shared<Player>(2, startingPos, _constants->get("ship"), _windows.getPaneHeight(), _windows.getPaneWidth());
    _player->setTexture(assets->get<Texture>("ship"));
    

    _playerLeft = std::make_shared<Player>(1, startingPos, _constants->get("ship"), _windows.getPaneHeight(), _windows.getPaneWidth());
    _playerLeft->setTexture(assets->get<Texture>("ship"));

    _playerRight = std::make_shared <Player>(3, startingPos, _constants->get("ship"), _windows.getPaneHeight(), _windows.getPaneWidth());
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

    // Get the bang sound
    _bang = assets->get<Sound>("bang");

    // Create and layout the health meter
    std::string health_msg = strtool::format("Health %d", _player->getHealth());
    _text = TextLayout::allocWithText(health_msg, assets->get<Font>("pixel32"));
    _text->layout();

    // Create and layout the dirt amount
    std::string dirt_msg = strtool::format("%d", _currentDirtAmount);
    _dirtText = TextLayout::allocWithText(dirt_msg, assets->get<Font>("pixel32"));
    _dirtText->layout();
    
    _collisions.init(getSize());
    
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
/**
 * Resets the status of the game so that we can play again.
 */
void GameScene::reset() {
    // starting position is most bottom left window
    Vec2 startingPos = Vec2(_windows.sideGap+(_windows.getPaneWidth()/2), _windows.getPaneHeight()/2);
    _player->setPosition(startingPos);
    _player->setAngle(0);
    _player->setVelocity(Vec2::ZERO);
    _player->setHealth(_constants->get("ship")->getInt("health",0));
    _windows.clearBoard();
    _windows.generateInitialBoard(_windows.getInitDirtNum());
    _projectiles.current.clear();
    _projectiles.init(_constants->get("projectiles"));
    _dirtThrowTimer = 0;
    _currentDirtAmount = 0;
}

/** 
 * Converts game state into a JSON value for sending over the network.
 * @returns JSON value representing game board state
 */
std::shared_ptr<cugl::JsonValue> GameScene::getJsonBoard() {
    const std::shared_ptr<JsonValue> json;
    json->init(JsonValue::Type::ObjectType);
    json->appendValue("player_id", static_cast<double>(_player->getId()));
    json->appendValue("player_x", _player->getPosition().x);
    json->appendValue("player_y", _player->getPosition().y);

    const std::shared_ptr<JsonValue> dirtArray;
    dirtArray->init(JsonValue::Type::ArrayType);
    for (int row = 0; row < _windows.getNHorizontal(); ++row) {
        for (int col = 0; col < _windows.getNVertical(); ++col) {
            bool hasDirt = _windows.getWindowState(row, col);
            if (hasDirt) {
                const std::shared_ptr<JsonValue> dirtPos;
                dirtPos->init(JsonValue::Type::ArrayType);
                dirtPos->appendValue(static_cast<double>(row));
                dirtPos->appendValue(static_cast<double>(col));
                dirtArray->appendChild(dirtPos);
            }
        }
    }
    
    const std::shared_ptr<JsonValue> projPosArray;
    projPosArray->init(JsonValue::Type::ArrayType);

    const std::shared_ptr<JsonValue> projTypeArray;
    projTypeArray->init(JsonValue::Type::ArrayType);

    for (shared_ptr<ProjectileSet::Projectile> proj : _projectiles.current) {
        const std::shared_ptr<JsonValue> projPos;
        projPos->init(JsonValue::Type::ArrayType);
        projPos->appendValue(proj->position.x);
        projPos->appendValue(proj->position.y);
        projPosArray->appendChild(projPos);

        if (proj->type == ProjectileSet::Projectile::ProjectileType::DIRT) {
            projTypeArray->appendValue("DIRT");
        }
        else if (proj->type == ProjectileSet::Projectile::ProjectileType::POOP) {
            projTypeArray->appendValue("POOP");
        }
    }

    return json;
}

/**
* Updates a neighboring board given the JSON value representing its game state
* 
* * Example board state:
 * {
    "player_id":  1,
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
void GameScene::updateNeighborBoard(std::shared_ptr<JsonValue> data) {
    int playerId = data->getInt("player_id", 0);;
    int myId = _player->getId();
    std::shared_ptr<Player> neighborPlayer = _player;
    WindowGrid* neighborWindow;
    ProjectileSet* neighborProjSet;
    
    if (playerId == myId + 1 || (myId == 4 && playerId == 1)) {
        // if assigning ids clockwise, this is the left neighbor
        neighborPlayer = _playerLeft;
        neighborWindow = &_windowsLeft;
        neighborProjSet = &_projectilesLeft;
    }
    else if (playerId == myId - 1 || (myId == 1 && playerId == 4)) {
        // if assigning ids clockwise, this is the right neighbor
        neighborPlayer = _playerRight;
        neighborWindow = &_windowsRight;
        neighborProjSet = &_projectilesRight;
    }
    else {
        // otherwise, player is on the opposite board and we do not need to track their board state.
        return;
    }

    // update neighbor's game states

    // get x, y positions of neighbor
    neighborPlayer->setPosition(Vec2(data->getFloat("player_x", 0), data->getFloat("player_y", 0)));

    // populate neighbor's board with dirt
    neighborWindow->clearBoard();
    for (const std::shared_ptr< JsonValue>& jsonDirt : data->get("dirts")->children()) {
        std::vector<int> dirtPos = jsonDirt->asIntArray();
        neighborWindow->addDirt(dirtPos[0], dirtPos[1]);
    }

    // populate neighbor's projectile set
    neighborProjSet->clearPending(); // clear pending set to rewrite
    for (const std::shared_ptr<JsonValue>& projNode : data->get("projectiles")->children()) {
        // get projectile position
        const std::vector<std::shared_ptr<JsonValue>>& projPos = projNode->get("pos")->children();
        Vec2 pos(projPos[0]->asFloat(), projPos[1]->asFloat());

        // get projectile velocity
        const std::vector<std::shared_ptr<JsonValue>>& projVel = projNode->get("vel")->children();
        Vec2 vel(projVel[0]->asInt(), projVel[1]->asInt());

        // get projectile type
        string typeStr = projNode->get("type")->asString();
        auto type = ProjectileSet::Projectile::ProjectileType::POOP;
        if (typeStr == "DIRT") {
            type = ProjectileSet::Projectile::ProjectileType::DIRT;
        }

        // add the projectile to neighbor's projectile set
        neighborProjSet->spawnProjectile(pos, vel, type);
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
 */
void GameScene::update(float timestep) {
    if (_network.getConnection()) {
        _network.getConnection()->receive([this](const std::string source,
            const std::vector<std::byte>& data) {
                std::shared_ptr<JsonValue> incomingMsg = _network.processData(source, data);
                updateNeighborBoard(incomingMsg);
            });
        _network.checkConnection();

        _network.transmitBoard(getJsonBoard());
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
                std::cout<<"finger: x: "<<worldPos.x<<", y: "<<worldPos.y;
                std::cout<<"player: x: "<<_player->getPosition().x<<", y: "<<_player->getPosition().y;
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
                Vec2 currentWorldPos = screenToWorldCoords(currentScreenPos);
                // Vec2 currentPos = Vec2(currentWorldPos.x, currentWorldPos.y);
                Vec2 diff = currentWorldPos - _prevDirtPos;
                Vec2 velocity = diff * -0.5;
                // logic for throwing the dirt = converting to projectile
                if (_curBoard == -1) {
                    _projectilesLeft.spawnProjectile(currentWorldPos, velocity, ProjectileSet::Projectile::ProjectileType::DIRT);
                }
                else if (_curBoard == 1) {
                    _projectilesRight.spawnProjectile(currentWorldPos, velocity, ProjectileSet::Projectile::ProjectileType::DIRT);
                }
            } else if (_dirtThrowInput.isDown()) {
                Vec2 currentScreenPos = _dirtThrowInput.getPosition();
                Vec3 currentWorldPos = screenToWorldCoords(currentScreenPos);
                Vec2 currentPos = Vec2(currentWorldPos.x, currentWorldPos.y);
                _player->setPosition(currentPos);
            }
        }
    }
    
    // When the player is on his own board
    else {
        
        // Read the keyboard for each controller.
        _input.update();
        if (_input.didPressReset()) {
            reset();
        }
        
        

        //Checks and returns true if board is full besides current player position
        if (checkBoardFull()) {
            //TODO: implment lose screen here?
        }
        

        bool movedOverEdge = false;
        // Check if player is stunned for this frame
        if (_player->getStunFrames() > 0) {
            _player->decreaseStunFrames();
        }
        else {
            // Move the player, ignoring collisions
            bool validMove = _player->move(_input.getDir(), getSize(), _windows.sideGap);
            // Player tried to move over the edge if the above call to move was not "valid", i.e. player tried
            // to move off of the board (which should constitute a toss dirt action).
            movedOverEdge = !validMove;
        }
            
        // remove any dirt the player collides with
        Vec2 grid_coors = _player->getCoorsFromPos(_windows.getPaneHeight(), _windows.getPaneWidth(), _windows.sideGap);
        _player->setCoors(grid_coors);
    //    if (grid_coors == NULL) {
    //        CULog("player coors: NULL");
    //        CULog("player coors: (%f, %f)", grid_coors.y, grid_coors.x);
    //    }
        bool dirtRemoved = _windows.removeDirt(grid_coors.y, grid_coors.x);
        if (dirtRemoved) {
            // filling up dirty bucket
            _currentDirtAmount = min(_maxDirtAmount, _currentDirtAmount + 1);
        }
        
        if (_player->getEdge(_windows.sideGap, getSize()) && !movedOverEdge) {
            _currentDirtAmount = max(0, _currentDirtAmount - 1);
        }
        
        // fixed dirt generation logic (every 5 seconds)
        if (!checkBoardEmpty() && !checkBoardFull()) {
            if (_dirtThrowTimer <= _fixedDirtUpdateThreshold) {
                auto search = _dirtGenTimes.find(_dirtThrowTimer);
                if (search != _dirtGenTimes.end()) {
                    // random dirt generation logic
    //                CULog("generating random dirt");
                    generateDirt();
                }
                _dirtThrowTimer++;
            } else {
                _dirtThrowTimer = 0;
                updateDirtGenTime();
    //            CULog("generating fixed dirt");
                generateDirt();
            }
        }
        
        // dynamic dirt generation logic (based on generation rate)

        
        // Move the projectiles
        _projectiles.update(getSize());
        
        // Check for collisions and play sound
        if (_player->getStunFrames() <= 0 && _collisions.resolveCollision(_player, _projectiles)) {
            AudioEngine::get()->play("bang", _bang, false, _bang->getVolume(), true);
        }

        // Update time
        _text->setText(strtool::format("Time %d", _gameTime));
//        gameTime=-1;
        
        _frame = _frame+1;
        if (_frame==_fps) {
            _gameTime=_gameTime-1;
            _frame = 0;
        }
        _text->layout();
        
        // Update the dirt display
        _dirtText->setText(strtool::format("%d", _currentDirtAmount));
        _dirtText->layout();
        
        _tn_button->setVisible(true);
        _tn_button->activate();
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

/** handles actual dirt generation */
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
    batch->begin(getCamera()->getCombined());
    
    batch->draw(_background,Rect(Vec2::ZERO,getSize()));
    //_asteroids.draw(batch,getSize());
    if (_curBoard == 0) {
        _windows.draw(batch, getSize());
        _player->draw(batch, getSize());
        _projectiles.draw(batch, getSize(), _windows.getPaneWidth(), _windows.getPaneHeight());
    }
    else if (_curBoard == -1) {
        _windows.draw(batch, getSize());
        _player->draw(batch, getSize());
        _projectilesLeft.draw(batch, getSize(), _windows.getPaneWidth(), _windows.getPaneHeight());
    }
    else if (_curBoard == 1) {
        _windows.draw(batch, getSize());
        _player->draw(batch, getSize());
        _projectilesRight.draw(batch, getSize(), _windows.getPaneWidth(), _windows.getPaneHeight());
    }
    batch->setColor(Color4::BLACK);
    batch->drawText(_text, Vec2(10, getSize().height - _text->getBounds().size.height));
    
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

