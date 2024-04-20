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
    
    Size dimen = Application::get()->getDisplaySize();

    dimen *= SCENE_HEIGHT/dimen.height;
    if (assets == nullptr) {
        return false;
    } else if (!Scene2::init(dimen)) {
        return false;
    }
    
    // Start up the input handler
    _assets = assets;
    _dirtThrowInput.init();

    // Get the background image and constant values
    _background = _assets->get<Texture>("night level background");
    _parallax = _assets->get<Texture>("night level parallax");
    _background->setWrapS(GL_CLAMP_TO_EDGE);
    _background->setWrapT(GL_CLAMP_TO_EDGE);
    _constants = _assets->get<JsonValue>("constants");

    // test progress bar for player
    vector<string> barNames = { "greenbar", "bluebar", "redbar", "yellowbar" };
    for (int i = 0; i < 4; i++) {
        auto currBar = std::dynamic_pointer_cast<scene2::ProgressBar>(assets->get<scene2::SceneNode>("game_" + barNames[i]));
        currBar->setAngle(1.5708);
        currBar->setScale(2);
        currBar->setVisible(false);
        _player_bars.push_back(currBar);
    }
    _char_to_barIdx["Chameleon"] = 0;
    _char_to_barIdx["Frog"] = 1;
    _char_to_barIdx["Mushroom"] = 2;
    _char_to_barIdx["Flower"] = 3;
    
    // Initialize dirt bucket
    setEmptyBucket(assets->get<Texture>("bucketempty"));
    setFullBucket(assets->get<Texture>("bucketfull"));
    
    _countdown1 =assets->get<Texture>("countdown1");
    
//    // Initialize switch scene icon
//    setSwitchSceneButton(assets->get<Texture>("switchSceneButton"));
//    // Initialize return scene icon
//    setReturnSceneButton(assets->get<Texture>("returnSceneButton"));

    // Create and layout the dirt amount
    std::string dirt_msg = "0";
    _dirtText = TextLayout::allocWithText(dirt_msg, assets->get<Font>("pixel32"));
    _dirtText->layout();


    // Create and layout the health meter
    std::string health_msg = "Health";
    std::string time_msg = "Time";
    _timeText = TextLayout::allocWithText(time_msg, assets->get<Font>("pixel32"));
    _timeText->layout();
//    _healthText = TextLayout::allocWithText(health_msg, assets->get<Font>("pixel32"));
//    _healthText->layout();
    
    reset();
    
    // Acquire the scene built by the asset loader and resize it the scene
    _scene_UI = _assets->get<scene2::SceneNode>("game");
    
//    _scene_UI->addChild(_dirtThrowArc);
    _scene_UI->setContentSize(dimen);
    _scene_UI->doLayout(); // Repositions the HUD
    
    // get the win background scene when game is win
    _winBackground = _assets->get<scene2::SceneNode>("win");
    
    // get the lose background scene when game is lose
    _loseBackground = _assets->get<scene2::SceneNode>("lose");
    

    _backout = std::dynamic_pointer_cast<scene2::Button>(_assets->get<scene2::SceneNode>("game_back"));
    _backout->addListener([=](const std::string& name, bool down) {
        if (down) {
            CULog("quitting game");
            _quit = true;
        }
    });
    
    _dirtThrowButton = std::dynamic_pointer_cast<scene2::Button>(_assets->get<scene2::SceneNode>("game_throw"));
    
    _dirtThrowArc = std::make_shared<scene2::SceneNode>();
    
    _quit = false;
    addChild(_scene_UI);
    addChild(_winBackground);
    addChild(_loseBackground);
//    _loseBackground->setVisible(false);
//    _winBackground->setVisible(false);
    setActive(false);
    return true;
}

/**
 * Disposes of all (non-static) resources allocated to this mode.
 */
void GameScene::dispose() {
    if (_active) {
        removeAllChildren();
        _active = false;
        _dirtThrowButton = nullptr;
        _winBackground = nullptr;
        _loseBackground = nullptr;
    }
}

