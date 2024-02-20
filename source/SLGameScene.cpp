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
    _dirtGenRate = 0.6;
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
        _dirtThrowTimer++;
    } else {
        _dirtThrowTimer = 0;
        CULog("generating fixed dirt");
    }
    
    // dynamic dirt generation logic (based on generation rate)
//    std::mt19937 rng(std::time(nullptr));
//    std::uniform_int_distribution<> distr(0, _fixedDirtUpdateThreshold);
//    for(int n=0; n<5; ++n)
//            std::cout << distr(eng) << ' ';
    
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

