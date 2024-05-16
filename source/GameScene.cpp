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
#define SCENE_WIDTH  1280

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
    
    
    _scroll = 0;
    
    
    // Get the current display size of the device
    Size displaySize = Application::get()->getDisplaySize();

    // Calculate the device's aspect ratio
    float aspectRatio = displaySize.width / displaySize.height;


    // Create the new dimensions for the scene
    Size dimen = Size(SCENE_WIDTH, SCENE_WIDTH / aspectRatio);


    if (assets == nullptr) {
        return false;
    } else if (!Scene2::init(dimen)) {
        return false;
    }
    
    // Start up the input handler
    _assets = assets;
    _dirtThrowInput.init();
    
    // Get the background image and constant values
    _background = _assets->get<Texture>("level4Background");
    _parallax = _assets->get<Texture>("level4Parallax");
    _background->setWrapS(GL_CLAMP_TO_EDGE);
    _background->setWrapT(GL_CLAMP_TO_EDGE);
    _constants = _assets->get<JsonValue>("constants");

    _textBubble = _assets->get<Texture>("text_bubble");
    _mushroomPoint = _assets->get<Texture>("mushroom_point");

    // victory screen
//    auto _victoryJson = _assets->get<JsonValue>("victory");
    _victory_UI = assets->get<scene2::SceneNode>("victory");
    _victoryBackout = std::dynamic_pointer_cast<scene2::Button>(_victory_UI->getChildByName("buttons")->getChildByName("backtohome"));
    _victoryBackout->addListener([=](const std::string& name, bool down) {
        if (down) {
            _quit = true;
        }
    });
    _victory_UI->setContentSize(dimen);
    _victory_UI->doLayout(); // This rearranges the children to fit the screen
    addChild(_victory_UI);

    
    
    // Initialize dirt bucket
    setEmptyBucket(assets->get<Texture>("bucketempty"));
    setFullBucket(assets->get<Texture>("bucketfull"));
    
//    _countdown1 =assets->get<Texture>("countdown1");
    


    // Create and layout the dirt amount
    std::string dirt_msg = "0";
    _dirtText = TextLayout::allocWithText(dirt_msg, assets->get<Font>("pixel32"));
    _dirtText->layout();


    // Create and layout the health meter
    std::string health_msg = "Health";
    std::string time_msg = "Time";
    _timeText = TextLayout::allocWithText(time_msg, assets->get<Font>("pixel32"));
    _timeText->layout();

    reset();

    // Acquire the scene built by the asset loader and resize it the scene
    assets->loadDirectory(assets->get<JsonValue>("gamescene"));
    _gameplay_elem = _assets->get<scene2::SceneNode>("game");

    _gameplay_elem->setContentSize(dimen);
    _gameplay_elem->doLayout();
    
    // progress bars for player
    auto greenBar = std::dynamic_pointer_cast<scene2::ProgressBar>(assets->get<scene2::SceneNode>("game_greenbar"));
    auto blueBar = std::dynamic_pointer_cast<scene2::ProgressBar>(assets->get<scene2::SceneNode>("game_bluebar"));
    auto redBar = std::dynamic_pointer_cast<scene2::ProgressBar>(assets->get<scene2::SceneNode>("game_redbar"));
    auto yellowBar = std::dynamic_pointer_cast<scene2::ProgressBar>(assets->get<scene2::SceneNode>("game_yellowbar"));

    _player_bars = { redBar, greenBar, blueBar, yellowBar };
    for (auto currBar : _player_bars) {
        currBar->setAngle(1.5708);
        currBar->setScale(2);
        currBar->setVisible(false);
    }
    _char_to_barIdx["Mushroom"] = 0;
    _char_to_barIdx["Chameleon"] = 1;
    _char_to_barIdx["Frog"] = 2;
    _char_to_barIdx["Flower"] = 3;
    
    _projectile_line = std::dynamic_pointer_cast<scene2::PolygonNode>(assets->get<scene2::SceneNode>("game_projectile_line"));
    _projectile_circle = _assets->get<Texture>("white_circle");

    _scene_UI = _assets->get<scene2::SceneNode>("gamescene");
    _scene_UI->setContentSize(dimen);
    _scene_UI->doLayout(); // Repositions the HUD
    
//    _scene_UI->getChildByName("tutorial")->getChildByName("mushroomtalk")->setVisible(false);
//    _scene_UI->getChildByName("tutorial")->getChildByName("dialog")->getChildByName("textbox")->setVisible(false);
//    
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
    
    _dirtThrowArc = _assets->get<scene2::SceneNode>("game_greenarc");
    
    _quit = false;
    addChild(_scene_UI);
    addChild(_gameplay_elem);
    addChild(_winBackground);
    addChild(_loseBackground);
//    _loseBackground->setVisible(false);
//    _winBackground->setVisible(false);
    setActive(false);
    return true;
}