/**
 * Sets whether the scene is currently active
 *
 * This method should be used to toggle all the UI elements.  Buttons
 * should be activated when it is made active and deactivated when
 * it is not.
 *
 * @param value whether the scene is currently active
 */
void GameScene::setActive(bool value) {
    if (isActive() != value) {
        Scene2::setActive(value);
        if (value) {
            _quit = false;
            _backout->activate();
            _dirtThrowButton->activate();
        }
        else {
            _backout->deactivate();
            _dirtThrowButton->deactivate();
            // If any were pressed, reset them
            _backout->setDown(false);
            _dirtThrowButton->setDown(false);
        }
    }
}



#pragma mark -
#pragma mark Gameplay Handling

/**
 * Converts game state into a JSON value for sending over the network.
 * Only called by the host, as only the host transmits board states over the network.
 * This method contains any gameplay code that is not an OpenGL call.
 *
 * We need to update this method to constantly talk to the server.
 *
 * @param timestep  The amount of time (in seconds) since the last frame
 */
void GameScene::update(float timestep) {
    _dirtThrowInput.update();
    Vec2 screenPos = _dirtThrowInput.getPosition();
    Vec3 convertedWorldPos = screenToWorldCoords(screenPos);
    Vec2 worldPos = Vec2(convertedWorldPos.x, convertedWorldPos.y);
    
    _gameController->update(timestep, worldPos, _dirtThrowInput, _dirtThrowButton, _dirtThrowArc);
    
    int currBarIdx = 0;
    for(int id = 1; id <= 4; id++) {
        auto player = _gameController->getPlayer(id);
        if (player == nullptr) continue;
        float numWindowPanes = _gameController->getPlayerWindow(id)->getNHorizontal() * _gameController->getPlayerWindow(id)->getNVertical();
        auto progress = (numWindowPanes - _gameController->getPlayerWindow(id)->getTotalDirt()) / numWindowPanes;
        _player_bars[currBarIdx]->setProgress(progress);
        currBarIdx += 1;
    }

    _timeText->setText(strtool::format("Time %d", _gameController->getTime()));
        

    _timeText->layout();
        
    // Update the dirt display
    _dirtText->setText(strtool::format("%d", _gameController->getCurDirtAmount()));
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
    // CULog("current board: %d", _curBoard);
    
    
    Vec3 idk = Vec3(getCamera()->getPosition().x, _gameController->getPlayer(_gameController->getId())->getPosition().y, 1);
    getCamera()->setPosition(idk);
    getCamera()->update();
    batch->begin(getCamera()->getCombined());
    _scene_UI->setPosition(idk-getSize().operator Vec2()/2);
    
//    batch->draw(_background,Rect(Vec2::ZERO));
    batch->draw(_background,(idk-getSize().operator Vec2()/2)-Vec2(0,idk.y-400));
    batch->draw(_parallax, (idk-getSize().operator Vec2()/2)-Vec2(0,idk.y));
    

    _gameController->draw(batch);
    
    batch->setColor(Color4::BLACK);
//    batch->drawText(_timeText, Vec2(getSize().width - 10 - _timeText->getBounds().size.width, getSize().height - _timeText->getBounds().size.height));
//    batch->drawText(_healthText, Vec2(10, getSize().height - _healthText->getBounds().size.height));
    
    batch->drawText(_timeText, Vec2(getCamera()->getPosition().x+ 412, getCamera()->getPosition().y + 300));
    
//    batch->drawText(_timeText, Vec2(getSize().width - 10 - _timeText->getBounds().size.width, getSize().height - _timeText->getBounds().size.height));
    
    //set bucket texture location
    Affine2 bucketTrans = Affine2();
    Vec2 bOrigin(_fullBucket->getWidth()/2,_fullBucket->getHeight()/2);
    float bucketScaleFactor = std::min(((float)getSize().getIWidth() / (float)_fullBucket->getWidth()) /2, ((float)getSize().getIHeight() / (float)_fullBucket->getHeight() /2));
    bucketTrans.scale(bucketScaleFactor);
    
//    Vec2 bucketLocation(getSize().width - ((float)_fullBucket->getWidth() * bucketScaleFactor/2 + getCamera()->getPosition().x),
//                        getSize().height + (float)_fullBucket->getHeight() * bucketScaleFactor/2 + getCamera()->getPosition().y + 100);
    
    Vec2 bucketLocation(getCamera()->getPosition().x+ 500, getCamera()->getPosition().y + 120);
    bucketTrans.translate(bucketLocation);
    
    // draw different bucket based on dirt amount
    if (_gameController->getCurDirtAmount() == 0) {
        batch->draw(_emptyBucket, bOrigin, bucketTrans);
    } else {
        batch->draw(_fullBucket, bOrigin, bucketTrans);
    }
    
    // draw dirt amount text, centered at bucket
    Affine2 dirtTextTrans = Affine2();
    dirtTextTrans.scale(1.2);
    dirtTextTrans.translate(bucketLocation.x - _dirtText->getBounds().size.width/2,
                            bucketLocation.y);
    batch->setColor(Color4::BLACK);
    batch->drawText(_dirtText, dirtTextTrans);
    batch->setColor(Color4::WHITE);
    
    if (_gameController->getCurBoard() != 0) {
        _dirtThrowButton->setVisible(true);
        _dirtThrowButton->activate();
        _dirtThrowButton->setDown(false);
    }
    else {
        _dirtThrowButton->setVisible(false);
        _dirtThrowButton->deactivate();
    }
    _scene_UI->render(batch);
//    std::cout<<"arc: "<<_dirtThrowArc->getPosition().x<<", "<<_dirtThrowArc->getPosition().y<<"\n";
//    std::cout<<"button: "<<_dirtThrowButton->getPosition().x<<", "<<_dirtThrowButton->getPosition().y<<"\n";
    
    if (_gameController->isGameWin()) {
        _winBackground->setPosition(idk-getSize().operator Vec2()/2);
        _winBackground->setVisible(true);
        _winBackground->render(batch);
    } else if (_gameController->isGameOver() && !_gameController->isGameWin()) {
        _loseBackground->setPosition(idk-getSize().operator Vec2()/2);
        _loseBackground->setVisible(true);
        _loseBackground->render(batch);
    }
    
    int offset_ct = 0;
    for (int id = 1; id <= 4; id++) {
        auto player = _gameController->getPlayer(id);
        if (player == nullptr) continue;
        int barIdx = _char_to_barIdx[player->getChar()];
        // CULog("character: %a", player->getChar().c_str());
        _player_bars[barIdx]->setPositionX(getSize().width - _gameController->getPlayerWindow(_gameController->getId())->sideGap + (offset_ct + 2) * 50);
        _player_bars[barIdx]->setVisible(true);
        Affine2 profileTrans = Affine2();
        profileTrans.scale(0.2);
        profileTrans.translate((idk - getSize().operator Vec2() / 2) + _player_bars[barIdx]->getPosition());
        profileTrans.translate(0, _player_bars[barIdx]->getHeight() / -2);
        batch->draw(player->getProfileTexture(), player->getProfileTexture()->getSize().operator Vec2() / 2, profileTrans);
        offset_ct += 1;
    }
    
    batch->end();
}

void GameScene::renderCountdown(std::shared_ptr<cugl::SpriteBatch> batch) {
    Affine2 countdown1Trans = Affine2();
    Vec2 countdown1Origin(_countdown1->getWidth()/2,_countdown1->getHeight()/2);
    float countdown1ScaleFactor = std::min(((float)getSize().getIWidth() / (float)_countdown1->getWidth()) /2, ((float)getSize().getIHeight() / (float)_countdown1->getHeight() /2));
    countdown1Trans.scale(countdown1ScaleFactor);
    
    Vec2 countdown1Location(getSize().width/2,
                        getSize().height - 10*_countdownFrame);
    countdown1Trans.translate(countdown1Location);
    
    batch->draw(_countdown1, countdown1Origin, countdown1Trans);
}

