//
//  LevelScene.cpp
//  Shine
//

#include <cugl/cugl.h>
#include <iostream>
#include <sstream>

#include "LevelScene.h"

using namespace cugl;
using namespace std;

#pragma mark -
#pragma mark Level Layout

/** Regardless of logo, lock the height to this */
#define SCENE_HEIGHT  720
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
bool LevelScene::init(const std::shared_ptr<cugl::AssetManager>& assets) {
    // Initialize the scene to a locked width


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
    
    // Acquire the scene built by the asset loader and resize it the scene
    _assets = assets;
    
    std::shared_ptr<scene2::SceneNode> scene = assets->get<scene2::SceneNode>("level");
    scene->setContentSize(dimen);
    scene->doLayout(); // Repositions the HUD
    _choice = Choice::NONE;
    _selectedlevel = -1;
    _backbutton = std::dynamic_pointer_cast<scene2::Button>(_assets->get<scene2::SceneNode>("level_back"));
    _nextbutton = std::dynamic_pointer_cast<scene2::Button>(_assets->get<scene2::SceneNode>("level_next"));
    _levelbuttons.push_back(std::dynamic_pointer_cast<scene2::Button>(_assets->get<scene2::SceneNode>("level_level1")));
    _levelbuttons.push_back(std::dynamic_pointer_cast<scene2::Button>(_assets->get<scene2::SceneNode>("level_level2")));
    _levelbuttons.push_back(std::dynamic_pointer_cast<scene2::Button>(_assets->get<scene2::SceneNode>("level_level3")));
    _levelbuttons.push_back(std::dynamic_pointer_cast<scene2::Button>(_assets->get<scene2::SceneNode>("level_level4")));
    _highlightedlevels.push_back(_assets->get<scene2::SceneNode>("level_level1h"));
    _highlightedlevels.push_back(_assets->get<scene2::SceneNode>("level_level2h"));
    _highlightedlevels.push_back(_assets->get<scene2::SceneNode>("level_level3h"));
    _highlightedlevels.push_back(_assets->get<scene2::SceneNode>("level_level4h"));
    
    // Program the buttons
    _backbutton->addListener([this](const std::string& name, bool down) {
        if (down) {
            _audioController->playBackPress();
            _choice = BACK;
        }
    });
    _nextbutton->addListener([this](const std::string& name, bool down) {
        if (down) {
            _audioController->playGoPress();
            _choice = NEXT;
        }
    });
    
    for (int i = 0; i < _levelbuttons.size(); ++i) {
        _levelbuttons[i]->addListener([this, i](const std::string& name, bool down) {
            if (down) {
                _audioController->playMovePress();
                this->_levelbuttons[i]->setVisible(false);
                this->_highlightedlevels[i]->setVisible(true);
                if (_selectedlevel != -1 && _selectedlevel != i) {
                    this->_levelbuttons[_selectedlevel]->setVisible(true);
                    this->_highlightedlevels[_selectedlevel]->setVisible(false);
                }
                _selectedlevel = i;
            }
        });
    }
    
    for (const auto& item : _highlightedlevels) {
        item->setVisible(false);
    }

    addChild(scene);
    setActive(false);
    return true;
}

/**
 * Disposes of all (non-static) resources allocated to this mode.
 */
void LevelScene::dispose() {
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
void LevelScene::setActive(bool value) {
    if (isActive() != value) {
        Scene2::setActive(value);
        if (value) {
            _choice = NONE;
            _selectedlevel = -1;
            _backbutton->activate();
            _nextbutton->activate();
            for (const auto& button: _levelbuttons) {
                button->activate();
                button->setVisible(true);
            }
            for (const auto& level: _highlightedlevels) {
                level->setVisible(false);
            }
            _levelbuttons[0]->setVisible(false);
            _highlightedlevels[0]->setVisible(true);
            _selectedlevel = 0;
        } else {
            _backbutton->deactivate();
            _nextbutton->deactivate();
            // If any were pressed, reset them
            _backbutton->setDown(false);
            _nextbutton->setDown(false);
            for (const auto& button: _levelbuttons) {
                button->deactivate();
                button->setDown(false);
            }
        }
    }
}