void GameScene::loadBackgroundTextures() {
    _background = _assets->get<Texture>(_gameController->background_string);
    _parallax   = _assets->get<Texture>(_gameController->parallax_string);
}

/**
 * Disposes of all (non-static) resources allocated to this mode.
 */
void GameScene::dispose() {
    if (_active) {
        removeAllChildren();
        _active = false;
        _dirtThrowButton = nullptr;
        _dirtThrowArc = nullptr;
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
            for (auto bar : _player_bars) {
                bar->setVisible(false);
            }
        }
        else {
            _backout->deactivate();
            _dirtThrowButton->deactivate();
            // If any were pressed, reset them
            _backout->setDown(false);
            _dirtThrowButton->setDown(false);
            for (auto bar : _player_bars) {
                bar->setVisible(false);
            }
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
    
    _scene_UI->getChild(0)->getChild(0)->getChild(0)->getChild(1)->getChild<scene2::Label>(1)->setText(std::to_string(_gameController->getTime()));
    
    _scene_UI->getChildByName("Leftgroup")->getChildByName("BucketUI")->getChildByName("BucketUI")->getChildByName<scene2::Label>("number")->setText(std::to_string(_gameController->getCurDirtAmount()));
    
    for (auto bar : _player_bars) {
        bar->setVisible(false);
    }
    
    for(int id = 1; id <= 4; id++) {
        auto player = _gameController->getPlayer(id);
        if (player == nullptr) continue;
        // float numWindowPanes = _gameController->getPlayerWindow(id)->getNHorizontal() * _gameController->getPlayerWindow(id)->getNVertical();
        // auto progress = (numWindowPanes - _gameController->getPlayerWindow(id)->getTotalDirt()) / numWindowPanes;
        _player_bars[_char_to_barIdx[player->getChar()]]->setProgress(_gameController->getPlayerProgress(id));
        _player_bars[_char_to_barIdx[player->getChar()]]->setVisible(true);
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
    

    Vec3 cameraPos;
    if (!_gameController->_countDown) {
        cameraPos = Vec3(getCamera()->getPosition().x, _gameController->getPlayerWindow(_gameController->_id)->_windowHeight * _gameController->getPlayerWindow(_gameController->_id)->_nVertical + _scroll, 1);
        if (cameraPos.y > _gameController->getPlayer(_gameController->getId())->getPosition().y) {
            _scroll=_scroll-15;
        } else {
            _gameController->_countDown = true;
        }
    } else {
        cameraPos = Vec3(getCamera()->getPosition().x, _gameController->getPlayer(_gameController->getId())->getPosition().y, 1);
    }
    
    getCamera()->setPosition(cameraPos);
    getCamera()->update();
    batch->begin(getCamera()->getCombined());
    _scene_UI->setPosition(cameraPos -getSize().operator Vec2()/2);
    _gameplay_elem->setPosition(cameraPos - getSize().operator Vec2() / 2);
    
//    batch->draw(_background,Rect(Vec2::ZERO));
    //batch->draw(_background, (cameraPos - getSize().operator Vec2() / 2) - Vec2(0, cameraPos.y - 400)); // revert next line to this to disable parallax
    batch->draw(_background,(cameraPos -getSize().operator Vec2()/2)-Vec2(0, cameraPos.y/3-0));
    batch->draw(_parallax, (cameraPos -getSize().operator Vec2()/2)-Vec2(0, cameraPos.y));
    

        
    
    _gameController->draw(batch);
    
    
    
    _gameController->drawCountdown(batch, getCamera()->getPosition(), getSize());
    
    
    
    
    
    batch->setColor(Color4::WHITE);
    
    if (_gameController->getCurBoard() != 0) {
        _dirtThrowButton->setVisible(true);
        _dirtThrowButton->activate();
        _dirtThrowButton->setDown(false);
        _dirtThrowArc->setVisible(true);

        Path2 dirtPath = _gameController->getDirtThrowVector();
        if (dirtPath.getVertices().size() > 0 && _gameController->isDirtSelected()) {
            _projectile_line->setVisible(true);

            auto p1 = dirtPath.getVertices().front();
            auto p2 = dirtPath.getVertices().back();
            
            _projectile_line->setPolygon(Rect(0, 0, (p2 - p1).length(), 10));
            _projectile_line->setPositionX(p1.x);
            _projectile_line->setPositionY(getSize().height / 2);
            double deltaX = p2.x - p1.x;
            double deltaY = p2.y - p1.y;

            // Calculate the angle using atan2
            double angle = std::atan2(deltaY, deltaX);

            // Ensure the angle is between 0 and 2*pi (radians)
            if (angle < 0)
                angle += 2 * M_PI;
            _projectile_line->setAngle(angle);
            Affine2 trans = Affine2();
            trans.translate(p2);
            batch->draw(_projectile_circle, _projectile_circle->getSize().operator Vec2() / 2, trans);
        }
        else {
            _projectile_line->setVisible(false);
        }
    }
    else {
        _dirtThrowButton->setVisible(false);
        _dirtThrowButton->deactivate();
        _dirtThrowArc->setVisible(false);
    }
    _scene_UI->render(batch);
    _gameplay_elem->render(batch);
    
    int offset_ct = 0;
    for (int id = 1; id <= 4; id++) {
        auto player = _gameController->getPlayer(id);
        if (player == nullptr) continue;
        int barIdx = _char_to_barIdx[player->getChar()];
//         CULog("character t: %s", player->getChar().c_str());
        _player_bars[barIdx]->setPositionX(getSize().width - _gameController->getPlayerWindow(_gameController->getId())->sideGap + (offset_ct + 2) * 60);
        _player_bars[barIdx]->setVisible(true);

        Affine2 medalTrans = Affine2();
        medalTrans.scale(0.4);
        medalTrans.translate((cameraPos - getSize().operator Vec2() / 2) + _player_bars[barIdx]->getPosition());
        // translate medal up to top of bar
        // medalTrans.translate(0, _player_bars[barIdx]->getHeight() / 2);
        batch->draw(player->getMedalTexture(), player->getMedalTexture()->getSize().operator Vec2() / 2, medalTrans);

        Affine2 profileTrans = Affine2();
        profileTrans.scale(0.5);
        profileTrans.translate((cameraPos - getSize().operator Vec2() / 2) + _player_bars[barIdx]->getPosition());
        // translate profile down to bottom of bar
        profileTrans.translate(0, _player_bars[barIdx]->getHeight() * -1);
        // translate profile to top of filled bar based on current progress
        profileTrans.translate(0, _player_bars[barIdx]->getHeight() * _player_bars[barIdx]->getProgress());
        batch->draw(player->getProfileTexture(), player->getProfileTexture()->getSize().operator Vec2() / 2, profileTrans);

        offset_ct += 1;
    }
    
    if (_gameController->isGameWin()) {
//        std::shared_ptr<Player> p = _gameController->getPlayer(_id);
        int barIdx = 0;
//        CULog("%s", p->getChar().c_str());
        _victory_UI->setPosition(cameraPos -getSize().operator Vec2()/2);
        _victory_UI->getChildByName("victorybg1")->setVisible(true);
        _victory_UI->getChildByName("victorybg2")->setVisible(true);
        switch (barIdx) {
            case 0:
                _victory_UI->getChildByName("redwinner")->setVisible(true);
                break;
            case 1:
                _victory_UI->getChildByName("greenwinner")->setVisible(true);
                break;
            case 2:
                _victory_UI->getChildByName("bluewinner")->setVisible(true);
                break;
            case 3:
                _victory_UI->getChildByName("yellowwinner")->setVisible(true);
                break;
            default:
                _victory_UI->getChildByName("yellowwinner")->setVisible(true);
                break;
        }
        _victoryBackout->activate();
        _backout->deactivate();
        _victory_UI->render(batch);
    } else if (_gameController->isGameOver() && !_gameController->isGameWin()) {
        _loseBackground->setPosition(cameraPos -getSize().operator Vec2()/2);
        _loseBackground->setVisible(true);
        _loseBackground->render(batch);
    }
    
    batch->end();
}

void GameScene::drawPrompt(std::string text, const std::shared_ptr<cugl::SpriteBatch>& batch, cugl::Vec2 location) {

    Affine2 bubbleTrans = Affine2();
    Vec2 bOrigin(_textBubble->getWidth() / 2, _textBubble->getHeight() / 2);
    Vec2 bubbleLocation(location);
    bubbleTrans.translate(bubbleLocation);
    bubbleTrans.translate(Vec2(370, -450));
    bubbleTrans.scale(0.2);

    _textOnBubble = TextLayout::allocWithTextWidth("efhoiewhfioewhfiowehfioiwehfeiwofheowifhewifhewifihhiewfihhieowihfeihfewhiowefhiihfewhiofwehiofweihfewhioefwhiofewihhiefwhiofewhiofeihfewhifewhiohifhiefwhiofeiwhohioefwihoefwihoefwiohfew", _assets->get<Font>("pixel32"), _textBubble->getWidth() / 2);
    _textOnBubble->setSpacing(1.5);
    _textOnBubble->layout();

    Affine2 mushroomPointTrans = Affine2();
    Vec2 mOrigin(_mushroomPoint->getWidth() / 2, _mushroomPoint->getHeight() / 2);
    Vec2 mushroomLocation(location);
    mushroomPointTrans.translate(mushroomLocation);
    mushroomPointTrans.translate(Vec2(370, -450));
    mushroomPointTrans.scale(0.2);


    batch->draw(_textBubble, bOrigin, bubbleTrans);
    batch->draw(_mushroomPoint, mOrigin, bubbleTrans);
    batch->setColor(Color4::BLACK);
    batch->drawText(_textOnBubble, Vec2(bubbleTrans.getTranslation().x - _textBubble->getWidth() / 2, bubbleTrans.getTranslation().y + _textBubble->getHeight() / 6));

}
