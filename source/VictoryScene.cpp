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
    _winnerMushroom = assets->get<Texture>("char_mushroom");
    _winnerFrog = assets->get<Texture>("char_frog");
    _winnerChameleon = assets->get<Texture>("char_chameleon");
    _winnerFlower = assets->get<Texture>("char_flower");
    _loserMushroom = assets->get<Texture>("char_mushroom");
    _loserFrog = assets->get<Texture>("char_frog");
    _loserChameleon = assets->get<Texture>("char_chameleon");
    _loserFlower = assets->get<Texture>("char_flower");
    
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
    batch->begin(getCamera()->getCombined());
    _scene->render(batch);
    Affine2 buildingTrans;
    buildingTrans.translate(_building->getSize().width / -2.0, _building->getSize().height / -2.0);
    buildingTrans.translate(getSize().width / 2, 0);
    batch->draw(_building, Vec2(), buildingTrans);
    std::shared_ptr<Texture> winnerTexture;
    if (_winnerChar == "Mushroom") {
        winnerTexture = _winnerMushroom;
    } else if (_winnerChar == "Flower") {
        winnerTexture = _winnerFlower;
    } else if (_winnerChar == "Frog") {
        winnerTexture = _winnerFrog;
    } else {
        winnerTexture = _winnerChameleon;
    }
    Affine2 winnerTrans;
    winnerTrans.scale(0.5);
    winnerTrans.translate(winnerTexture->getSize().width * -0.5, _building->getSize().height / 3.0);
    winnerTrans.translate(getSize().width * 0.6, 0);
    batch->draw(winnerTexture, Vec2(), winnerTrans);
    for (int i = 0; i < _otherChars.size(); i++) {
        string loser = _otherChars[i];
        std::shared_ptr<Texture> loserTexture;
        if (loser == "Mushroom") {
            loserTexture = _loserMushroom;
        } else if (loser == "Flower") {
            loserTexture = _loserMushroom;
        } else if (loser == "Frog") {
            loserTexture = _loserMushroom;
        } else {
            loserTexture = _loserMushroom;
        }
        float ratio;
        if (i == 0) {
            ratio = 0.45;
        } else if (i == 1) {
            ratio = 0.7;
        } else {
            ratio = 0.3;
        }
        Affine2 loserTrans;
        loserTrans.scale(0.3);
        loserTrans.translate(loserTexture->getSize().width * -0.3, _building->getSize().height / 3.0);
        loserTrans.translate(getSize().width * ratio, 0);
        batch->draw(loserTexture, Vec2(), loserTrans);
    }
    batch->end();
}
