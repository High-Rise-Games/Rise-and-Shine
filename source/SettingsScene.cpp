//
//  SettingsScene.cpp
//  Shine
//
//  Created by Troy Moslemi on 5/12/24.
//

#include "SettingsScene.h"
#include <cugl/cugl.h>
#include <iostream>
#include <sstream>

/** Regardless of logo, lock the height to this */
#define SCENE_HEIGHT  720


using namespace cugl;
using namespace std;

/**
 * Initializes the controller contents
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
bool SettingsScene::init(const std::shared_ptr<cugl::AssetManager>& assets) {
    // Initialize the scene to a locked width
    Size dimen = Application::get()->getDisplaySize();
    dimen *= SCENE_HEIGHT/dimen.height;
    if (assets == nullptr) {
        return false;
    } else if (!Scene2::init(dimen)) {
        return false;
    }
    
    _assets = assets;
    // Acquire the scene built by the asset loader and resize it the scene
    _assets->loadDirectory(assets->get<JsonValue>("settings"));
    
    std::shared_ptr<scene2::SceneNode> scene = assets->get<scene2::SceneNode>("settingsUI");
    scene->setContentSize(dimen);
    scene->doLayout(); // Repositions the HUD
    


    addChild(scene);
    setActive(false);
    return true;
}

/**
 * Disposes of all (non-static) resources allocated to this mode.
 */
void SettingsScene::dispose() {
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
void SettingsScene::setActive(bool value) {
    if (isActive() != value) {
        Scene2::setActive(value);
    }
}
