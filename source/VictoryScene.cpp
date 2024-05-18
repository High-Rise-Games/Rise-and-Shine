//
//  VictoryScene.cpp
//  Shine
//

#include <cugl/cugl.h>
#include "VictoryScene.h"

using namespace cugl;
using namespace std;

#pragma mark -
#pragma mark Victory Layout

/** Regardless of logo, lock the height to this */
#define SCENE_HEIGHT 720
#define SCENE_WIDTH  1280

#pragma mark -
#pragma mark Constructors

/**
 * Initializes the controller contents, and starts the game
 *
 * In previous labs, this method "started" the scene.  But in this
 * case, we only use to initialize the scene user interface.  We
 * do not activate the user interface yet, as an active user
 * interface will still receive input EVEN WHEN IT IS HIDDEN.
 *
 * That is why we have the method {@link #setActive}.
 *
 * @param assets    The (loaded) assets for this game mode
 *
 * @return true if the controller is initialized properly, false otherwise.
 */
bool VictoryScene::init(const std::shared_ptr<cugl::AssetManager>& assets) {
    // Initialize the scene to a locked width
    // Size dimen = Application::get()->getDisplaySize();

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
    _assets = assets;
    
    _building = assets->get<Texture>("victory_building");
    _winnerRedText = _assets->get<Texture>("redwinnertext");
    _winnerBlueText = _assets->get<Texture>("bluewinnertext");
    _winnerYellowText = _assets->get<Texture>("yellowwinnertext");
    _winnerGreenText = _assets->get<Texture>("redwinnertext");
    _winnerMushroom = SpriteSheet::alloc(_assets->get<Texture>("redwinner"),  1, 3, 3);
    _winnerMushroom->setFrame(0);
    _winnerFrog = SpriteSheet::alloc(_assets->get<Texture>("bluewinner"),  1, 3, 3);
    _winnerFrog->setFrame(0);
    _winnerFlower = SpriteSheet::alloc(_assets->get<Texture>("yellowwinner"),  1, 3, 3);
    _winnerFlower->setFrame(0);
    _winnerChameleon = SpriteSheet::alloc(_assets->get<Texture>("greenwinner"),  1, 3, 3);
    _winnerChameleon->setFrame(0);
    _loserMushroom = SpriteSheet::alloc(_assets->get<Texture>("redloser"), 1, 3, 3);
    _loserMushroom->setFrame(0);
    _loserFrog = SpriteSheet::alloc(_assets->get<Texture>("blueloser"), 1, 3, 3);
    _loserFrog->setFrame(0);
    _loserFlower = SpriteSheet::alloc(_assets->get<Texture>("yellowloser"), 1, 3, 3);
    _loserFlower->setFrame(0);
    _loserChameleon = SpriteSheet::alloc(_assets->get<Texture>("greenloser"), 1, 3, 3);
    _loserChameleon->setFrame(0);
    animFrameCounter = 0;
    loseFrameCounter = 0;
    
    // Acquire the scene built by the asset loader and resize it the scene
    assets->loadDirectory(assets->get<JsonValue>("victory"));
    _scene = assets->get<scene2::SceneNode>("victory");
    _scene->setContentSize(dimen);
    _scene->doLayout(); // Repositions the HUD
    _backbutton = std::dynamic_pointer_cast<scene2::Button>(_assets->get<scene2::SceneNode>("victory_buttons_backtohome"));
    _backbutton->addListener([=](const std::string& name, bool down) {
        if (down) {
            _quit = true;
        }
    });
    addChild(_scene);
    setActive(false);
    return true;
}

/**
 * Disposes of all (non-static) resources allocated to this mode.
 */
