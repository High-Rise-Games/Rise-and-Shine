//
//  SLGameScene.cpp
//  Ship Lab
//
//  This is the primary class file for running the game.  You should study this file
//  for ideas on how to structure your own root class. This class is a reimagining of
//  the first game lab from 3152 in CUGL.
//
//  Author: Walker White
//  Based on original GameX Ship Demo by Rama C. Hoetzlein, 2002
//  Version: 1/20/22
//
#include <cugl/cugl.h>
#include <iostream>
#include <sstream>
#include <random>

#include "SLGameScene.h"
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
bool GameScene::init(const std::shared_ptr<cugl::AssetManager>& assets) {
    // Initialize the scene to a locked width
    Size dimen = Application::get()->getDisplaySize();
    _rng.seed(std::time(nullptr));
    _dirtGenSpeed = 2;
    _dirtThrowTimer = 0;
    _fixedDirtUpdateThreshold = 5 * 60;
    _maxDirtAmount = 1;
    _currentDirtAmount = 0;
    dimen *= SCENE_HEIGHT/dimen.height;
    if (assets == nullptr) {
        return false;
    } else if (!Scene2::init(dimen)) {
        return false;
    }
    
    // Start up the input handler
    _assets = assets;
    
    // Get the background image and constant values
    _background = assets->get<Texture>("background");
    _constants = assets->get<JsonValue>("constants");


    // Make a ship and set its texture
    _player = std::make_shared<Player>(getSize()/2, _constants->get("ship"));
    _player->setTexture(assets->get<Texture>("ship"));

    // Initialize the window grid
    _windows.setTexture(assets->get<Texture>("window")); // MUST SET TEXTURE FIRST
    _windows.init(_constants->get("easy board"), getSize()); // init depends on texture
    _windows.setDirtTexture(assets->get<Texture>("dirt"));
    
    // Initialize random dirt generation
    updateDirtGenTime();
    
    // Initialize dirt bucket
    setEmptyBucket(assets->get<Texture>("bucketempty"));
    setFullBucket(assets->get<Texture>("bucketfull"));

    // Initialize the asteroid set
    //_asteroids.init(_constants->get("asteroids"));
    //_asteroids.setTexture(assets->get<Texture>("asteroid1"));

    // Get the bang sound
    _bang = assets->get<Sound>("bang");

    // Create and layout the dirt amount
    std::string msg = strtool::format("%d", _currentDirtAmount);
    _dirtText = TextLayout::allocWithText(msg, assets->get<Font>("pixel32"));
    _dirtText->layout();
    
    //_collisions.init(getSize());
    
    reset();
    return true;
}

/**
 * Disposes of all (non-static) resources allocated to this mode.
 */
void GameScene::dispose() {
    if (_active) {
        removeAllChildren();
        _active = false;
    }
}


#pragma mark -
#pragma mark Gameplay Handling
/**
 * Resets the status of the game so that we can play again.
 */
void GameScene::reset() {
    _player->setPosition(getSize()/2);
    _player->setAngle(0);
    _player->setVelocity(Vec2::ZERO);
    _player->setHealth(_constants->get("ship")->getInt("health",0));
    //_asteroids.init(_constants->get("asteroids"));
}

/**
 * The method called to update the game mode.
 *
 * This method contains any gameplay code that is not an OpenGL call.
 *
 * @param timestep  The amount of time (in seconds) since the last frame
 */
