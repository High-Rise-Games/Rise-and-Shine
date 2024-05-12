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
    Size dimen = Application::get()->getDisplaySize();
    dimen *= SCENE_HEIGHT/dimen.height;
    
    if (assets == nullptr) {
        return false;
    } else if (!Scene2::init(dimen)) {
        return false;
    }
    
    // Acquire the scene built by the asset loader and resize it the scene
    _assets = assets;
    _camera = std::dynamic_pointer_cast<OrthographicCamera>(getCamera());
    _input = std::make_shared<DirtThrowInputController>();
    _input->init();
    
    assets->loadDirectory(assets->get<JsonValue>("level"));
    _scene = assets->get<scene2::SceneNode>("level");
    dimen = _scene->getContentSize() * dimen.height / _scene->getContentSize().height;
    _scene->setContentSize(dimen);
    _scene->doLayout(); // Repositions the HUD
    
    _scene_ui = assets->get<scene2::SceneNode>("levelui");
    _scene_ui->setContentSize(Application::get()->getDisplaySize() * SCENE_HEIGHT/Application::get()->getDisplaySize().height);
    _scene_ui->doLayout();
    
    _choice = Choice::NONE;
    _selectedlevel = -1;
    _backbutton = std::dynamic_pointer_cast<scene2::Button>(_assets->get<scene2::SceneNode>("levelui_back"));
    _nextbutton = std::dynamic_pointer_cast<scene2::Button>(_assets->get<scene2::SceneNode>("levelui_next"));
    _levelbuttons.push_back(std::dynamic_pointer_cast<scene2::Button>(_assets->get<scene2::SceneNode>("level_level1")));
    _levelbuttons.push_back(std::dynamic_pointer_cast<scene2::Button>(_assets->get<scene2::SceneNode>("level_level2")));
    _levelbuttons.push_back(std::dynamic_pointer_cast<scene2::Button>(_assets->get<scene2::SceneNode>("level_level3")));
    _levelbuttons.push_back(std::dynamic_pointer_cast<scene2::Button>(_assets->get<scene2::SceneNode>("level_level4")));
    _levelbuttons.push_back(std::dynamic_pointer_cast<scene2::Button>(_assets->get<scene2::SceneNode>("level_level5")));
    _levelbuttons.push_back(std::dynamic_pointer_cast<scene2::Button>(_assets->get<scene2::SceneNode>("level_level6")));
    _levelbuttons.push_back(std::dynamic_pointer_cast<scene2::Button>(_assets->get<scene2::SceneNode>("level_level7")));
    _levelbuttons.push_back(std::dynamic_pointer_cast<scene2::Button>(_assets->get<scene2::SceneNode>("level_level8")));
    _highlightedlevels.push_back(_assets->get<scene2::SceneNode>("level_level1h"));
    _highlightedlevels.push_back(_assets->get<scene2::SceneNode>("level_level2h"));
    _highlightedlevels.push_back(_assets->get<scene2::SceneNode>("level_level3h"));
    _highlightedlevels.push_back(_assets->get<scene2::SceneNode>("level_level4h"));
    _highlightedlevels.push_back(_assets->get<scene2::SceneNode>("level_level5h"));
    _highlightedlevels.push_back(_assets->get<scene2::SceneNode>("level_level6h"));
    _highlightedlevels.push_back(_assets->get<scene2::SceneNode>("level_level7h"));
    _highlightedlevels.push_back(_assets->get<scene2::SceneNode>("level_level8h"));
    
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

    addChild(_scene);
    addChild(_scene_ui);
    std::cout<<_scene->getContentSize().width<<", "<<_scene->getContentSize().height<<"\n";
    std::cout<<_scene_ui->getContentSize().width<<", "<<_scene_ui->getContentSize().height<<"\n";
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
        _input->dispose();
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

/**
 * The method called to update the level scene.
 *
 * @param timestep  The amount of time (in seconds) since the last frame
 */
void LevelScene::update(float timestep) {
    _input->update();
    if (_pressed) {
        if (_input->didRelease()) {
            _pressed = false;
        } else if (_input->isDown()) {
            Vec2 cameraPos = _camera->getPosition();
            Vec2 dst = Vec2((_input->getPrevious() - _input->getPosition()).x, 0) + cameraPos;
            dst.x = std::max(std::min(_scene->getSize().width - _camera->getViewport().getMaxX() / (2 * _camera->getZoom()), dst.x), _camera->getViewport().getMaxX() / (2 * _camera->getZoom()));
            _camera->translate(dst - cameraPos);
            _camera->update();
            Vec2 uiPos = Vec2(_camera->getPosition().x - _camera->getViewport().getMaxX() / (2 * _camera->getZoom()), _camera->getPosition().y - _camera->getViewport().getMaxY() / (2 * _camera->getZoom()));
            _scene_ui->setPosition(uiPos);
        }
    } else {
        if (_input->didPress()) {
            _pressed = true;
//            std::cout<<_input->getPosition().x<<", "<<_input->getPosition().y<<"\n";
//            std::cout<<_scene_ui->getChildByName("title")->getPosition().x<<", "<<_scene_ui->getChildByName("title")->getPosition().y<<"\n";
            if (_backbutton->isDown() || _nextbutton->isDown()) {
                _pressed = false;
            }
            for (size_t i = 0; i < _levelbuttons.size(); ++i) {
                if (_levelbuttons[i]->isDown()) {
                    _pressed = false;
                }
            }
        }
    }
}