void VictoryScene::dispose() {
    if (_active) {
        removeAllChildren();
        _active = false;
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
void VictoryScene::setActive(bool value) {
    if (isActive() != value) {
        Scene2::setActive(value);
        if (value) {
            _quit = false;
            _backbutton->activate();
        } else {
            _otherChars.clear();
            _backbutton->deactivate();
            _backbutton->setDown(false);
        }
    }
}

/**
 * Set the winner and other player characters in the game
 */
void VictoryScene::setCharacters(std::shared_ptr<GameplayController> gameplay) {
    vector<bool> win = gameplay->getWin();
    for (int id = 1; id <= 4; id++) {
        auto player = gameplay->getPlayer(id);
        if (player == nullptr) continue;
        if (win[id - 1]) {
            _winnerChar = player->getChar();
        } else {
            _otherChars.push_back(player->getChar());
        }
    }
}

void VictoryScene::render(const std::shared_ptr<cugl::SpriteBatch>& batch) {
    int step = 40 / 4;
    int loseStep = 30/3;
    if (loseFrameCounter < 30) {
        if (loseFrameCounter % loseStep == 0) {
            _winnerMushroom->setFrame((int)loseFrameCounter / loseStep);
            _winnerFrog->setFrame((int)loseFrameCounter / loseStep);
            _winnerFlower->setFrame((int)loseFrameCounter / loseStep);
            _winnerChameleon->setFrame((int)loseFrameCounter / loseStep);
        }
        loseFrameCounter += 1;
//        CULog("victory Frame %d", animFrameCounter);
    }
    else {
        loseFrameCounter = 0;
    }
    if (loseFrameCounter < 30) {
        if (loseFrameCounter % step == 0) {
            _loserMushroom->setFrame((int)loseFrameCounter / loseStep);
            _loserFrog->setFrame((int)loseFrameCounter / loseStep);
            _loserFlower->setFrame((int)loseFrameCounter / loseStep);
            _loserChameleon->setFrame((int)loseFrameCounter / loseStep);
        }
        loseFrameCounter += 1;
    } else {
        loseFrameCounter = 0;
    }
    batch->begin(getCamera()->getCombined());
    _scene->render(batch);
    Affine2 buildingTrans;
    buildingTrans.translate(_building->getSize().width / -2.0, _building->getSize().height / -2.0);
    buildingTrans.translate(getSize().width / 2, 0);
    batch->draw(_building, Vec2(), buildingTrans);
    std::shared_ptr<cugl::Texture> textTexture;
    std::shared_ptr<cugl::SpriteSheet> winnerTexture;
    if (_winnerChar == "Chameleon") {
        winnerTexture = _winnerChameleon;
        textTexture = _winnerGreenText;
    } else if (_winnerChar == "Flower") {
        winnerTexture = _winnerFlower;
        textTexture = _winnerYellowText;
    } else if (_winnerChar == "Frog") {
        winnerTexture = _winnerFrog;
        textTexture = _winnerBlueText;
    } else {
        winnerTexture = _winnerMushroom;
        textTexture = _winnerRedText;
    }
    Affine2 winnerTrans;
    winnerTrans.translate(winnerTexture->getFrameSize().width * -0.5, winnerTexture->getFrameSize().height * -0.5);
    winnerTrans.scale(0.15);
    winnerTrans.translate(getSize().width * 0.55, _building->getSize().height);
    winnerTexture->draw(batch, winnerTrans);
    
    Affine2 textTrans;
    textTrans.translate(textTexture->getSize().width * -0.5, textTexture->getSize().height * -0.5);
    textTrans.scale(0.75);
    textTrans.translate(getSize().width * 0.55, getSize().height * 0.8);
    batch->draw(textTexture, Vec2(), textTrans);
    
    for (int i = 0; i < _otherChars.size(); i++) {
        string loser = _otherChars[i];
        std::shared_ptr<cugl::SpriteSheet> loserTexture;
        if (loser == "Mushroom") {
            loserTexture = _loserMushroom;
        } else if (loser == "Flower") {
            loserTexture = _loserFlower;
        } else if (loser == "Frog") {
            loserTexture = _loserFrog;
        } else {
            loserTexture = _loserChameleon;
        }
        float ratio;
        if (i == 0) {
            ratio = 0.4;
        } else if (i == 1) {
            ratio = 0.7;
        } else {
            ratio = 0.25;
        }
        Affine2 loserTrans;
        loserTrans.translate(loserTexture->getFrameSize().width * -0.5, loserTexture->getFrameSize().height * -0.5);
        loserTrans.scale(0.15);
        loserTrans.translate(getSize().width * ratio, _building->getSize().height * 0.75);
        loserTexture->draw(batch, loserTrans);
    }
    batch->end();
}