void GameScene::update(float timestep) {
    // Read the keyboard for each controller.
    _input.readInput();
    if (_input.didPressReset()) {
        reset();
    }

    //Checks and returns true if board is full besides current player position
    if (checkBoardFull()) {
        //TODO: implment lose screen here?
    }

    // Move the player, ignoring collisions
    _player->move( _input.getForward(),  _input.getTurn(), getSize(), _windows.sideGap);
    // remove any dirt the player collides with
    Vec2 grid_coors = _player->getCoorsFromPos(_windows.getPaneHeight(), _windows.getPaneWidth(), _windows.sideGap);
    _player->setCoors(grid_coors);
//    CULog("player coors: (%f, %f)", grid_coors.x, grid_coors.y);
    bool dirtRemoved = _windows.removeDirt(grid_coors.x, grid_coors.y);
    if (dirtRemoved) {
        // TODO: implement logic to deal with filling up dirty bucket
        _currentDirtAmount = min(_maxDirtAmount, _currentDirtAmount + 1);
    }
    
    if (_player->atEdge(_windows.sideGap)) {
        _currentDirtAmount = max(0, _currentDirtAmount - 1);
    }
    
    // fixed dirt generation logic (every 5 seconds)
    if (_dirtThrowTimer <= _fixedDirtUpdateThreshold) {
        auto search = _dirtGenTimes.find(_dirtThrowTimer);
        if (search != _dirtGenTimes.end()) {
            // random dirt generation logic
            CULog("generating random dirt");
            generateDirt();
        }
        _dirtThrowTimer++;
    } else {
        _dirtThrowTimer = 0;
        updateDirtGenTime();
        CULog("generating fixed dirt");
        generateDirt();
    }
    
    // dynamic dirt generation logic (based on generation rate)

    
    // Move the asteroids
    //_asteroids.update(getSize());
    
    // Check for collisions and play sound
    //if (_collisions.resolveCollision(_ship, _asteroids)) {
    //    AudioEngine::get()->play("bang", _bang, false, _bang->getVolume(), true);
    //}
    
    // Update the dirt display
    _dirtText->setText(strtool::format("%d", _currentDirtAmount));
    _dirtText->layout();
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
    std::uniform_int_distribution<int> rowDist(0, _windows.getNHorizontal() - 1);
    std::uniform_int_distribution<int> colDist(0, _windows.getNVertical() - 1);
    int rand_row = rowDist(_rng);
    int rand_col = colDist(_rng);
    // if add dirt already exists at location or player at location and board is not full, repeat
    while ((_player->getCoors() == Vec2(rand_row, rand_col) || !_windows.addDirt(rand_row, rand_col)) && !checkBoardFull()) {
        rand_row = rowDist(_rng);
        rand_col = colDist(_rng);
    }
}

/** Checks whether board is full except player current location*/
const bool GameScene::checkBoardFull() {
    for (int x = 0; x < _windows.getNHorizontal(); x++) {
        for (int y = 0; y < _windows.getNVertical(); y++) {
                if (_windows.getWindowState(x, y) == 0 && !(_player->getCoors() != Vec2(x, y))) {
                    return false; // Found a 0, at least one clean spot
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
void GameScene::render(const std::shared_ptr<cugl::SpriteBatch>& batch) {
    // For now we render 3152-style
    // DO NOT DO THIS IN YOUR FINAL GAME
    batch->begin(getCamera()->getCombined());
    
    batch->draw(_background,Rect(Vec2::ZERO,getSize()));
    //_asteroids.draw(batch,getSize());
    _windows.draw(batch, getSize());
    _player->draw(batch, getSize());
    
    
    //set bucket texture location
    Affine2 bucket_trans = Affine2();
    Vec2 origin(_fullBucket->getWidth()/2,_fullBucket->getHeight()/2);
    float bucketScaleFactor = std::min(((float)getSize().getIWidth() / (float)_fullBucket->getWidth()) /2, ((float)getSize().getIHeight() / (float)_fullBucket->getHeight() /2));
    bucket_trans.scale(bucketScaleFactor);
    
    Vec2 bucketLocation(getSize().width - ((float)_fullBucket->getWidth() * bucketScaleFactor/2),
                        (float)_fullBucket->getHeight() * bucketScaleFactor/2);
    bucket_trans.translate(bucketLocation);
    
    // draw different bucket based on dirt amount
    if (_currentDirtAmount == 0) {
        batch->draw(_emptyBucket, origin, bucket_trans);
    } else {
        batch->draw(_fullBucket, origin, bucket_trans);
    }
    
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

